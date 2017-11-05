/* 
 * media_reader.c
 * @author: ppmpeg@ovenworks.jp
 */

#include "media_reader.h"

#include <stdlib.h>
#include <stdbool.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>

#include "util/instance.h"
#include "util/buffer.h"
#include "util/frame.h"

/** 参照カウントをインクリメントする */ 
static PyObject* inc_ref(PyObject* obj) {   //@todo ユーティリティにまとめる
    Py_IncRef(obj);
    return obj;
}

/**
 * メディアリーダー構造体
 * @author ppmpeg@ovenworks.jp
 */
typedef struct MediaReader {

    bool opened;                        ///< オープン済みか
    AVFormatContext* format_context;    ///< フォーマットコンテキスト

    AVStream* vstream;                  ///< ビデオストリーム
    AVCodecContext* vcodec_context;     ///< ビデオコーデックコンテキスト
    uint8_t* vbuffer;                   ///< ビデオバッファ
    int vbuffer_size;                   ///< ビデオバッファのサイズ

    AVStream* astream;                  ///< オーディオストリーム
    AVCodecContext* acodec_context;     ///< オーディオコーデックコンテキスト
    uint8_t* abuffer;                   ///< オーディオバッファ
    int abuffer_size;                   ///< オーディオバッファのサイズ
    
    AVFrame* frame;                     ///< フレーム
    AVPacket packet;                    ///< パケット
    bool read_frame_completed;          ///< フレームの読み出しが完了済みか
    bool send_vpacket_completed;        ///< ビデオパケットの送信が完了済みか
    bool receive_vframe_completed;      ///< ビデオフレームの受信が完了済みか
    bool send_apacket_completed;        ///< オーディオパケットの送信が完了済みか
    bool receive_aframe_completed;      ///< オーディオフレームの受信が完了済みか

    bool closed;                        ///< クローズ済みか
    
} MediaReader;

/** demux結果型 */
typedef enum DemuxResult {
    
    DemuxResult_OK,         ///< OK
    DemuxResult_CONTINUE,   ///< 再度処理が必要
    DemuxResult_EOF,        ///< 終端
    DemuxResult_ERROR       ///< エラー
    
} DemuxResult;

static PyObject* PpmpegExc_MediaReaderError;   ///< 例外オブジェクト

/** オープンしているかチェックする */
#define CHECK_OPEN(inst) \
    if(!(inst)->opened || (inst)->closed) return PyErr_Format(PpmpegExc_MediaReaderError, "Not opened yet or already closed.")

static DemuxResult demux_video(MediaReader* instance);
static DemuxResult demux_audio(MediaReader* instance);

/* MediaReaderError 例外を作成する */
PyObject* MediaReader_newMediaReaderError(const char* name) {

    PpmpegExc_MediaReaderError = PyErr_NewException(name, NULL, NULL);
    Py_INCREF(PpmpegExc_MediaReaderError);

    return PpmpegExc_MediaReaderError;
}

/** 初期化する */
PyObject* MediaReader_init(PyObject* self, PyObject* args) {

    MediaReader* instance = (MediaReader*)PyMem_Malloc(sizeof(MediaReader));

    instance->opened = false;
    instance->format_context = NULL;
    
    instance->vstream = NULL;
    instance->vcodec_context = NULL;
    instance->vbuffer = NULL;
    instance->vbuffer_size = 0;
    
    instance->astream = NULL;
    instance->acodec_context = NULL;
    instance->abuffer = NULL;
    instance->abuffer_size = 0;

    instance->frame = NULL;
    av_init_packet(&instance->packet);
    instance->packet.data = NULL;
    instance->packet.size = 0;
    instance->read_frame_completed = false;
    instance->send_vpacket_completed = false;
    instance->receive_vframe_completed = false;
    instance->send_apacket_completed = false;
    instance->receive_aframe_completed = false;
    
    instance->closed = false;

    instance_bind(self, instance);
    
    Py_RETURN_NONE;
}

/** メディアをオープンする */
PyObject* MediaReader_open(PyObject* self, PyObject* args) {

    MediaReader* instance = instance_get_bound(self);

    if(instance->opened || instance->closed)
        return PyErr_Format(PpmpegExc_MediaReaderError, "Already opened or closed.");
    
    //引数をパースする
    const char* filepath;

    if(!PyArg_ParseTuple(args, "s", &filepath))
        return NULL;
    
    //フォーマットをオープンする
    if(avformat_open_input(&instance->format_context, filepath, NULL, NULL) < 0)
        return PyErr_Format(PpmpegExc_MediaReaderError, "Failed to open input: %s", filepath);

    //ストリーム情報を探す
    if(avformat_find_stream_info(instance->format_context, NULL) < 0)
        return PyErr_Format(PpmpegExc_MediaReaderError, "Failed to find stream information: %s", filepath);
    
    //ビデオストリームをオープンする
    int vsindex;
    AVCodec* vcodec;
    
    if((vsindex = av_find_best_stream(instance->format_context, AVMEDIA_TYPE_VIDEO, -1, -1, &vcodec, 0)) >= 0) {
    
        instance->vstream = instance->format_context->streams[vsindex];

        if((instance->vcodec_context = avcodec_alloc_context3(vcodec)) == NULL)
            return PyErr_Format(PpmpegExc_MediaReaderError, "Failed to allocate video codec context: %s", filepath);

        if(avcodec_parameters_to_context(instance->vcodec_context, instance->vstream->codecpar) < 0)
            return PyErr_Format(PpmpegExc_MediaReaderError, "Failed to setup video codec context: %s", filepath);

        if(avcodec_open2(instance->vcodec_context, vcodec, NULL) < 0)
            return PyErr_Format(PpmpegExc_MediaReaderError, "Failed to open video codec context: %s", filepath);
    }

    //オーディオストリームをオープンする
    int asindex;
    AVCodec* acodec;
    
    if((asindex = av_find_best_stream(instance->format_context, AVMEDIA_TYPE_AUDIO, -1, -1, &acodec, 0)) >= 0) {
    
        instance->astream = instance->format_context->streams[asindex];

        if((instance->acodec_context = avcodec_alloc_context3(acodec)) == NULL)
            return PyErr_Format(PpmpegExc_MediaReaderError, "Failed to alloc audio codec context: %s", filepath);

        if(avcodec_parameters_to_context(instance->acodec_context, instance->astream->codecpar) < 0)
            return PyErr_Format(PpmpegExc_MediaReaderError, "Failed to setup audio codec context: %s", filepath);

        if(avcodec_open2(instance->acodec_context, acodec, NULL) < 0)
            return PyErr_Format(PpmpegExc_MediaReaderError, "Failed to open audio codec context: %s", filepath);
    }

    //フレームを確保する
    if((instance->frame = av_frame_alloc()) == NULL)
        return PyErr_Format(PpmpegExc_MediaReaderError, "Failed to alloc frame: %s", filepath);
    
    instance->opened = true;
    
    Py_RETURN_NONE;
}

/** メタデータを返す */
PyObject* MediaReader_get_metadata(PyObject* self, PyObject* args) {

    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    //引数をパースする
    const char* key;
    
    if(!PyArg_ParseTuple(args, "s", &key))
        return NULL;

    //メタデータを取得する
    AVDictionaryEntry* entry = av_dict_get(instance->format_context->metadata, key, NULL, AV_DICT_IGNORE_SUFFIX);
    
    return (entry != NULL) ? PyUnicode_FromString(entry->value) : inc_ref(Py_None);
}

/** 長さを返す */
PyObject* MediaReader_get_duration(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);    
    CHECK_OPEN(instance);
    
    return PyFloat_FromDouble((double)instance->format_context->duration / AV_TIME_BASE);
}

/** ビットレートを返す */
PyObject* MediaReader_get_bit_rate(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    return PyLong_FromLong(instance->format_context->bit_rate);
}

/** ビデオストリームがあるかを返す */
PyObject* MediaReader_has_video_stream(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->vstream != NULL) ? inc_ref(Py_True) : inc_ref(Py_False);
}

/** ビデオの幅を返す */
PyObject* MediaReader_get_video_width(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    return (instance->vstream != NULL) ? PyLong_FromLong(instance->vcodec_context->width) : inc_ref(Py_None);
}

/** ビデオの高さを返す */
PyObject* MediaReader_get_video_height(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    return (instance->vstream != NULL) ? PyLong_FromLong(instance->vcodec_context->height) : inc_ref(Py_None);
}

/** ビデオピクセルフォーマットを返す */
PyObject* MediaReader_get_video_pixel_format(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->vstream != NULL) ? PyUnicode_FromString(av_get_pix_fmt_name(instance->vcodec_context->pix_fmt)) : inc_ref(Py_None);
}

/** ビデオのフレームレートを返す */
PyObject* MediaReader_get_video_frame_rate(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->vstream != NULL) ? PyFloat_FromDouble(av_q2d(av_stream_get_r_frame_rate(instance->vstream))) : inc_ref(Py_None);
}

/** ビデオのビットレートを返す */
PyObject* MediaReader_get_video_bit_rate(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
        
    return (instance->vstream != NULL) ? PyLong_FromLong(instance->vcodec_context->bit_rate) : inc_ref(Py_None);
}

/** ビデオのGOPサイズを返す */
PyObject* MediaReader_get_video_gop_size(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    return (instance->vstream != NULL) ? PyLong_FromLong(instance->vcodec_context->gop_size) : inc_ref(Py_None);
}

/** ビデオの開始時刻を返す */
PyObject* MediaReader_get_video_start_time(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    if(instance->vstream == NULL)
        Py_RETURN_NONE;
    
    if(instance->vstream->start_time == AV_NOPTS_VALUE)
        return PyFloat_FromDouble(0.0);

    return PyFloat_FromDouble(instance->vstream->start_time * av_q2d(instance->vstream->time_base));
}

/** ビデオコーデックを返す */
PyObject* MediaReader_get_video_codec(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    return (instance->vstream != NULL) ? PyUnicode_FromString(instance->vcodec_context->codec->name) : inc_ref(Py_None);
}

/** オーディオストリームがあるかを返す */
PyObject* MediaReader_has_audio_stream(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? inc_ref(Py_True) : inc_ref(Py_False);
}

/** オーディオのチャンネル数を返す */
PyObject* MediaReader_get_audio_channels(PyObject* self, PyObject* args) {

    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? PyLong_FromLong(instance->acodec_context->channels) : inc_ref(Py_None);
}

/** オーディオのフレームあたりのサンプル数を返す */
PyObject* MediaReader_get_audio_nb_samples(PyObject* self, PyObject* args) {

    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? PyLong_FromLong(instance->acodec_context->frame_size) : inc_ref(Py_None);
}

/** オーディオのサンプルフォーマットを返す */
PyObject* MediaReader_get_audio_sample_format(PyObject* self, PyObject* args) {

    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? PyUnicode_FromString(av_get_sample_fmt_name(instance->acodec_context->sample_fmt)) : inc_ref(Py_None);
}

/** オーディオのサンプルレートを返す */
PyObject* MediaReader_get_audio_sample_rate(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? PyLong_FromLong(instance->acodec_context->sample_rate) : inc_ref(Py_None);
}

/** オーディオのビットレートを返す */
PyObject* MediaReader_get_audio_bit_rate(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? PyLong_FromLong(instance->acodec_context->bit_rate) : inc_ref(Py_None);
}

/** オーディオの開始時刻を返す */
PyObject* MediaReader_get_audio_start_time(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    if(instance->astream == NULL)
        Py_RETURN_NONE;
    
    if(instance->astream->start_time == AV_NOPTS_VALUE)
        return PyFloat_FromDouble(0.0);

    return PyFloat_FromDouble(instance->astream->start_time * av_q2d(instance->astream->time_base));
}

/** オーディオコーデックを返す */
PyObject* MediaReader_get_audio_codec(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? PyUnicode_FromString(instance->acodec_context->codec->name) : inc_ref(Py_None);
}

/** フレームのデコードを進める */
PyObject* MediaReader_process_decode(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    //引数をパースする
    int share_vpixels, share_asamples;
  
    if(!PyArg_ParseTuple(args, "pp", &share_vpixels, &share_asamples))
        return NULL;
    
    //フレームを読み出す
    if(instance->read_frame_completed && instance->receive_vframe_completed && instance->receive_aframe_completed)
        Py_RETURN_FALSE;
    
    if(!instance->read_frame_completed) {
        if(av_read_frame(instance->format_context, &instance->packet) < 0)
            instance->read_frame_completed = true;
    }
    
    //ビデオフレームをデコードする
    if(instance->vstream != NULL && instance->packet.stream_index == instance->vstream->index) {

        //demuxする
        switch(demux_video(instance)) {
            case DemuxResult_OK:
                break;
            case DemuxResult_CONTINUE:
                Py_RETURN_TRUE;
            case DemuxResult_EOF:
                Py_RETURN_FALSE;
            case DemuxResult_ERROR:
            default:
                return PyErr_Format(PpmpegExc_MediaReaderError, "Failed to demux video.");                
        }

        //エラーフレームならスキップする
        if(instance->frame->format == AV_PIX_FMT_NONE) {
            fprintf(stderr, "Pixel format of video frame %d is invalid. Skip frame.\n", instance->vcodec_context->frame_number);
            Py_RETURN_TRUE;
        }
        
        //PTSを求める
        double pts = instance->frame->pts * av_q2d(instance->vstream->time_base);
        
        //ビデオバッファを確保する
        int size;

        size = av_image_get_buffer_size(instance->frame->format, instance->frame->width, instance->frame->height, 1);
        instance->vbuffer_size = buffer_alloc(&instance->vbuffer, instance->vbuffer_size, size);

        //ピクセルデータをバッファにコピーする
        uint8_t* buffer;
        
        if((buffer = frame_copy_pixels_to_buffer(instance->frame, instance->vbuffer, instance->vbuffer_size)) == NULL)
            return PyErr_Format(PpmpegExc_MediaReaderError, "Failed to copy pixels from frame.");
        
        //ピクセルデータ領域の共有指定に従ってメモリビュー／バイト配列を作成する
        PyObject* pixels;
        
        if(share_vpixels)
            pixels = PyMemoryView_FromMemory(buffer, size, PyBUF_READ);
        else
            pixels = PyBytes_FromStringAndSize(buffer, size);
        
        PyObject* result = Py_BuildValue("(sdiisNO)",
                                         "V",
                                         pts,
                                         instance->frame->width,
                                         instance->frame->height,
                                         av_get_pix_fmt_name(instance->frame->format),  ///< @todo ピクセルフォーマットIDを返して高速化する
                                         pixels,
                                         (instance->frame->key_frame) ? Py_True : Py_False);

        av_frame_unref(instance->frame);

        return result;
    }
    //オーディオフレームをデコードする
    else if(instance->astream != NULL && instance->packet.stream_index == instance->astream->index) {

        //demuxする
        switch(demux_audio(instance)) {
            case DemuxResult_OK:
                break;
            case DemuxResult_CONTINUE:
                Py_RETURN_TRUE;
            case DemuxResult_EOF:
                Py_RETURN_FALSE;
            case DemuxResult_ERROR:
            default:
                return PyErr_Format(PpmpegExc_MediaReaderError, "Failed to demux audio.");                
        }

        //エラーフレームならスキップする
        if(instance->frame->format == AV_SAMPLE_FMT_NONE) {
            fprintf(stderr, "Sample format of audio frame %d is invalid. Skip frame.\n", instance->acodec_context->frame_number);
            Py_RETURN_TRUE;
        }
         
        //PTSを求める
        double pts = instance->frame->pts * av_q2d(instance->astream->time_base);

        //オーディオバッファを確保する        
        int size;

        size = av_samples_get_buffer_size(NULL, instance->frame->channels, instance->frame->nb_samples, instance->frame->format, 1);
        instance->abuffer_size = buffer_alloc(&instance->abuffer, instance->abuffer_size, size);

        //サンプルデータをバッファにコピーする
        uint8_t* buffer;
        
        if((buffer = frame_copy_samples_to_buffer(instance->frame, instance->abuffer, instance->abuffer_size)) == NULL)
            return PyErr_Format(PpmpegExc_MediaReaderError, "Failed to copy samples from frame.");
        
        //サンプルデータ領域の共有指定に従ってメモリビュー／バイト配列を作成する
        PyObject* samples;
        
        if(share_asamples)
            samples = PyMemoryView_FromMemory(buffer, size, PyBUF_READ);
        else
            samples = PyBytes_FromStringAndSize(buffer, size);

        PyObject* result = Py_BuildValue("(sdiisN)",
                                         "A",
                                         pts,
                                         instance->frame->channels,
                                         instance->frame->nb_samples,
                                         av_get_sample_fmt_name(instance->frame->format),   ///< @todo サンプルフォーマットIDを返して高速化する
                                         samples);

        av_frame_unref(instance->frame);
        
        return result;
    }
    else {
        Py_RETURN_TRUE;
    }
}

/**
 * ビデオをdemuxする
 * @param instance  インスタンス
 * @return          結果
 */
DemuxResult demux_video(MediaReader* instance) {
    
    if(instance->send_vpacket_completed && instance->receive_vframe_completed)
        return DemuxResult_EOF;
    
    //デコーダーにパケットを送る
    if(!instance->send_vpacket_completed) {
        switch(avcodec_send_packet(instance->vcodec_context, &instance->packet)) {
            case 0:
                break;
            case AVERROR(EAGAIN):
                return DemuxResult_CONTINUE;
            case AVERROR_EOF:
                instance->send_vpacket_completed = true;
                break;
            default:
                return DemuxResult_ERROR;
        }
    }

    //デコーダーからフレームを受け取る
    if(!instance->receive_vframe_completed) {
        switch(avcodec_receive_frame(instance->vcodec_context, instance->frame)){
            case 0:
                av_packet_unref(&instance->packet);
                break;
            case AVERROR(EAGAIN):
                return DemuxResult_CONTINUE;
            case AVERROR_EOF:
                instance->receive_vframe_completed = true;
                return DemuxResult_EOF;
            default:
                return DemuxResult_ERROR;
        }
    }
    
    return DemuxResult_OK;
}

/**
 * オーディオをdemuxする
 * @param instance  インスタンス
 * @return          結果
 */
DemuxResult demux_audio(MediaReader* instance) {
    
    if(instance->send_apacket_completed && instance->receive_aframe_completed)
        return DemuxResult_EOF;

    //デコーダーにパケットを送る
    if(!instance->send_apacket_completed) {
        switch(avcodec_send_packet(instance->acodec_context, &instance->packet)) {
            case 0:
                break;
            case AVERROR(EAGAIN):
                return DemuxResult_CONTINUE;
            case AVERROR_EOF:
                instance->send_apacket_completed = true;
                break;
            default:
                return DemuxResult_ERROR;
        }
    }
    
    //デコーダーからパケットを受け取る
    if(!instance->receive_aframe_completed) {
        switch(avcodec_receive_frame(instance->acodec_context, instance->frame)) {
            case 0:
                av_packet_unref(&instance->packet);
                break;
            case AVERROR(EAGAIN):
                return DemuxResult_CONTINUE;
            case AVERROR_EOF:
                return DemuxResult_EOF;
            default:
                return DemuxResult_ERROR;
        }
    }
    
    return DemuxResult_OK;
}

/** デコードしたビデオフレーム数を返す */
PyObject* MediaReader_get_decoded_video_frames(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    if(instance->vcodec_context == NULL)
        return PyLong_FromLong(0);
    
    return PyLong_FromLong(instance->vcodec_context->frame_number);
}

/** デコードしたオーディオフレーム数を返す */
PyObject* MediaReader_get_decoded_audio_frames(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    if(instance->acodec_context == NULL)
        return PyLong_FromLong(0);
        
    return PyLong_FromLong(instance->acodec_context->frame_number);
}

/** クローズする */
PyObject* MediaReader_close(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);
    
    if(!instance->opened || instance->closed)
        Py_RETURN_NONE;
        
    //フレームを解放する
    av_frame_free(&instance->frame);

    //オーディオバッファを解放する    
    buffer_freep(&instance->abuffer);
    instance->abuffer_size = 0;
    
    //ビデオバッファを解放する
    buffer_freep(&instance->vbuffer);
    instance->vbuffer_size = 0;
        
    //オーディオストリームをクローズする
    if(instance->astream != NULL) {
        avcodec_free_context(&instance->acodec_context);
        instance->astream = NULL;
    }

    //ビデオストリームをクローズする
    if(instance->vstream != NULL) {
        avcodec_free_context(&instance->vcodec_context);
        instance->vstream = NULL;
    }
    
    //フォーマットをクローズする
    if(instance->format_context != NULL)
        avformat_close_input(&instance->format_context);

    instance->closed = true;
    
    Py_RETURN_NONE;
}

/** 破棄する */
PyObject* MediaReader_del(PyObject* self, PyObject* args) {
    
    MediaReader* instance = instance_get_bound(self);

    if(instance == NULL)
        Py_RETURN_NONE;

    if(instance->opened && !instance->closed)
        MediaReader_close(self, NULL);

    PyMem_Free(instance);
    instance_unbind(self);

    Py_RETURN_NONE;
}
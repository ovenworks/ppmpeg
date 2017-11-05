/* 
 * media_writer.c
 * @author: ppmpeg@ovenworks.jp
 */

#include "media_writer.h"

#include <stdlib.h>
#include <stdbool.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>

#include "util/instance.h"
#include "util/buffer.h"
#include "util/frame.h"

/** 大きいほうの値を返す */ //@todo ユーティリティにまとめる
#define MAX(v1, v2) \
    ((v1) < (v2)) ? (v2) : (v1)

/** 参照カウントをインクリメントする */
static PyObject* inc_ref(PyObject* obj) { //@todo ユーティリティにまとめる
    Py_IncRef(obj);
    return obj;
}

/**
 * メディアライター構造体
 * @author ppmpeg@ovenworks.jp
 */
typedef struct MediaWriter {

    bool opened;                        ///< オープン済みか
    AVFormatContext* format_context;    ///< フォーマットコンテキスト

    AVStream* vstream;                  ///< ビデオストリーム
    AVCodecContext* vcodec_context;     ///< ビデオコーデックコンテキスト
    int64_t vstart_time;                ///< ビデオの開始時刻
    AVFrame* vframe;                    ///< ビデオフレーム
    
    AVStream* astream;                  ///< オーディオストリーム
    AVCodecContext* acodec_context;     ///< オーディオコーデックコンテキスト
    int64_t astart_time;                ///< オーディオの開始時刻
    AVFrame* aframe;                    ///< オーディオフレーム

    AVPacket packet;                    ///< パケット

    bool header_written;                ///< ヘッダ書き込み済みか
    bool send_vframe_completed;         ///< ビデオフレームの送信が完了済みか
    bool receive_vpacket_completed;     ///< ビデオパケットの受信が完了済みか
    bool send_aframe_completed;         ///< オーディオフレームの送信が完了済みか
    bool receive_apacket_completed;     ///< オーディオパケットの受信が完了済みか
    bool flushed;                       ///< フラッシュ済みか

    bool closed;                        ///< クローズ済みか
    
} MediaWriter;

/** mux結果型 */
typedef enum MuxResult {
    
    MuxResult_OK,       ///< OK
    MuxResult_CONTINUE, ///< 再度処理が必要
    MuxResult_EOF,      ///< 終端
    MuxResult_ERROR     ///< エラー
    
} MuxResult;

static PyObject* PpmpegExc_MediaWriterError;   ///< 例外オブジェクト

/** オープンしているかチェックする */
#define CHECK_OPEN(inst) \
    if(!(inst)->opened || (inst)->closed) return PyErr_Format(PpmpegExc_MediaWriterError, "Not opened yet or already closed.")

static MuxResult mux_video(MediaWriter* instance);
static MuxResult mux_audio(MediaWriter* instance);

/* MediaWriterError 例外を作成する */
PyObject* MediaWriter_newMediaWriterError(const char* name) {

    PpmpegExc_MediaWriterError = PyErr_NewException(name, NULL, NULL);
    Py_INCREF(PpmpegExc_MediaWriterError);

    return PpmpegExc_MediaWriterError;
}

/** 初期化する */
PyObject* MediaWriter_init(PyObject* self, PyObject* args) {

    MediaWriter* instance = (MediaWriter*)PyMem_Malloc(sizeof(MediaWriter));

    instance->opened = false;
    instance->format_context = NULL;
    
    instance->vstream = NULL;
    instance->vcodec_context = NULL;
    instance->vstart_time = 0;
    instance->vframe = NULL;

    instance->astream = NULL;
    instance->acodec_context = NULL;
    instance->astart_time = 0;
    instance->aframe = NULL;

    av_init_packet(&instance->packet);
    instance->packet.data = NULL;
    instance->packet.size = 0;

    instance->header_written = false;
    instance->send_vframe_completed = false;
    instance->receive_vpacket_completed = false;
    instance->send_aframe_completed = false;
    instance->receive_apacket_completed = false;
    instance->flushed = false;

    instance->closed = false;

    instance_bind(self, instance);
    
    Py_RETURN_NONE;
}

/** オープンする */
PyObject* MediaWriter_open(PyObject* self, PyObject* args) {

    MediaWriter* instance = instance_get_bound(self);

    if(instance->opened || instance->closed)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Already opened or closed.");
    
    //引数をパースする
    const char* filepath;

    if(!PyArg_ParseTuple(args, "s", &filepath))
        return NULL;
        
    //フォーマットコンテキストを確保する
    if(avformat_alloc_output_context2(&instance->format_context, NULL, NULL, filepath) < 0)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to allocate format context.");

    //ファイルをオープンする
    if(!(instance->format_context->oformat->flags & AVFMT_NOFILE)) {
        if(avio_open(&instance->format_context->pb, filepath, AVIO_FLAG_WRITE) < 0)
            return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to open avio: %s", filepath);
    }

    instance->opened = true;
    
    Py_RETURN_NONE;
}

/** メタデータを追加する */
PyObject* MediaWriter_put_metadata(PyObject* self, PyObject* args) {

    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    //引数をパースする
    const char* key;
    const char* value;
    
    if(!PyArg_ParseTuple(args, "ss", &key, &value))
        return NULL;

    //メタデータを設定する
    if(av_dict_set(&instance->format_context->metadata, key, value, 0) < 0)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Cannot put metadata: %s=>%s", key, value);
    
    Py_RETURN_NONE;
}

/** ビデオストリームを追加する */
PyObject* MediaWriter_add_video_stream(PyObject* self, PyObject* args) {

    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    //引数をパースする
    int width, height;
    double fps;
    PyObject* bit_rate;
    PyObject* gop_size;
    PyObject* start_time;
    const char* codec_name;

    if(!PyArg_ParseTuple(args, "iidOOOz", &width, &height, &fps, &bit_rate, &gop_size, &start_time, &codec_name))
        return NULL;
            
    //コーデックを決定する
    if(instance->format_context->oformat->video_codec == AV_CODEC_ID_NONE)
        Py_RETURN_FALSE;
    
    AVCodec* vcodec;

    if(codec_name != NULL) {
        if((vcodec = avcodec_find_encoder_by_name(codec_name)) == NULL || vcodec->type != AVMEDIA_TYPE_VIDEO)
            return PyErr_Format(PpmpegExc_MediaWriterError, "Could not find video encoder.");        
    }
    else {
        if((vcodec = avcodec_find_encoder(instance->format_context->oformat->video_codec)) == NULL)
            return PyErr_Format(PpmpegExc_MediaWriterError, "Could not find video encoder.");        
    }
    
    //ストリームを作成する
    if((instance->vstream = avformat_new_stream(instance->format_context, vcodec)) == NULL)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to allocate video stream.");

    //コーデックコンテキストを確保する
    if((instance->vcodec_context = avcodec_alloc_context3(vcodec)) == NULL)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to allocate codec context.");

    //ストリームとコーデックのパラメーターを設定する
    bool fps_supported = false;
    
    for(int i = 0; vcodec->supported_framerates != NULL && vcodec->supported_framerates[i].den != 0; ++i) {
        if(av_cmp_q(vcodec->supported_framerates[i], av_d2q(fps, INT_MAX)) == 0) {
            fps_supported = true;
            break;
        }
    }
    //@todo 正確なrational値でマッチングし、近傍値を選ぶ
    //if(!fps_supported)
    //    fprintf(stderr, "Specified frame rate is not supported. Making stream will fail or incorrect.\n");

    instance->vcodec_context->width = width;
    instance->vcodec_context->height = height;
    instance->vcodec_context->framerate = av_d2q(fps, INT_MAX);
    instance->vcodec_context->time_base = av_d2q(1.0 / fps, INT_MAX);
    instance->vcodec_context->pix_fmt = vcodec->pix_fmts[0];
    instance->vcodec_context->bit_rate = (bit_rate != Py_None) ? PyLong_AsLong(bit_rate) : instance->vcodec_context->bit_rate;
    instance->vcodec_context->gop_size = (gop_size != Py_None) ? MAX(PyLong_AsLong(gop_size), 1) : MAX((int)fps, 1);

    //
    instance->vstream->time_base = instance->vcodec_context->time_base;

    if(start_time != Py_None)
        instance->vstart_time = PyFloat_AsDouble(start_time) / av_q2d(instance->vstream->time_base);

    av_stream_set_r_frame_rate(instance->vstream, instance->vcodec_context->framerate);
                
    //
    if(instance->format_context->oformat->flags & AVFMT_GLOBALHEADER)
        instance->vcodec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;    

    //コーデックをオープンする        
    if(avcodec_open2(instance->vcodec_context, NULL/*codec*/, NULL/*&opt*/) < 0)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to open video codec.");

    //ストリームのパラメータをコーデックにコピーする
    if(avcodec_parameters_from_context(instance->vstream->codecpar, instance->vcodec_context) < 0)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to copy video stream parameters to codec.");

    //ビデオフレームを確保する
    if((instance->vframe = av_frame_alloc()) == NULL)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to allocate video frame.");
        
    instance->vframe->width  = instance->vcodec_context->width;
    instance->vframe->height = instance->vcodec_context->height;
    instance->vframe->format = instance->vcodec_context->pix_fmt;

    if(av_frame_get_buffer(instance->vframe, 32) < 0)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to allocate video buffer.");
    
    Py_RETURN_TRUE;
}

/** ビデオストリームがあるかを返す */
PyObject* MediaWriter_has_video_stream(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    return (instance->vstream != NULL) ? inc_ref(Py_True) : inc_ref(Py_False);
}

/** ビデオの幅を返す */
PyObject* MediaWriter_get_video_width(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    return (instance->vstream != NULL) ? PyLong_FromLong(instance->vcodec_context->width) : inc_ref(Py_None);
}

/** ビデオの高さを返す */
PyObject* MediaWriter_get_video_height(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    return (instance->vstream != NULL) ? PyLong_FromLong(instance->vcodec_context->height) : inc_ref(Py_None);
}

/** ビデオのピクセルフォーマットを返す */
PyObject* MediaWriter_get_video_pixel_format(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->vstream != NULL) ? PyUnicode_FromString(av_get_pix_fmt_name(instance->vcodec_context->pix_fmt)) : inc_ref(Py_None);
}

/** ビデオのフレームレートを返す */
PyObject* MediaWriter_get_video_frame_rate(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
        
    return (instance->vstream != NULL) ? PyFloat_FromDouble(av_q2d(av_stream_get_r_frame_rate(instance->vstream))) : inc_ref(Py_None);
}

/** ビデオのビットレートを返す */
PyObject* MediaWriter_get_video_bit_rate(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
        
    return (instance->vstream != NULL) ? PyLong_FromLong(instance->vcodec_context->bit_rate) : inc_ref(Py_None);
}

/** ビデオのGOPサイズを返す */
PyObject* MediaWriter_get_video_gop_size(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    return (instance->vstream != NULL) ? PyLong_FromLong(instance->vcodec_context->gop_size) : inc_ref(Py_None);
}

/** ビデオの開始時刻を返す */
PyObject* MediaWriter_get_video_start_time(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    if(instance->vstream == NULL)
        Py_RETURN_NONE;
    
    if(instance->vstream->start_time == AV_NOPTS_VALUE)
        return PyFloat_FromDouble(0.0);

    return PyFloat_FromDouble(instance->vstream->start_time * av_q2d(instance->vstream->time_base));
}

/** ビデオコーデックを返す */
PyObject* MediaWriter_get_video_codec(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    return (instance->vstream != NULL) ? PyUnicode_FromString(instance->vcodec_context->codec->name) : inc_ref(Py_None);
}

/** オーディオストリームを追加する */
PyObject* MediaWriter_add_audio_stream(PyObject* self, PyObject* args) {

    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    //引数をパースする
    int channels, sample_rate;
    PyObject* bit_rate;
    PyObject* start_time;
    const char* codec_name;

    if(!PyArg_ParseTuple(args, "iiOOz", &channels, &sample_rate, &bit_rate, &start_time, &codec_name))
        return NULL;

    //コーデックを決定する
    if(instance->format_context->oformat->audio_codec == AV_CODEC_ID_NONE)
        Py_RETURN_FALSE;

    AVCodec* acodec;

    if(codec_name != NULL) {
        if((acodec = avcodec_find_encoder_by_name(codec_name)) == NULL || acodec->type != AVMEDIA_TYPE_AUDIO)
            return PyErr_Format(PpmpegExc_MediaWriterError, "Could not find audio encoder.");
    }
    else {
        if((acodec = avcodec_find_encoder(instance->format_context->oformat->audio_codec)) == NULL)
            return PyErr_Format(PpmpegExc_MediaWriterError, "Could not find audio encoder.");
    }
    
    //ストリームを作成する
    if((instance->astream = avformat_new_stream(instance->format_context, acodec)) == NULL)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to allocate audio stream.");
    
    //コーデックコンテキストを確保する
    if((instance->acodec_context = avcodec_alloc_context3(acodec)) == NULL)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to allocate codec context.");

    //ストリームとコーデックのパラメーターを設定する
    bool channels_supported = false;
    
    for(int i = 0; acodec->channel_layouts != NULL && acodec->channel_layouts[i] != 0; ++i) {
        if(av_get_channel_layout_nb_channels(acodec->channel_layouts[i]) == channels) {
            channels_supported = true;
            break;
        }
    }
    //@todo 近傍値を選ぶ
    //if(!channels_supported)
    //    fprintf(stderr, "Specified channels is not supported. Making stream will fail or incorrect.\n");

    bool sample_rate_supported = false;
    
    for(int i = 0; acodec->supported_samplerates != NULL && acodec->supported_samplerates[i] != 0; ++i) {
        if(acodec->supported_samplerates[i] == sample_rate) {
            sample_rate_supported = true;
            break;
        }
    }
    //@todo 近傍値を選ぶ
    //if(!sample_rate_supported)
    //    fprintf(stderr, "Specified sample rate is not supported. Making stream will fail or incorrect.\n");

    instance->acodec_context->channels = channels;
    instance->acodec_context->channel_layout = av_get_default_channel_layout(channels);
    instance->acodec_context->sample_fmt = acodec->sample_fmts[0];
    instance->acodec_context->sample_rate = sample_rate;
    instance->acodec_context->time_base = av_make_q(1, sample_rate);
    instance->acodec_context->bit_rate = (bit_rate != Py_None) ? PyLong_AsLong(bit_rate) : instance->acodec_context->bit_rate;

    instance->astream->time_base = instance->acodec_context->time_base;

    if(start_time != Py_None)
        instance->astart_time = PyFloat_AsDouble(start_time) / av_q2d(instance->astream->time_base);
    
    //
    if(instance->format_context->oformat->flags & AVFMT_GLOBALHEADER)
        instance->acodec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    
    //コーデックをオープンする
    if(avcodec_open2(instance->acodec_context, NULL/*codec*/, NULL/*&opt*/) < 0)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to open audio codec.");
        
    //ストリームのパラメータをコーデックにコピーする
    if(avcodec_parameters_from_context(instance->astream->codecpar, instance->acodec_context) < 0)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to copy audio stream parameters to codec.");

    //オーディオフレームを確保する
    if((instance->aframe = av_frame_alloc()) == NULL)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to allocate audio frame.");
                
    instance->aframe->channel_layout = instance->acodec_context->channel_layout;
    instance->aframe->sample_rate = instance->acodec_context->sample_rate;
    instance->aframe->nb_samples = instance->acodec_context->frame_size;
    instance->aframe->format = instance->acodec_context->sample_fmt;
    
    if(instance->aframe->nb_samples > 0) {
        if(av_frame_get_buffer(instance->aframe, 0) < 0)
            return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to allocate audio buffer.");
    }
#ifdef undef
    if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = c->frame_size;
#endif
    
    Py_RETURN_TRUE;
}

/** オーディオストリームがあるかを返す */
PyObject* MediaWriter_has_audio_stream(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? inc_ref(Py_True) : inc_ref(Py_False);
}

/** オーディオのチャンネル数を返す */
PyObject* MediaWriter_get_audio_channels(PyObject* self, PyObject* args) {

    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? PyLong_FromLong(instance->acodec_context->channels) : inc_ref(Py_None);
}

/** オーディオのフレームアタリのサンプル数を返す */
PyObject* MediaWriter_get_audio_nb_samples(PyObject* self, PyObject* args) {

    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? PyLong_FromLong(instance->acodec_context->frame_size) : inc_ref(Py_None);
}

/** オーディオのサンプルフォーマットを返す */
PyObject* MediaWriter_get_audio_sample_format(PyObject* self, PyObject* args) {

    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? PyUnicode_FromString(av_get_sample_fmt_name(instance->acodec_context->sample_fmt)) : inc_ref(Py_None);
}

/** オーティオのサンプルレートを返す */
PyObject* MediaWriter_get_audio_sample_rate(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? PyLong_FromLong(instance->acodec_context->sample_rate) : inc_ref(Py_None);
}

/** オーティオのビットレートを返す */
PyObject* MediaWriter_get_audio_bit_rate(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? PyLong_FromLong(instance->acodec_context->bit_rate) : inc_ref(Py_None);
}

/** オーディオの開始時刻を返す */
PyObject* MediaWriter_get_audio_start_time(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    if(instance->astream == NULL)
        Py_RETURN_NONE;
    
    if(instance->astream->start_time == AV_NOPTS_VALUE)
        return PyFloat_FromDouble(0.0);

    return PyFloat_FromDouble(instance->astream->start_time * av_q2d(instance->astream->time_base));
}

/** オーディオコーデックを返す */
PyObject* MediaWriter_get_audio_codec(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    return (instance->astream != NULL) ? PyUnicode_FromString(instance->acodec_context->codec->name) : inc_ref(Py_None);
}

/** ビデオフレームのエンコードを進める*/
PyObject* MediaWriter_process_encode_video(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    if(instance->vstream == NULL)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Video stream does not exist.");

    //引数をパースする
    int width, height;
    const char* pixel_format_name;
    Py_buffer pixels;
    
    if(!PyArg_ParseTuple(args, "iisy*", &width, &height, &pixel_format_name, &pixels))
        return NULL;
        
    //ピクセルフォーマットIDを取得する
    enum AVPixelFormat pixel_format;
    
    if((pixel_format = av_get_pix_fmt(pixel_format_name)) == AV_PIX_FMT_NONE)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Cannot find pixel format: %s", pixel_format_name);
    
    if(pixel_format != instance->vframe->format)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Pixel format does not match with %s.", av_get_pix_fmt_name(instance->vcodec_context->pix_fmt));

    //ピクセルデータをビデオバッファにコピーする
    if(!frame_copy_pixels_from_buffer(instance->vframe, width, height, pixel_format, pixels.buf, pixels.len))
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to copy pixels to frame.");
    
    PyBuffer_Release(&pixels);
    
    //フレームを準備し、PTSを設定する
    //@attention ストリームの time_base は FFmpeg によって変更される場合があるため利用しないこと。
    if(av_frame_make_writable(instance->vframe) < 0)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to make video frame writablee.");

    instance->vframe->pts = av_rescale_q(instance->vcodec_context->frame_number,
                                         av_make_q(instance->vcodec_context->framerate.den, instance->vcodec_context->framerate.num), //@todo ビデオストリーム作成時にinstanceに保存しておいてもよい
                                         instance->vcodec_context->time_base);
    instance->vframe->pts += instance->vstart_time;
    
    //muxする
    switch(mux_video(instance)) {
        case MuxResult_OK:
            Py_RETURN_TRUE;
        case MuxResult_CONTINUE:
            Py_RETURN_TRUE;
        case MuxResult_EOF:
            Py_RETURN_FALSE;
        case MuxResult_ERROR:
        default:
            return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to mux video.");
    }
}

/**
 * ビデオをmuxする
 * @param instance  インスタンス
 * @return          結果
 */
MuxResult mux_video(MediaWriter* instance) {
    
    if(instance->send_vframe_completed && instance->receive_vpacket_completed)
        return MuxResult_EOF;
    
    //エンコーダーにフレームを送る
    if(!instance->send_vframe_completed) {

        switch(avcodec_send_frame(instance->vcodec_context, instance->vframe)) {
            case 0:
                break;
            case AVERROR(EAGAIN):
                return MuxResult_CONTINUE;
            case AVERROR_EOF:
                instance->send_vframe_completed = true;
                break;
            default:
                return MuxResult_ERROR;
        }
    }
    
    //エンコーダーからパケットを受け取る
    if(!instance->receive_vpacket_completed) {

        switch(avcodec_receive_packet(instance->vcodec_context, &instance->packet)) {
            case 0:
                break;
            case AVERROR(EAGAIN):
                return MuxResult_CONTINUE;
            case AVERROR_EOF:
                instance->receive_vpacket_completed = true;
                return MuxResult_EOF;
            default:
                return MuxResult_ERROR;
        }
    }

    //ヘッダを書き込む
    if(!instance->header_written) {

        if(avformat_write_header(instance->format_context, NULL) < 0)
            return MuxResult_ERROR;

        instance->header_written = true;
    }
    
    //パケットを書き込む
    av_packet_rescale_ts(&instance->packet, instance->vcodec_context->time_base, instance->vstream->time_base);
    instance->packet.stream_index = instance->vstream->index;

    if(av_interleaved_write_frame(instance->format_context, &instance->packet) < 0)
        return MuxResult_ERROR;

    av_packet_unref(&instance->packet);

    return MuxResult_OK;
}

/** オーディオフレームのエンコードを進める */
PyObject* MediaWriter_process_encode_audio(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    if(instance->astream == NULL)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Audio stream does not exist.");
    
    //引数をパースする
    int channels, nb_samples;
    const char* sample_format_name;
    Py_buffer samples;

    if(!PyArg_ParseTuple(args, "iisy*", &channels, &nb_samples, &sample_format_name, &samples))
        return NULL;

    //サンプルフォーマットIDを取得する
    enum AVSampleFormat sample_format;
    
    if((sample_format = av_get_sample_fmt(sample_format_name)) == AV_SAMPLE_FMT_NONE)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Cannot find sample format: %s", sample_format_name);
    
    if(sample_format != instance->aframe->format)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Sample format does not match width %s.", av_get_sample_fmt_name(instance->acodec_context->sample_fmt));
    
    //サンプルデータをオーディオバッファにコピーする
    if(!frame_copy_samples_from_buffer(instance->aframe, channels, nb_samples, sample_format, samples.buf, samples.len))
        return PyErr_Format(PpmpegExc_MediaWriterError, "Faile to copy samples to frame.");

    PyBuffer_Release(&samples);
    
    //フレームを準備し、PTSを設定する
    //@attention ストリームの time_base は FFmpeg によって変更される場合があるため利用しないこと。
    if(av_frame_make_writable(instance->aframe) < 0)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to make audio frame writablee.");

    instance->aframe->pts = av_rescale_q(instance->acodec_context->frame_number * instance->acodec_context->frame_size,
                                         av_make_q(1, instance->acodec_context->sample_rate), //@todo オーディオストリーム作成時にinstanceに保存しておいてもよい
                                         instance->acodec_context->time_base);
    instance->aframe->pts += instance->astart_time;

    //muxする
    switch(mux_audio(instance)) {
        case MuxResult_OK:
            Py_RETURN_TRUE;
        case MuxResult_CONTINUE:
            Py_RETURN_TRUE;
        case MuxResult_EOF:
            Py_RETURN_FALSE;
        case MuxResult_ERROR:
        default:
            return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to mux audio.");
    }
}

/**
 * オーディオをmuxする
 * @param instance  インスタンス
 * @return          結果
 */
MuxResult mux_audio(MediaWriter* instance) {
    
    if(instance->send_aframe_completed && instance->receive_apacket_completed)
        return MuxResult_EOF;

    //エンコーダーにフレームを送る
    if(!instance->send_aframe_completed) {

        switch(avcodec_send_frame(instance->acodec_context, instance->aframe)) {
            case 0:
                break;
            case AVERROR(EAGAIN):
                return MuxResult_CONTINUE;
            case AVERROR_EOF:
                instance->send_aframe_completed = true;
                break;
            default:
                return MuxResult_ERROR;
        }
    }
    
    //エンコーダーからパケットを受け取る
    if(!instance->receive_apacket_completed) {

        switch(avcodec_receive_packet(instance->acodec_context, &instance->packet)) {
            case 0:
                break;
            case AVERROR(EAGAIN):
                return MuxResult_CONTINUE;
            case AVERROR_EOF:
                instance->receive_apacket_completed = true;
                return MuxResult_EOF;
            default:
                return MuxResult_ERROR;
        }
    }

    //ヘッダを書き込む
    if(!instance->header_written) {

        if(avformat_write_header(instance->format_context, NULL) < 0)
            return MuxResult_ERROR;

        instance->header_written = true;
    }
    
    //パケットを書き込む
    av_packet_rescale_ts(&instance->packet, instance->acodec_context->time_base, instance->astream->time_base);
    instance->packet.stream_index = instance->astream->index;

    if(av_interleaved_write_frame(instance->format_context, &instance->packet) < 0)
        return MuxResult_ERROR;

    av_packet_unref(&instance->packet);
        
    return MuxResult_OK;
}

/** エンコードしたビデオフレーム数を返す */
PyObject* MediaWriter_get_encoded_video_frames(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    if(instance->vcodec_context == NULL)
        return PyLong_FromLong(0);
    
    return PyLong_FromLong(instance->vcodec_context->frame_number);
}

/** エンコードしたオーディオフレーム数を返す */
PyObject* MediaWriter_get_encoded_audio_frames(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);

    if(instance->acodec_context == NULL)
        return PyLong_FromLong(0);
        
    return PyLong_FromLong(instance->acodec_context->frame_number);
}

/** エンコード結果をフラッシュする */
PyObject* MediaWriter_flush_encode(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    CHECK_OPEN(instance);
    
    if(instance->flushed)
        Py_RETURN_NONE;

    //ビデオとオーディオのmuxを完了するまで実行する
    AVFrame* last_vframe = instance->vframe;
    instance->vframe = NULL;

    AVFrame* last_aframe = instance->aframe;    
    instance->aframe = NULL;

    bool video_flushed = (instance->vstream == NULL);
    bool audio_flushed = (instance->astream == NULL);
    
    while(!video_flushed || !audio_flushed) {
        
        if(!video_flushed) {

            switch(mux_video(instance)) {
                case MuxResult_OK:
                    break;
                case MuxResult_CONTINUE:
                    break;
                case MuxResult_EOF:
                    video_flushed = true;
                    break;
                case MuxResult_ERROR:
                default:
                    fprintf(stderr, "Warning: Failed to flush video.");
                    video_flushed = true;
                    break;
            }
        }

        if(!audio_flushed) {
        
            switch(mux_audio(instance)) {
                case MuxResult_OK:
                    break;
                case MuxResult_CONTINUE:
                    break;
                case MuxResult_EOF:
                    audio_flushed = true;
                    break;
                case MuxResult_ERROR:
                default:
                    fprintf(stderr, "Warning: Failed to flush audio.");
                    audio_flushed = true;
                    break;
            }
        }
    }

    instance->vframe = last_vframe;
    instance->aframe = last_aframe;

    //トレーラーを書き込む
    if(av_write_trailer(instance->format_context) < 0)
        return PyErr_Format(PpmpegExc_MediaWriterError, "Failed to write trailer.");
    
    instance->flushed = true;
    
    Py_RETURN_NONE;
}

/** クローズする */
PyObject* MediaWriter_close(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);
    
    if(!instance->opened || instance->closed)
        Py_RETURN_NONE;

    if(!instance->flushed)
        fprintf(stderr, "Warning: Flush is not called. Writed media may be imcomplete.");

    //オーディオフレームを解放する
    av_frame_free(&instance->aframe); 

    //ビデオフレームを解放する
    av_frame_free(&instance->vframe);

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
    
    //ファイルをクローズする
    if(!(instance->format_context->oformat->flags & AVFMT_NOFILE))
        avio_closep(&instance->format_context->pb);
    
    //フォーマットコンテキストを解放する
    avformat_free_context(instance->format_context);
    instance->format_context = NULL;
    
    instance->closed = true;
    
    Py_RETURN_NONE;
}

/** 破棄する */
PyObject* MediaWriter_del(PyObject* self, PyObject* args) {
    
    MediaWriter* instance = instance_get_bound(self);

    if(instance == NULL)
        Py_RETURN_NONE;

    if(instance->opened && !instance->closed)
        MediaWriter_close(self, NULL);

    PyMem_Free(instance);
    instance_unbind(self);

    Py_RETURN_NONE;
}
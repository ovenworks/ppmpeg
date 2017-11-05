/* 
 * media_util.c
 * @author: ppmpeg@ovenworks.jp
 */

#include "media_util.h"

#include <stdlib.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#include "util/instance.h"
#include "util/buffer.h"
#include "util/frame.h"
#include "util/codec.h"
#include "util/scalars.h"
#include "util/resamplers.h"

/**
 * メディアユーティリティ構造体
 * @author ppmpeg@ovenworks.jp
 */
typedef struct MediaUtil {
    
    uint8_t* vbuffer;   ///< ビデオバッファ
    int vbuffer_size;   ///< ビデオバッファのサイズ
    
    uint8_t* abuffer;   ///< オーディオバッファ
    int abuffer_size;   ///< オーディオバッファのサイズ
    
} MediaUtil;

static MediaUtil cinstance;                 ///< クラスインスタンス @todo Pythonのクラス自体にバインドできるようにしてinstance_xxx()使用に統一する
static PyObject* PpmpegExc_MediaUtilError; ///< 例外オブジェクト

/* MediaUtilError例外を作成する */
PyObject* MediaUtil_newMediaUtilError(const char* name) {

    PpmpegExc_MediaUtilError = PyErr_NewException(name, NULL, NULL);
    Py_INCREF(PpmpegExc_MediaUtilError);

    return PpmpegExc_MediaUtilError;
}

/** 初期化する */
PyObject* MediaUtil_init(PyObject* self, PyObject* args) {
    
    MediaUtil* instance = &cinstance;

    instance->vbuffer = NULL;
    instance->vbuffer_size = 0;
    
    instance->abuffer = NULL;
    instance->abuffer_size = 0;
    
    Py_RETURN_NONE;
}

/** ビデオピクセルフォーマットを変換する */
PyObject* MediaUtil_convert_pixels(PyObject* self, PyObject* args) {

    MediaUtil* instance = &cinstance;

    //引数をパースする
    int src_width, src_height, dst_width, dst_height;
    const char* src_format_name;
    Py_buffer src_pixels;
    const char* dst_format_name;

    if(!PyArg_ParseTuple(args, "iisy*iis",
                         &src_width, &src_height, &src_format_name, &src_pixels,
                         &dst_width, &dst_height, &dst_format_name)) {
        return NULL;
    }

    //ピクセルフォーマットIDを取得する
    enum AVPixelFormat src_format, dst_format;
    
    if((src_format = av_get_pix_fmt(src_format_name)) == AV_PIX_FMT_NONE)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Cannot find pixel format: %s", src_format_name);

    if((dst_format = av_get_pix_fmt(dst_format_name)) == AV_PIX_FMT_NONE)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Cannot find pixel format: %s", dst_format_name);

    //変換元のバッファを取得する
    uint8_t* src_buffers[4];
    int src_linesizes[4];
    
    int need_size = av_image_fill_arrays(src_buffers, src_linesizes, src_pixels.buf, src_format, src_width, src_height, 1);
    
    if(need_size < 0 || need_size > src_pixels.len)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to setup buffer.");        
    
    //変換先のバッファを確保する
    int dst_size;
    uint8_t* dst_buffers[4];
    int dst_linesizes[4];

    dst_size = av_image_get_buffer_size(dst_format, dst_width, dst_height, 1);
    instance->vbuffer_size = buffer_alloc(&instance->vbuffer, instance->vbuffer_size, dst_size);
    
    if(av_image_fill_arrays(dst_buffers, dst_linesizes, instance->vbuffer, dst_format, dst_width, dst_height, 1) < 0)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to setup buffer.");

    //変換する
    struct SwsContext* sws_context;
            
    if((sws_context = scalars_get(src_width,
                                  src_height,
                                  src_format,
                                  dst_width,
                                  dst_height,
                                  dst_format,
                                  0)) == NULL) {

        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to get scalars.");
    }

    if(scalars_safe_scale((const uint8_t*)src_pixels.buf,
                          need_size,
                          sws_context,
                          (const uint8_t**)src_buffers,
                          src_linesizes,
                          0,
                          src_height,
                          dst_buffers,
                          dst_linesizes) < 0) {
        
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to convert pixels.");
    }

    PyBuffer_Release(&src_pixels);
    
    return PyBytes_FromStringAndSize(dst_buffers[0], dst_size);
}

/** オーディオサンプルフォーマットを変換する */
PyObject* MediaUtil_convert_samples(PyObject* self, PyObject* args) {
    
    MediaUtil* instance = &cinstance;

    //引数をパースする
    int src_channels, src_nb_samples, dst_channels, dst_nb_samples;
    const char* src_format_name;
    Py_buffer src_samples;
    const char* dst_format_name;

    if(!PyArg_ParseTuple(args, "iisy*iis",
                         &src_channels, &src_nb_samples, &src_format_name, &src_samples,
                         &dst_channels, &dst_nb_samples, &dst_format_name)) {
        return NULL;
    }
    
    //サンプルフォーマットIDを取得する
    enum AVSampleFormat src_format, dst_format;
    
    if((src_format = av_get_sample_fmt(src_format_name)) == AV_SAMPLE_FMT_NONE)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Cannot find sample format: %s", src_format_name);

    if((dst_format = av_get_sample_fmt(dst_format_name)) == AV_SAMPLE_FMT_NONE)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Cannot find sample format: %s", dst_format_name);

    //変換元のバッファを取得する
    //@todo バッファサイズをチェックする
    uint8_t* src_buffers[src_channels];
    int src_linesize;
    
    int need_size = av_samples_fill_arrays(src_buffers, &src_linesize, src_samples.buf, src_channels, src_nb_samples, src_format, 1);
    
    if(need_size < 0 || need_size > src_samples.len)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to setup buffer.");        

    //変換先のバッファを確保する
    int dst_size;
    uint8_t* dst_buffers[dst_channels];
    int dst_linesize;

    dst_size = av_samples_get_buffer_size(NULL, dst_channels, dst_nb_samples, dst_format, 1);
    instance->abuffer_size = buffer_alloc(&instance->abuffer, instance->abuffer_size, dst_size);
    
    if(av_samples_fill_arrays(dst_buffers, &dst_linesize, instance->abuffer, dst_channels, dst_nb_samples, dst_format, 1) < 0)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to setup buffer.");                

    //変換する
    //@todo サンプルフォーマットのみを変換する場合にチャンネルレイアウトとサンプルレートがこれでよいかは追加検証が必要。
    struct SwrContext* swr_context;
    int src_channel_layout = av_get_default_channel_layout(src_channels);
    int src_sample_rate = 1;
    int dst_channel_layout = av_get_default_channel_layout(dst_channels);
    int dst_sample_rate = 1;

    if((swr_context = resamplers_get(src_channel_layout,
                                     src_sample_rate,
                                     src_format,
                                     dst_channel_layout,
                                     dst_sample_rate,
                                     dst_format)) == NULL) {

        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to get resamplers.");
    }
            
    if(swr_convert(swr_context,
                   dst_buffers,
                   dst_nb_samples,
                   (const uint8_t**)src_buffers,
                   src_nb_samples) < 0) {

        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to convert samples.");
    }

    PyBuffer_Release(&src_samples);
    
    return PyBytes_FromStringAndSize(dst_buffers[0], dst_size);
}

/** ビデオピクセルデータをエンコードする */
PyObject* MediaUtil_encode_pixels(PyObject* self, PyObject* args) {

    //MediaUtil* instance = &cinstance;

    //引数をパースする
    int width, height;
    const char* pixel_format_name;
    Py_buffer pixels;
    const char* codec_name;

    if(!PyArg_ParseTuple(args, "iisy*s", &width, &height, &pixel_format_name, &pixels, &codec_name))
        return NULL;

    //ピクセルフォーマットIDを取得する
    enum AVPixelFormat pixel_format;
    
    if((pixel_format = av_get_pix_fmt(pixel_format_name)) == AV_PIX_FMT_NONE)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Cannot find pixel format: %s", pixel_format_name);
        
    //コーデックを取得する
    AVCodec* codec;
    
    if((codec = avcodec_find_encoder_by_name(codec_name)) == NULL)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Cannot find encoder: %s", codec_name);

    if(!codec_is_support_pixel_format(codec, pixel_format))
        return PyErr_Format(PpmpegExc_MediaUtilError, "Unsupported pixel formatr: %s", pixel_format_name);
    
    //コーデックをオープンする
    AVCodecContext* context;
    
    if((context = avcodec_alloc_context3(codec)) == NULL)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to allocate codec context.");
    
    context->width = width;
    context->height = height;
    context->time_base = av_make_q(1, 1);
    context->pix_fmt = pixel_format;
        
    if(avcodec_open2(context, codec, NULL) < 0)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to open codec.");

    //フレームを確保する
    AVFrame* frame;
    
    if((frame = av_frame_alloc()) == NULL)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to allocate frame.");
        
    frame->width  = context->width;
    frame->height = context->height;
    frame->format = context->pix_fmt;

    //バッファを確保する
    if(av_frame_get_buffer(frame, 32) < 0)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to allocate video buffer.");

    //ピクセルデータをバッファにコピーする
    if(!frame_copy_pixels_from_buffer(frame, width, height, pixel_format, pixels.buf, pixels.len))
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to copy pixels to frame.");
    
    PyBuffer_Release(&pixels);

    //パケットを作成する
    AVPacket packet;

    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;

    //エンコードする
    if(avcodec_send_frame(context, frame) < 0)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to encode.");
        
    if(avcodec_receive_packet(context, &packet) < 0)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to encode.");

    PyObject* encoded = PyBytes_FromStringAndSize(packet.data, packet.size);

    //パケットを破棄する
    av_packet_unref(&packet);
    
    //フレームを解放する
    av_frame_free(&frame);

    //コーデックをクローズする
    avcodec_close(context);
    avcodec_free_context(&context);
    
    return encoded;
}

/** オーディオサンプルデータをエンコードする */
PyObject* MediaUtil_encode_samples(PyObject* self, PyObject* args) {

    //MediaUtil* instance = &cinstance;

    //引数をパースする
    int channels, nb_samples;
    const char* sample_format_name;
    Py_buffer samples;
    const char* codec_name;

    if(!PyArg_ParseTuple(args, "iisy*s", &channels, &nb_samples, &sample_format_name, &samples, &codec_name))
        return NULL;

    //サンプルフォーマットIDを取得する
    enum AVSampleFormat sample_format;
    
    if((sample_format = av_get_sample_fmt(sample_format_name)) == AV_PIX_FMT_NONE)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Cannot find sample format: %s", sample_format_name);
        
    //コーデックを取得する
    AVCodec* codec;
    
    if((codec = avcodec_find_encoder_by_name(codec_name)) == NULL)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Cannot find encoder: %s", codec_name);

    if(!codec_is_support_sample_format(codec, sample_format))
        return PyErr_Format(PpmpegExc_MediaUtilError, "Unsupported sample formatr: %s", sample_format_name);
    
    //コーデックをオープンする
    AVCodecContext* context;
    
    if((context = avcodec_alloc_context3(codec)) == NULL)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to allocate codec context.");
    
    context->channels = channels;
    context->channel_layout = av_get_default_channel_layout(channels);
    context->sample_rate = 1;
    context->time_base = av_make_q(1, 1);
    context->sample_fmt = sample_format;
        
    if(avcodec_open2(context, codec, NULL) < 0)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to open codec.");

    //フレームを確保する
    AVFrame* frame;
    
    if((frame = av_frame_alloc()) == NULL)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to allocate frame.");
        
    frame->channel_layout = context->channel_layout;
    frame->sample_rate = context->sample_rate;
    frame->nb_samples = context->frame_size;
    frame->format = context->sample_fmt;

    //バッファを確保する
    if(av_frame_get_buffer(frame, 32) < 0)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to allocate audio buffer.");

    //サンプルデータをバッファにコピーする
    if(!frame_copy_samples_from_buffer(frame, channels, nb_samples, sample_format, samples.buf, samples.len))
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to copy samples to frame.");
    
    PyBuffer_Release(&samples);

    //パケットを作成する
    AVPacket packet;

    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;

    //エンコードする
    if(avcodec_send_frame(context, frame) < 0)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to encode.");
        
    if(avcodec_receive_packet(context, &packet) < 0)
        return PyErr_Format(PpmpegExc_MediaUtilError, "Failed to encode.");

    PyObject* encoded = PyBytes_FromStringAndSize(packet.data, packet.size);

    //パケットを破棄する
    av_packet_unref(&packet);
    
    //フレームを解放する
    av_frame_free(&frame);

    //コーデックをクローズする
    avcodec_close(context);
    avcodec_free_context(&context);
    
    return encoded;
}

/** 破棄する */
PyObject* MediaUtil_del(PyObject* self, PyObject* args) {
    
    MediaUtil* instance = &cinstance;

    buffer_freep(&instance->abuffer);
    instance->abuffer_size = 0;
    
    buffer_freep(&instance->vbuffer);
    instance->vbuffer_size = 0;
    
//    PyMem_Free(instance);
        
    Py_RETURN_NONE;
}

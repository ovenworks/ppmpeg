/* 
 * ffmpeg.c
 * @author: ppmpeg@ovenworks.jp
 */

#include "ffmpeg.h"

#include <stdlib.h>
#include <stdbool.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include "util/instance.h"

/**
 * FFmpeg構造体
 * @author ppmpeg@ovenworks.jp
 */
typedef struct FFmpeg {

    int version;                ///< バージョン
    const char* configuration;  ///< 構成情報

} FFmpeg;

/** 初期化する */
PyObject* FFmpeg_init(PyObject* self, PyObject* args) {

    FFmpeg* instance = (FFmpeg*)PyMem_Malloc(sizeof(FFmpeg));
    instance->version = avcodec_version();
    instance->configuration = avcodec_configuration();
    
    instance_bind(self, instance);

    Py_RETURN_NONE;
}

/** バージョン情報を返す */
PyObject* FFmpeg_get_version(PyObject* self, PyObject* args) {
    
    FFmpeg* instance = instance_get_bound(self);

    return PyLong_FromLong(instance->version);
}

/** 構成情報を返す */
PyObject* FFmpeg_get_cofiguration(PyObject* self, PyObject* args) {

    FFmpeg* instance = instance_get_bound(self);

    return PyUnicode_FromString(instance->configuration);
}

/** ビデオコーデックのリストを返す */
PyObject* FFmpeg_get_video_codecs(PyObject* self, PyObject* args) {

    PyObject* result = PyList_New(0);
    
    for(AVCodec* codec = av_codec_next(NULL); codec != NULL; codec = av_codec_next(codec)) {

        if(codec->type == AVMEDIA_TYPE_VIDEO)
            PyList_Append(result, PyUnicode_FromString(codec->name));
    }
    
    return result;
}

/** ビデオピクセルフォーマットのリストを返す */
PyObject* FFmpeg_get_video_pixel_formats(PyObject* self, PyObject* args) {

    //引数をパースする
    const char* codec_name;

    if(!PyArg_ParseTuple(args, "s", &codec_name))
        return NULL;

    //
    AVCodec* encoder = avcodec_find_encoder_by_name(codec_name);
    AVCodec* decoder = avcodec_find_decoder_by_name(codec_name);

    if((encoder == NULL || encoder->type != AVMEDIA_TYPE_VIDEO) &&
       (decoder == NULL || decoder->type != AVMEDIA_TYPE_VIDEO)) {

        Py_RETURN_NONE;
    }

    PyObject* result = PyList_New(0);
    
    if(encoder != NULL) {
        for(const enum AVPixelFormat* fmt = encoder->pix_fmts; fmt != NULL && *fmt != -1; ++fmt) {
            PyList_Append(result, PyUnicode_FromString(av_get_pix_fmt_name(*fmt)));
        }
    }

    if(decoder != NULL) {
        for(const enum AVPixelFormat* fmt = decoder->pix_fmts; fmt != NULL && *fmt != -1; ++fmt) {
            PyList_Append(result, PyUnicode_FromString(av_get_pix_fmt_name(*fmt)));
        }
    }

    return result;
}

/** オーディオコーデックのリストを返す */
PyObject* FFmpeg_get_audio_codecs(PyObject* self, PyObject* args) {

    PyObject* result = PyList_New(0);
    
    for(AVCodec* codec = av_codec_next(NULL); codec != NULL; codec = av_codec_next(codec)) {

        if(codec->type == AVMEDIA_TYPE_AUDIO)
            PyList_Append(result, PyUnicode_FromString(codec->name));
    }
    
    return result;
}

/** オーディオサンプルフォーマットのリストを返す */
PyObject* FFmpeg_get_audio_sample_formats(PyObject* self, PyObject* args) {

    //引数をパースする
    const char* codec_name;

    if(!PyArg_ParseTuple(args, "s", &codec_name))
        return NULL;
    
    //
    AVCodec* encoder = avcodec_find_encoder_by_name(codec_name);
    AVCodec* decoder = avcodec_find_decoder_by_name(codec_name);

    if((encoder == NULL || encoder->type != AVMEDIA_TYPE_AUDIO) &&
       (decoder == NULL || decoder->type != AVMEDIA_TYPE_AUDIO)) {

        Py_RETURN_NONE;
    }

    PyObject* result = PyList_New(0);

    if(encoder != NULL) {
        for(const enum AVSampleFormat* fmt = encoder->sample_fmts; fmt != NULL && *fmt != -1; ++fmt) {
            PyList_Append(result, PyUnicode_FromString(av_get_sample_fmt_name(*fmt)));
        }
    }
    
    if(decoder != NULL) {
        for(const enum AVSampleFormat* fmt = decoder->sample_fmts; fmt != NULL && *fmt != -1; ++fmt) {
            PyList_Append(result, PyUnicode_FromString(av_get_sample_fmt_name(*fmt)));
        }
    }

    return result;
}
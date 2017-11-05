/* 
 * _native.c
 * @author: ppmpeg@ovenworks.jp
 */

#include <sys/time.h>
#include <Python.h>

#include "ffmpeg.h"
#include "media_reader.h"
#include "media_writer.h"
#include "media_util.h"

#include <libavformat/avformat.h>
#include "util/scalars.h"
#include "util/resamplers.h"

/**
 * モジュール状態構造体
 * @author ppmpeg@ovenworks.jp
 */
typedef struct ModuleState {

    PyObject *error;

} ModuleState;

/**
 * メソッド定義
 */
static PyMethodDef methods[] = {
    
    //FFmpeg
    {"FFmpeg_init", FFmpeg_init, METH_VARARGS},
    {"FFmpeg_get_version", FFmpeg_get_version, METH_VARARGS},
    {"FFmpeg_get_configuration", FFmpeg_get_cofiguration, METH_VARARGS},
    {"FFmpeg_get_video_codecs", FFmpeg_get_video_codecs, METH_VARARGS},
    {"FFmpeg_get_video_pixel_formats", FFmpeg_get_video_pixel_formats, METH_VARARGS},
    {"FFmpeg_get_audio_codecs", FFmpeg_get_audio_codecs, METH_VARARGS},
    {"FFmpeg_get_audio_sample_formats", FFmpeg_get_audio_sample_formats, METH_VARARGS},
    
    //MediaReader
    {"MediaReader_init", MediaReader_init, METH_VARARGS},

    {"MediaReader_open", MediaReader_open, METH_VARARGS},
    
    {"MediaReader_get_metadata", MediaReader_get_metadata, METH_VARARGS},
    {"MediaReader_get_duration", MediaReader_get_duration, METH_VARARGS},
    {"MediaReader_get_bit_rate", MediaReader_get_bit_rate, METH_VARARGS},

    {"MediaReader_has_video_stream", MediaReader_has_video_stream, METH_VARARGS},
    {"MediaReader_get_video_width", MediaReader_get_video_width, METH_VARARGS},
    {"MediaReader_get_video_height", MediaReader_get_video_height, METH_VARARGS},
    {"MediaReader_get_video_pixel_format", MediaReader_get_video_pixel_format, METH_VARARGS},
    {"MediaReader_get_video_frame_rate", MediaReader_get_video_frame_rate, METH_VARARGS},
    {"MediaReader_get_video_bit_rate", MediaReader_get_video_bit_rate, METH_VARARGS},
    {"MediaReader_get_video_gop_size", MediaReader_get_video_gop_size, METH_VARARGS},
    {"MediaReader_get_video_start_time", MediaReader_get_video_start_time, METH_VARARGS},
    {"MediaReader_get_video_codec", MediaReader_get_video_codec, METH_VARARGS},

    {"MediaReader_has_audio_stream", MediaReader_has_audio_stream, METH_VARARGS},    
    {"MediaReader_get_audio_channels", MediaReader_get_audio_channels, METH_VARARGS},
    {"MediaReader_get_audio_nb_samples", MediaReader_get_audio_nb_samples, METH_VARARGS},
    {"MediaReader_get_audio_sample_format", MediaReader_get_audio_sample_format, METH_VARARGS},
    {"MediaReader_get_audio_sample_rate", MediaReader_get_audio_sample_rate, METH_VARARGS},
    {"MediaReader_get_audio_bit_rate", MediaReader_get_audio_bit_rate, METH_VARARGS},
    {"MediaReader_get_audio_start_time", MediaReader_get_audio_start_time, METH_VARARGS},
    {"MediaReader_get_audio_codec", MediaReader_get_audio_codec, METH_VARARGS},

    {"MediaReader_process_decode", MediaReader_process_decode, METH_VARARGS},
    {"MediaReader_get_decoded_video_frames", MediaReader_get_decoded_video_frames, METH_VARARGS},
    {"MediaReader_get_decoded_audio_frames", MediaReader_get_decoded_audio_frames, METH_VARARGS},

    {"MediaReader_close", MediaReader_close, METH_VARARGS},
    
    {"MediaReader_del", MediaReader_del, METH_VARARGS},
    
    //MediaWriter
    {"MediaWriter_init", MediaWriter_init, METH_VARARGS},

    {"MediaWriter_open", MediaWriter_open, METH_VARARGS},

    {"MediaWriter_put_metadata", MediaWriter_put_metadata, METH_VARARGS},
//    {"MediaReader_get_duration", MediaReader_get_duration, METH_VARARGS},
//    {"MediaReader_get_bit_rate", MediaReader_get_bit_rate, METH_VARARGS},

    {"MediaWriter_add_video_stream", MediaWriter_add_video_stream, METH_VARARGS},
    {"MediaWriter_has_video_stream", MediaWriter_has_video_stream, METH_VARARGS},
    {"MediaWriter_get_video_width", MediaWriter_get_video_width, METH_VARARGS},
    {"MediaWriter_get_video_height", MediaWriter_get_video_height, METH_VARARGS},
    {"MediaWriter_get_video_pixel_format", MediaWriter_get_video_pixel_format, METH_VARARGS},
    {"MediaWriter_get_video_frame_rate", MediaWriter_get_video_frame_rate, METH_VARARGS},
    {"MediaWriter_get_video_bit_rate", MediaWriter_get_video_bit_rate, METH_VARARGS},
    {"MediaWriter_get_video_gop_size", MediaWriter_get_video_gop_size, METH_VARARGS},
    {"MediaWriter_get_video_start_time", MediaWriter_get_video_start_time, METH_VARARGS},
    {"MediaWriter_get_video_codec", MediaWriter_get_video_codec, METH_VARARGS},

    {"MediaWriter_add_audio_stream", MediaWriter_add_audio_stream, METH_VARARGS},
    {"MediaWriter_has_audio_stream", MediaWriter_has_audio_stream, METH_VARARGS},    
    {"MediaWriter_get_audio_channels", MediaWriter_get_audio_channels, METH_VARARGS},
    {"MediaWriter_get_audio_nb_samples", MediaWriter_get_audio_nb_samples, METH_VARARGS},
    {"MediaWriter_get_audio_sample_format", MediaWriter_get_audio_sample_format, METH_VARARGS},
    {"MediaWriter_get_audio_sample_rate", MediaWriter_get_audio_sample_rate, METH_VARARGS},
    {"MediaWriter_get_audio_bit_rate", MediaWriter_get_audio_bit_rate, METH_VARARGS},
    {"MediaWriter_get_audio_start_time", MediaWriter_get_audio_start_time, METH_VARARGS},
    {"MediaWriter_get_audio_codec", MediaWriter_get_audio_codec, METH_VARARGS},

    {"MediaWriter_process_encode_video", MediaWriter_process_encode_video, METH_VARARGS},
    {"MediaWriter_process_encode_audio", MediaWriter_process_encode_audio, METH_VARARGS},
    {"MediaWriter_get_encoded_video_frames", MediaWriter_get_encoded_video_frames, METH_VARARGS},
    {"MediaWriter_get_encoded_audio_frames", MediaWriter_get_encoded_audio_frames, METH_VARARGS},
    {"MediaWriter_flush_encode", MediaWriter_flush_encode, METH_VARARGS},

    {"MediaWriter_close", MediaWriter_close, METH_VARARGS},
    
    {"MediaWriter_del", MediaWriter_del, METH_VARARGS},
    
    //MediaUtil
    {"MediaUtil_init", MediaUtil_init, METH_VARARGS},
    {"MediaUtil_convert_pixels", MediaUtil_convert_pixels, METH_VARARGS},
    {"MediaUtil_convert_samples", MediaUtil_convert_samples, METH_VARARGS},
    {"MediaUtil_encode_pixels", MediaUtil_encode_pixels, METH_VARARGS},
    {"MediaUtil_encode_samples", MediaUtil_encode_samples, METH_VARARGS},
    {"MediaUtil_del", MediaUtil_del, METH_VARARGS},
    
    {NULL},
};

/**
 * モジュール定義
 */
static int module_traverse(PyObject* m, visitproc visit, void *arg);
static int module_clear(PyObject* m);

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "_native",
    NULL,
    sizeof(ModuleState),
    methods,
    NULL,
    module_traverse,
    module_clear,
    NULL
};

/** モジュールを初期化する */
PyMODINIT_FUNC PyInit__native() {

    PyObject *module = PyModule_Create(&moduledef);
    
    if(module == NULL)
        return NULL;

    //例外オブジェクトを作成して登録する
    PyModule_AddObject(module, "MediaReaderError", MediaReader_newMediaReaderError("_native.MediaReaderError"));
    PyModule_AddObject(module, "MediaWriterError", MediaWriter_newMediaWriterError("_native.MediaWriterError"));
    PyModule_AddObject(module, "MediaUtilError", MediaUtil_newMediaUtilError("_native.MediaUtilError"));
    
    //AVライブラリを初期化する
    av_register_all();
    avformat_network_init();

    //スケーラーマネージャーを初期化する
    scalars_init();
    
    //リサンプラーマネージャーを初期化する
    resamplers_init();
    
    return module;
}

/** モジュールをトラバースする */
int module_traverse(PyObject* m, visitproc visit, void *arg) {
    Py_VISIT(((ModuleState*)PyModule_GetState(m))->error);
    return 0;
}

/** モジュールをクリアする */
int module_clear(PyObject* m) {

    Py_CLEAR(((ModuleState*)PyModule_GetState(m))->error);

    //リサンプラーマネージャーを破棄する
    resamplers_dispose();
    
    //スケーラーマネージャーを破棄する
    scalars_dispose();

    return 0;
}

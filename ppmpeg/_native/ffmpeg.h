/* 
 * ffmpeg.h
 * @author: ppmpeg@ovenworks.jp
 */

#ifndef FFMPEG_H
#define FFMPEG_H

#include <sys/time.h>
#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

PyObject* FFmpeg_init(PyObject* self, PyObject* args);
PyObject* FFmpeg_get_version(PyObject* self, PyObject* args);
PyObject* FFmpeg_get_cofiguration(PyObject* self, PyObject* args);
PyObject* FFmpeg_get_video_codecs(PyObject* self, PyObject* args);
PyObject* FFmpeg_get_video_pixel_formats(PyObject* self, PyObject* args);
PyObject* FFmpeg_get_audio_codecs(PyObject* self, PyObject* args);
PyObject* FFmpeg_get_audio_sample_formats(PyObject* self, PyObject* args);

#ifdef __cplusplus
}
#endif

#endif /* FFMPEG_H */


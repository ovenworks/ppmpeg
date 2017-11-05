/* 
 * media_reader.h
 * @author: ppmpeg@ovenworks.jp
 */

#ifndef MEDIA_READER_H
#define MEDIA_READER_H

#include <sys/time.h>
#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

PyObject* MediaReader_newMediaReaderError(const char* name);
    
PyObject* MediaReader_init(PyObject* self, PyObject* args);

PyObject* MediaReader_open(PyObject* self, PyObject* args);

PyObject* MediaReader_get_metadata(PyObject* self, PyObject* args);
PyObject* MediaReader_get_duration(PyObject* self, PyObject* args);
PyObject* MediaReader_get_bit_rate(PyObject* self, PyObject* args);

PyObject* MediaReader_has_video_stream(PyObject* self, PyObject* args);
PyObject* MediaReader_get_video_width(PyObject* self, PyObject* args);
PyObject* MediaReader_get_video_height(PyObject* self, PyObject* args);
PyObject* MediaReader_get_video_pixel_format(PyObject* self, PyObject* args);
PyObject* MediaReader_get_video_frame_rate(PyObject* self, PyObject* args);
PyObject* MediaReader_get_video_bit_rate(PyObject* self, PyObject* args);
PyObject* MediaReader_get_video_gop_size(PyObject* self, PyObject* args);
PyObject* MediaReader_get_video_start_time(PyObject* self, PyObject* args);
PyObject* MediaReader_get_video_codec(PyObject* self, PyObject* args);

PyObject* MediaReader_has_audio_stream(PyObject* self, PyObject* args);
PyObject* MediaReader_get_audio_channels(PyObject* self, PyObject* args);
PyObject* MediaReader_get_audio_nb_samples(PyObject* self, PyObject* args);
PyObject* MediaReader_get_audio_sample_format(PyObject* self, PyObject* args);
PyObject* MediaReader_get_audio_sample_rate(PyObject* self, PyObject* args);
PyObject* MediaReader_get_audio_bit_rate(PyObject* self, PyObject* args);
PyObject* MediaReader_get_audio_start_time(PyObject* self, PyObject* args);
PyObject* MediaReader_get_audio_codec(PyObject* self, PyObject* args);

PyObject* MediaReader_process_decode(PyObject* self, PyObject* args);
PyObject* MediaReader_get_decoded_video_frames(PyObject* self, PyObject* args);
PyObject* MediaReader_get_decoded_audio_frames(PyObject* self, PyObject* args);

PyObject* MediaReader_close(PyObject* self, PyObject* args);

PyObject* MediaReader_del(PyObject* self, PyObject* args);

#ifdef __cplusplus
}
#endif

#endif /* MEDIA_READER_H */


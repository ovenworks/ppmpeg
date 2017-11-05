/* 
 * media_writer.h
 * @author: ppmpeg@ovenworks.jp
 */

#ifndef MEDIA_WRITER_H
#define MEDIA_WRITER_H

#include <sys/time.h>
#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

PyObject* MediaWriter_newMediaWriterError(const char* name);

PyObject* MediaWriter_init(PyObject* self, PyObject* args);

PyObject* MediaWriter_open(PyObject* self, PyObject* args);

PyObject* MediaWriter_put_metadata(PyObject* self, PyObject* args);
//PyObject* MediaReader_get_duration(PyObject* self, PyObject* args);
//PyObject* MediaReader_get_bit_rate(PyObject* self, PyObject* args);

PyObject* MediaWriter_add_video_stream(PyObject* self, PyObject* args);
PyObject* MediaWriter_has_video_stream(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_video_width(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_video_height(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_video_pixel_format(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_video_frame_rate(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_video_bit_rate(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_video_gop_size(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_video_start_time(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_video_codec(PyObject* self, PyObject* args);

PyObject* MediaWriter_add_audio_stream(PyObject* self, PyObject* args);
PyObject* MediaWriter_has_audio_stream(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_audio_channels(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_audio_nb_samples(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_audio_sample_format(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_audio_sample_rate(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_audio_bit_rate(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_audio_start_time(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_audio_codec(PyObject* self, PyObject* args);

PyObject* MediaWriter_process_encode_video(PyObject* self, PyObject* args);
PyObject* MediaWriter_process_encode_audio(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_encoded_video_frames(PyObject* self, PyObject* args);
PyObject* MediaWriter_get_encoded_audio_frames(PyObject* self, PyObject* args);
PyObject* MediaWriter_flush_encode(PyObject* self, PyObject* args);

PyObject* MediaWriter_close(PyObject* self, PyObject* args);

PyObject* MediaWriter_del(PyObject* self, PyObject* args);


#ifdef __cplusplus
}
#endif

#endif /* MEDIA_WRITER_H */


/* 
 * media_util.h
 * @author: ppmpeg@ovenworks.jp
 */

#ifndef MEDIA_UTIL_H
#define MEDIA_UTIL_H

#include <sys/time.h>
#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

PyObject* MediaUtil_newMediaUtilError(const char* name);

PyObject* MediaUtil_init(PyObject* self, PyObject* args);

PyObject* MediaUtil_convert_pixels(PyObject* self, PyObject* args);
PyObject* MediaUtil_convert_samples(PyObject* self, PyObject* args);

PyObject* MediaUtil_encode_pixels(PyObject* self, PyObject* args);
PyObject* MediaUtil_encode_samples(PyObject* self, PyObject* args);

PyObject* MediaUtil_del(PyObject* self, PyObject* args);

#ifdef __cplusplus
}
#endif

#endif /* MEDIA_UTIL_H */


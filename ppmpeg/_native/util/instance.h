/* 
 * instance.h
 * @author: ppmpeg@ovenworks.jp
 */

#ifndef INSTANCE_H
#define INSTANCE_H

#include <sys/time.h>
#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

void instance_bind(PyObject* self, void* instance);
void* instance_get_bound(PyObject* self);
void instance_unbind(PyObject* self);

#ifdef __cplusplus
}
#endif

#endif /* INSTANCE_H */


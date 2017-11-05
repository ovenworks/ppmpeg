/* 
 * instance.c
 * @author: ppmpeg@ovenworks.jp
 */

#include "instance.h"

/**
 * インスタンスをPythonオブジェクトにバインドする
 * @param self      Pythonオブジェクト
 * @param instance  インスタンス
 */
void instance_bind(PyObject* self, void* instance) {

    PyObject* capsule = PyCapsule_New(instance, "_instance", instance_unbind);
    PyObject_SetAttrString(self, "__native_instance", capsule);    
}

/**
 * Pythonオブジェクトにバインドされているインスタンスを返す
 * @param self  Pythonオブジェクト
 * @return      インスタンス。バインドされていない場合はNULL
 */
void* instance_get_bound(PyObject* self) {
    
    PyObject* capsule = PyObject_GetAttrString(self, "__native_instance");
    
    if(capsule == NULL)
        return NULL;
    
    return PyCapsule_GetPointer(capsule, "_instance");
}

/**
 * インスタンスをPythonオブジェクトからアンバインドする
 * @param self  Pythonオブジェクト
 */
void instance_unbind(PyObject* self) {

    PyObject_DelAttrString(self, "__native_instance");
}
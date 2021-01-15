#ifndef GUIDEPYCLI_H
#define GUIDEPYCLI_H

#include <python2.7/Python.h>
#include "guide.grpc.pb.h"

extern PyMethodDef GuidePyCliMethods[];

PyMODINIT_FUNC initguidepycli();
PyAPI_FUNC(PyObject *) guidepycli_make_stub(PyObject *, PyObject *);
PyAPI_FUNC(PyObject *) guidepycli_get_next_step(PyObject *, PyObject *);

#endif
/* @(#)PyInterpolator.h
 */

#ifndef _PYINTERPOLATOR_H
#define _PYINTERPOLATOR_H 1

#include "vtkPython.h"
#include <structmember.h>

#include "Interpolator.h"
#include "ConsistentInterpolator.h"

extern "C" {
  
  typedef struct {
    PyObject_HEAD
    Interpolator *interpolator_ptr;    // pointer to the C++ object
  } PyInterpolator ;
  
  void PyInterpolator_dealloc(PyInterpolator*);
  int PyInterpolator_init(PyInterpolator*, PyObject*, PyObject*);
  PyObject* PyInterpolator_call(PyObject*, PyObject*, PyObject*);
  PyObject* PyInterpolator_InterpolatePoint(PyObject*, PyObject*);
  PyObject* PyInterpolator_new(PyTypeObject*, PyObject*, PyObject*);
  PyObject* PyInterpolator_getDataSource(PyInterpolator*, void*);
  int PyInterpolator_setDataSource(PyInterpolator*, PyObject*, void*);
  
  static PyGetSetDef PyInterpolator_getseters[] = {
    {(char *)"DataSource",(getter) PyInterpolator_getDataSource, (setter) PyInterpolator_setDataSource, (char *)"vtk UnstructuredGrid data source", NULL},
    {NULL}  /* Sentinel */
  };
  
  static PyMemberDef PyInterpolator_members[] = {
    {NULL}  /* Sentinel */
  };  

  static PyMethodDef PyInterpolator_methods [] = {
    { (char *)"InterpolatePoint", (PyCFunction) PyInterpolator_InterpolatePoint, METH_VARARGS, (char*)"Evaluate point."},
    {NULL} /* Sentinel */
  };
  
  static PyTypeObject PyInterpolatorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "vtk_extras.Interpolator",             /* tp_name */
    sizeof(PyInterpolator), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PyInterpolator_dealloc,        /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    PyInterpolator_call,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "vtk interpolator object", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyInterpolator_methods,             /* tp_methods */
    PyInterpolator_members,             /* tp_members */
    PyInterpolator_getseters,           /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyInterpolator_init,      /* tp_init */
    0,                         /* tp_alloc */
    PyInterpolator_new,                 /* tp_new */
  };
}

#endif /* _PYINTERPOLATOR_H */


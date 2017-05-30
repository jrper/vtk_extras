#include "PyInterpolator.h"
#include "vtkPythonArgs.h"

#include "vtkUnstructuredGrid.h"

#include "vtkExtrasErrors.h"

extern "C" {
  
  void PyInterpolator_dealloc(PyInterpolator* self)
  {
    self->interpolator_ptr->Delete();
    Py_TYPE(self)->tp_free((PyObject*)self);
  }

  PyObject * PyInterpolator_new(PyTypeObject *type, PyObject *args,
				       PyObject *kwds)
  {
    PyInterpolator *self;
    
    self = (PyInterpolator *)type->tp_alloc(type, 0);
    if (self != NULL) {
      // deal with allocation of members
      self->interpolator_ptr = (Interpolator*) ConsistentInterpolator::New();
    }
    
    return (PyObject *)self;
  }

  int PyInterpolator_init(PyInterpolator *self, PyObject *args,
				 PyObject *kwds)
  {
    return 0;
  }
 



  PyObject* PyInterpolator_getDataSource(PyInterpolator *self, void *closure) {
    return vtkPythonUtil::GetObjectFromPointer(self->interpolator_ptr->GetDataSource());
  }

  int PyInterpolator_setDataSource(PyInterpolator *self, PyObject *value, void *closure) {

    vtkUnstructuredGrid* ugrid = (vtkUnstructuredGrid*) vtkPythonUtil::GetPointerFromObject(value, "vtkUnstructuredGrid");
    self->interpolator_ptr->SetDataSource(ugrid);
    
    return 0;}

  PyObject* PyInterpolator_call(PyObject *self, PyObject *args, PyObject *kw) {

    vtkPythonArgs argument_parser(args, "extras_interpolate");
    vtkUnstructuredGrid *input;   

    if (!argument_parser.GetVTKObject(input, "vtkUnstructuredGrid")) {
      PyErr_SetString(PyExc_TypeError, "Need VTK unstructured grid as first argument");
      return NULL;
    }
    
    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
    
    ((PyInterpolator*) self)->interpolator_ptr->Interpolate(input, ugrid);
    // The object below is what we'll return (this seems to add a reference)
    PyObject* pyugrid = vtkPythonUtil::GetObjectFromPointer(ugrid);

    // Clean up our spare reference now (or you could use smart pointers)
    ugrid->Delete();

    // Now back to Python
    return pyugrid;
  }

  PyObject* PyInterpolator_InterpolatePoint(PyObject *self, PyObject *args) {

    vtkPythonArgs argument_parser(args, "extras_interpolate_point");
    double x[3];

    if (!argument_parser.GetArray(x, 3)) {
      PyErr_SetString(PyExc_TypeError, "Need VTK unstructured grid as first argument");
      return NULL;
    }

    double val;
    
    ((PyInterpolator*) self)->interpolator_ptr->InterpolatePoint(x,&val);
    // The object below is what we'll return (this seems to add a reference)
    PyObject* pyout = PyFloat_FromDouble(val);

    // Now back to Python
    return pyout;
  }

}

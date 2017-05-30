#include "vtkPythonArgs.h"
#include "vtkUnstructuredGrid.h"
#include "vtkDoubleArray.h"
#include "stdio.h"

#include "vtkExtrasErrors.h"
#include "GmshWriter.h"
#include "GmshReader.h"
#include "PyInterpolator.h"
#include "MergePoints.h"
#include "BoundingSurface.h"
#include "Distance.h"

extern "C" {

  static PyObject *extras_distance(PyObject *self, PyObject *args) {

    vtkPythonArgs argument_parser(args, "extras_distance");
    vtkUnstructuredGrid *mesh, *boundary;    

    if (!argument_parser.GetVTKObject(mesh, "vtkUnstructuredGrid")) {
      PyErr_SetString(PyExc_TypeError, "Need VTK unstructured grid as first argument");
      return NULL;
    }
    if (!argument_parser.GetVTKObject(boundary, "vtkUnstructuredGrid")) {
      PyErr_SetString(PyExc_TypeError, "Need VTK unstructured grid as second argument");
      return NULL;
    }
    
    // apply our function

    vtkDoubleArray* dist = vtkDoubleArray::New();
    Distance* distance = Distance::New();

    distance->SetBoundary(boundary);
    int flag = distance->CalculateDistance(mesh, dist);
    distance->Delete();

    if (flag == -1)  Py_RETURN_NONE;

    // The object below is what we'll return (this seems to add a reference)
    PyObject* pydist = vtkPythonUtil::GetObjectFromPointer(dist);

    // Clean up our spare reference now (or you could use smart pointers)
    dist->Delete();

    // Now back to Python
    return pydist;
  }

  char distance_docstring[] = "Distance(vtkUnstructuredGrid mesh, vtkUnstructuredGrid boundary) -> vtkDoubleArray\n\n Get distance of mesh from specified boundary.";

  static PyObject *extras_bounding_surface(PyObject *self, PyObject *args) {

    vtkPythonArgs argument_parser(args, "extras_bounding_surface");
    vtkUnstructuredGrid *input;    

    if (!argument_parser.GetVTKObject(input, "vtkUnstructuredGrid")) {
      PyErr_SetString(PyExc_TypeError, "Need VTK unstructured grid as first argument");
      return NULL;
    }
    
    // apply our function

    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
    BoundingSurface* boundary = BoundingSurface::New();
    boundary->GetSurface(input, ugrid);
    boundary->Delete();

    // The object below is what we'll return (this seems to add a reference)
    PyObject* pyugrid = vtkPythonUtil::GetObjectFromPointer(ugrid);

    // Clean up our spare reference now (or you could use smart pointers)
    ugrid->Delete();

    // Now back to Python
    return pyugrid;
  }

  char bounding_surface_docstring[] = "ReadGmsh(vtkUnstructuredGrid) -> vtkUnstructuredGrid\n\n Extract the boundary from a VTK unstructured grid object.";

  static PyObject *extras_mergePoints(PyObject *self, PyObject *args, PyObject *kw) {

    vtkPythonArgs argument_parser(args, "extras_interpolate");
    vtkUnstructuredGrid *input, *source;    

    if (!argument_parser.GetVTKObject(input, "vtkUnstructuredGrid")) {
      PyErr_SetString(PyExc_TypeError, "Need VTK unstructured grid as first argument");
      return NULL;
    }
    PyObject* tmp = NULL;
    int degree = 1, continuity=1, mapper=REMAP;
    if(kw) {
      tmp = PyDict_GetItemString(kw, "degree");
      if (tmp)  degree = (int) PyInt_AsLong(tmp);
      tmp = PyDict_GetItemString(kw, "continuity");
      if (tmp) continuity = (int) PyInt_AsLong(tmp);
      tmp = PyDict_GetItemString(kw, "map_method");
      if (tmp) {
	char* cmapper=PyString_AsString(tmp);
	if (std::string("remap").compare(cmapper)==0) mapper=REMAP;
	if (std::string("project").compare(cmapper)==0) mapper=PROJECT;
      }	
    }
      
    
    // apply our function

    vtkUnstructuredGrid* output = vtkUnstructuredGrid::New();
    MergePoints* mergePoints = MergePoints::New();
    mergePoints->SetDegree(degree);
    mergePoints->SetContinuity(continuity);
    mergePoints->SetMapper(mapper);
    mergePoints->Merge(input, output);
    mergePoints->Delete();

    // The object below is what we'll return (this seems to add a reference)
    PyObject* pyugrid = vtkPythonUtil::GetObjectFromPointer(output);

    // Clean up our spare reference now (or you could use smart pointers)
    output->Delete();

    // Now back to Python
    return pyugrid;
  }

  char merge_points_docstring[] = "MergePoints(vtkUnstructuredGrid) -> vtkUnstructuredGrid\n\nDerive continuous data from a VTK unstructured grid object.";

  static PyObject *extras_interpolate(PyObject *self, PyObject *args) {

    vtkPythonArgs argument_parser(args, "extras_interpolate");
    vtkUnstructuredGrid *input, *source;    

    if (!argument_parser.GetVTKObject(input, "vtkUnstructuredGrid")) {
      PyErr_SetString(PyExc_TypeError, "Need VTK unstructured grid as first argument");
      return NULL;
    }
    if (!argument_parser.GetVTKObject(source, "vtkUnstructuredGrid")) {
      PyErr_SetString(PyExc_TypeError, "Need VTK unstructured grid  as second argument");
      return NULL;
    }
    
    // apply our function

    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
    ConsistentInterpolator* interpolator = ConsistentInterpolator::New();
    interpolator->SetDataSource(source);
    interpolator->Interpolate(input, ugrid);
    interpolator->Delete();

    // The object below is what we'll return (this seems to add a reference)
    PyObject* pyugrid = vtkPythonUtil::GetObjectFromPointer(ugrid);

    // Clean up our spare reference now (or you could use smart pointers)
    ugrid->Delete();

    // Now back to Python
    return pyugrid;
  }

  char gmsh_interpolate_docstring[] = "ReadGmsh(vtkUnstructuredGrid, vtkUnstructuredGrid) -> vtkUnstructuredGrid\n\nInterpolate the data from one VTK unstructured grid object onto the geometry of another.";

 static PyObject *extras_readgmsh(PyObject *self, PyObject *args) {

    vtkPythonArgs argument_parser(args, "extras_writegmsh");
    char* FileName;
    
    if (!argument_parser.GetValue(FileName)) {
      PyErr_SetString(PyExc_TypeError, "Need string as argument");
      return NULL;
    }
    
    // apply our function

    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
    GmshReader* reader = GmshReader::New();
    reader->SetFileName(FileName);
    ErrorCode ierr  = reader->ReadFile(ugrid);
    delete reader;

    if (ierr == RETURN_FAIL_IO)
      {
    	PyErr_SetString(PyExc_IOError, "Couldn't write file");
    	return NULL;
      }

    // The object below is what we'll return (this seems to add a reference)
    PyObject* pyugrid = vtkPythonUtil::GetObjectFromPointer(ugrid);

    // Clean up our spare reference now (or you could use smart pointers)
    ugrid->Delete();

    // Now back to Python
    return pyugrid;
  }

  char gmsh_read_docstring[] = "ReadGmsh(str filename) -> vtkUnstructuredGrid\n\nWrite the mesh from a vtkUnstructuredGrid object to a gmsh format.";

  static PyObject *extras_writegmsh(PyObject *self, PyObject *args) {

    vtkPythonArgs argument_parser(args, "extras_writegmsh");
    vtkUnstructuredGrid* ugrid;
    char* FileName;
    bool isBinary=1;
    
    if (!argument_parser.GetVTKObject(ugrid, "vtkUnstructuredGrid")) {
      PyErr_SetString(PyExc_TypeError, "Need VTK unstructured grid as first argument");
      return NULL;
    }
    if (!argument_parser.GetValue(FileName)) {
      PyErr_SetString(PyExc_TypeError, "Need string  as second argument");
      return NULL;
    }
    if (argument_parser.GetArgCount(args)==3){
      argument_parser.GetValue(isBinary);
    }
    
    // apply our function

    GmshWriter* writer = GmshWriter::New();
    writer->SetFileName(FileName);
    writer->SetBinaryWriteMode(isBinary);
    int ierr = writer->Write(ugrid);
    delete writer;
    if (!ierr)
      {
	PyErr_SetString(PyExc_IOError, "Couldn't write file");
	return NULL;
      }

    // Now back to Python
    Py_RETURN_NONE;
  }

  char gmsh_write_docstring[] = "WriteGmsh(vtkUnstructuredGrid ugrid, str filename, bool BinaryWriteMode=True)\n\nWrite the mesh from a vtkUnstructuredGrid object to a gmsh format.";

  static PyMethodDef extrasMethods[] = {
    { (char *)"Distance", (PyCFunction) extras_distance, METH_VARARGS, distance_docstring},
    { (char *)"BoundingSurface", (PyCFunction) extras_bounding_surface, METH_VARARGS, bounding_surface_docstring},
    { (char *)"Interpolate", (PyCFunction) extras_interpolate, METH_VARARGS, gmsh_interpolate_docstring},
    { (char *)"MergePoints", (PyCFunction) extras_mergePoints, METH_VARARGS| METH_KEYWORDS, merge_points_docstring},
    { (char *)"WriteGmsh", (PyCFunction) extras_writegmsh, METH_VARARGS, gmsh_write_docstring},
    { (char *)"ReadGmsh", (PyCFunction) extras_readgmsh, METH_VARARGS, gmsh_read_docstring},
    { NULL, NULL, 0, NULL }
  };

  static char extrasDocString[] = "Module collecting python wrappers to VTK stuff.";

  PyMODINIT_FUNC initvtk_extras() {

    if (PyType_Ready(&PyInterpolatorType) < 0)
      return;
    
    PyObject* m = Py_InitModule3("vtk_extras", extrasMethods, extrasDocString);
    if (m == NULL) return;

    PyObject* vtk = PyImport_ImportModule("vtk");

    Py_INCREF(&PyInterpolatorType);
    PyModule_AddObject(m,"Interpolator", (PyObject*)&PyInterpolatorType);

    Py_INCREF(vtk);
    PyModule_AddObject(m,"vtk",vtk);
    
    
  }

}

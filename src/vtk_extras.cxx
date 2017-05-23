#include "vtkPythonArgs.h"
#include "vtkUnstructuredGrid.h"
#include "stdio.h"

#include "GmshWriter.h"
#include "GmshReader.h"
#include "ConsistentInterpolation.h"

extern "C" {

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
    ConsistentInterpolation* interpolator = ConsistentInterpolation::New();
    interpolator->Interpolate(input,source, ugrid);
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
    reader->ReadFile(ugrid);
    delete reader;

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
    writer->Write(ugrid);
    delete writer;

    // Now back to Python
    Py_RETURN_NONE;
  }

  char gmsh_write_docstring[] = "WriteGmsh(vtkUnstructuredGrid ugrid, str filename, bool BinaryWriteMode=True)\n\nWrite the mesh from a vtkUnstructuredGrid object to a gmsh format.";

  static PyMethodDef extrasMethods[] = {
    { (char *)"Interpolate", (PyCFunction) extras_interpolate, METH_VARARGS, gmsh_interpolate_docstring},
    { (char *)"WriteGmsh", (PyCFunction) extras_writegmsh, METH_VARARGS, gmsh_write_docstring},
    { (char *)"ReadGmsh", (PyCFunction) extras_readgmsh, METH_VARARGS, gmsh_read_docstring},
    { NULL, NULL, 0, NULL }
  };

  static char extrasDocString[] = "Module collecting python wrappers to VTK stuff.";

  PyMODINIT_FUNC initpyextras() {
    
    PyObject* m = Py_InitModule3("vtk_extras", extrasMethods, extrasDocString);
  if (m == NULL) { return; };
  
  }

}

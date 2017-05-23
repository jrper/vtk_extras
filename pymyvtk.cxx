#include "vtkPythonArgs.h"
#include "vtkUnstructuredGrid.h"
#include "stdio.h"

#include "GmshWriter.h"
#include "GmshReader.h"
#include "ConsistentInterpolation.h"

vtkUnstructuredGrid* australia(vtkUnstructuredGrid* input) {

  vtkUnstructuredGrid* output = vtkUnstructuredGrid::New();
  output->DeepCopy(input);
  for (int i=0; i<input->GetNumberOfPoints(); ++i){
    double x[3];
    output->GetPoint(i, x);
    output->GetPoints()->SetPoint(i, x[0], -x[1], x[2]);
  }
  return output;
}

extern "C" {

 static PyObject *myvtk_interpolate(PyObject *self, PyObject *args) {

    vtkPythonArgs argument_parser(args, "myvtk_interpolate");
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

 static PyObject *myvtk_readgmsh(PyObject *self, PyObject *args) {

    vtkPythonArgs argument_parser(args, "myvtk_writegmsh");
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

  static PyObject *myvtk_writegmsh(PyObject *self, PyObject *args) {

    vtkPythonArgs argument_parser(args, "myvtk_writegmsh");
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

  static PyObject *myvtk_australia(PyObject *self, PyObject *args) {

    vtkPythonArgs argument_parser(args, "myvtk_australia");
    vtkUnstructuredGrid* ugrid;

    if (!argument_parser.GetVTKObject(ugrid, "vtkUnstructuredGrid")) {
      PyErr_SetString(PyExc_TypeError, "Need VTK unstructured grid.");
      return NULL;
    }

    // apply our function
    ugrid = australia(ugrid);

    // The object below is what we'll return (this seems to add a reference)
    PyObject* pyugrid = vtkPythonUtil::GetObjectFromPointer(ugrid);

    // Clean up our spare reference now (or you could use smart pointers)
    ugrid->Delete();

    // Now back to Python
    return pyugrid;
}

  char australia_docstring[] = "Take a vtk UnstructuredGrid object and reflect it in the x-z plane.";

  static PyMethodDef exampleMethods[] = {
    { (char *)"australia", (PyCFunction) myvtk_australia, METH_VARARGS, australia_docstring},
    { (char *)"Interpolate", (PyCFunction) myvtk_interpolate, METH_VARARGS, gmsh_interpolate_docstring},
    { (char *)"WriteGmsh", (PyCFunction) myvtk_writegmsh, METH_VARARGS, gmsh_write_docstring},
    { (char *)"ReadGmsh", (PyCFunction) myvtk_readgmsh, METH_VARARGS, gmsh_read_docstring},
    { NULL, NULL, 0, NULL }
  };

  static char myvtkDocString[] = "Module collecting python wrappers to VTK stuff.";

  PyMODINIT_FUNC initpymyvtk() {
    
    PyObject* m = Py_InitModule3("pymyvtk", exampleMethods, myvtkDocString);
  if (m == NULL) { return; };
  
  }

}

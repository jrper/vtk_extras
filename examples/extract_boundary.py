import vtk_extras
import vtk

reader=vtk.vtkXMLUnstructuredGridReader()
reader.SetFileName("DG_data.vtu")
reader.Update()
u = vtk_extras.BoundingSurface(reader.GetOutput())

writer = vtk.vtkXMLUnstructuredGridWriter()
writer.SetFileName("DG_data_boundary.vtu")
writer.SetInputData(u)
writer.Write()

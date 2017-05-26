import vtk_extras
import vtk

reader=vtk.vtkXMLUnstructuredGridReader()
reader.SetFileName("DG_data.vtu")
reader.Update()
u = vtk_extras.MergePoints(reader.GetOutput(),degree=1,map_method='project')

writer = vtk.vtkXMLUnstructuredGridWriter()
writer.SetFileName("DG_data_projected.vtu")
writer.SetInputData(u)
writer.Write()

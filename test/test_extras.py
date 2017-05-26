import pytest
import vtk

def tmp_ugrid():
    
    ugrid = vtk.vtkUnstructuredGrid()
    pts = vtk.vtkPoints()
    pts.InsertNextPoint(0,0,0)
    pts.InsertNextPoint(1,0,0)
    pts.InsertNextPoint(0,1,0)
    ugrid.SetPoints(pts)
    ugrid.InsertNextCell(vtk.VTK_TRIANGLE,3,[0,1,2])

    data = vtk.vtkDoubleArray()
    data.InsertNextValue(0.0)
    data.InsertNextValue(1.0)
    data.InsertNextValue(0.0)
    data.SetName("Data")

    ugrid.GetPointData().AddArray(data)

    return ugrid

def tmp_ugrid2():
    
    ugrid = vtk.vtkUnstructuredGrid()
    pts = vtk.vtkPoints()
    pts.InsertNextPoint(0,0,0)
    pts.InsertNextPoint(0.5,0,0)
    pts.InsertNextPoint(0.5,0.5,0)
    pts.InsertNextPoint(1.0,0,0)
    pts.InsertNextPoint(0.0,0.5,0)
    pts.InsertNextPoint(0.0,1.0,0)
    ugrid.SetPoints(pts)
    ugrid.InsertNextCell(vtk.VTK_TRIANGLE,3,[0,1,2])
    ugrid.InsertNextCell(vtk.VTK_TRIANGLE,3,[1,2,4])
    ugrid.InsertNextCell(vtk.VTK_TRIANGLE,3,[1,3,4])
    ugrid.InsertNextCell(vtk.VTK_TRIANGLE,3,[2,4,5])
    return ugrid

def tmp_ugrid3():
    ugrid = vtk.vtkUnstructuredGrid()
    pts = vtk.vtkPoints()
    pts.InsertNextPoint(0,0,0)    
    pts.InsertNextPoint(1,0,0) 
    pts.InsertNextPoint(0,1,0)
    pts.InsertNextPoint(1,1,0)
    pts.InsertNextPoint(1,0,0)
    pts.InsertNextPoint(0,1,0)    
    ugrid.SetPoints(pts)
    ugrid.InsertNextCell(vtk.VTK_TRIANGLE,3,[0,1,2])
    ugrid.InsertNextCell(vtk.VTK_TRIANGLE,3,[3,4,5])

    data = vtk.vtkDoubleArray()
    data.InsertNextValue(0.0)
    data.InsertNextValue(1.0)
    data.InsertNextValue(2.0)
    data.InsertNextValue(3.0)
    data.InsertNextValue(4.0)
    data.InsertNextValue(5.0)
    data.SetName("Data")

    ugrid.GetPointData().AddArray(data)

    return ugrid

def test_import():
    import vtk_extras


def test_read():
    from vtk_extras import ReadGmsh

    ugrid = ReadGmsh("test/box.msh")
    assert(ugrid.GetNumberOfPoints()>0)

def test_no_read():
    from vtk_extras import ReadGmsh
    
    try:
        ugrid = ReadGmsh("nonexistant.msh")
        assert(False)
    except IOError:
        pass

def test_write(tmpdir):
    from vtk_extras import WriteGmsh

    ugrid = tmp_ugrid()

    fpth = tmpdir.join("box.msh")

    assert(not fpth.check())

    WriteGmsh(ugrid, str(fpth))

    assert(fpth.check())

def test_interpolate():
    from vtk_extras import Interpolate
    
    ugrid = tmp_ugrid()
    ugrid2 = tmp_ugrid2()

    out = Interpolate(ugrid2, ugrid)

    assert(out.GetNumberOfPoints()==6)
    assert(out.GetPointData().GetArray("Data").GetValue(2)==0.5)

def test_interpolator():
    from vtk_extras import Interpolator
    
    ugrid = tmp_ugrid()
    ugrid2 = tmp_ugrid2()

    intpr = Interpolator()
    intpr.DataSource = ugrid

    out = intpr(ugrid2)

    assert(out.GetNumberOfPoints()==6)
    assert(out.GetPointData().GetArray("Data").GetValue(2)==0.5)

def test_mergepoints():
    from vtk_extras import MergePoints

    ugrid = tmp_ugrid3()

    out = MergePoints(ugrid,map_method='project')
    assert(out.GetNumberOfPoints()==4)
    assert(out.GetNumberOfCells()==2)
    assert(out.GetPointData().GetNumberOfArrays()==1)
    assert(out.GetPointData().GetNumberOfArrays()==1)
    assert(out.GetPointData().GetArray(0).GetRange()==(0.75,3.75))

    out = MergePoints(ugrid, degree=2)
    assert(out.GetNumberOfPoints()==9)
    assert(out.GetNumberOfCells()==2)
    
def test_boundingsurface():
    from vtk_extras import BoundingSurface

    ugrid = tmp_ugrid2()

    out = BoundingSurface(ugrid)

    assert(out.GetNumberOfCells() == 6)
    assert(out.GetNumberOfPoints() == 6)
    for i in range(6):
        assert(out.GetCell(i).GetCellType()==vtk.VTK_LINE)

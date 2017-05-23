import pytest
import vtk

def test_import():
    import vtk_extras


def test_read():
    from vtk_extras import ReadGmsh
    import os
    
    os.chdir("test")
    ugrid = ReadGmsh("box.msh")
    assert(ugrid.GetNumberOfPoints()>0)

def test_no_read():
    from vtk_extras import ReadGmsh
    
    try:
        ugrid = ReadGmsh("nonexistant.msh")
        assert(False)
    except IOError:
        pass

import pytest

def test_import():
    import vtk_extras


def test_read():
    from vtk_extras import ReadGmsh
    import os
    
    os.chdir("test")
    ugrid = ReadGmsh("box.msh")
    assert(ugrid.GetNumberOfPoints()>0)

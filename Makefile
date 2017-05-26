SWIG = swig
UNAME := $(shell uname -s)
ifeq (${UNAME},Darwin)
	SHARED_FLAG = "-bundle"
	CXXFLAGS = -fPIC -std=c++11
else
	SHARED_FLAG = "-shared"
	CXXFLAGS = -fPIC -std=c++11
endif
PYTEST=py.test-2.7
VTK_FLAGS := $(shell cmake --find-package -DNAME=VTK -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=COMPILE)
VTK_LIBS := $(shell cmake --find-package -DNAME=VTK -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=LINK)

PYTHON_CFLAGS := $(shell python-config --includes)

## For just the python libs use
## PYTHON_LIBS := $(shell python-config --ldflags)

VTK_MAJOR_VERSION := $(shell python -c "import vtk; print vtk.vtkVersion().GetVTKMajorVersion()")

ifeq (${VTK_MAJOR_VERSION},5)
	VTK_PYTHON_LIBS=-lvtkPythonCore
endif

PROJECT = vtk_extras

OBJS = src/GmshWriter.o src/GmshReader.o src/Interpolator.o src/PyInterpolator.o src/ConsistentInterpolator.o src/MergePoints.o src/BoundingSurface.o

default: ${PROJECT}.so

${PROJECT}.so: src/${PROJECT}.o ${OBJS}
	${CXX} ${SHARED_FLAG} ${OBJS} src/${PROJECT}.o ${VTK_LIBS} ${VTK_PYTHON_LIBS} -o ${PROJECT}.so

src/%.cxx :
	cd src
	touch $@

src/%.o : src/%.cxx
	${CXX} ${CXXFLAGS} ${VTK_FLAGS} ${PYTHON_CFLAGS} -c src/$*.cxx -o src/$*.o

clean:
	rm -rf src/*.o *.so *.py *.pyc *~ CMakeFiles

test: ${PROJECT}.so
	PYTHONPATH=${PWD} ${PYTEST} 

.PHONY: clean test


SWIG = swig
UNAME := $(shell uname -s)
ifeq (${UNAME},Darwin)
	SHARED_FLAG = "-bundle"
else
	SHARED_FLAG = "-shared"
endif
PYTEST=py.test-2.7
VTK_FLAGS := $(shell cmake --find-package -DNAME=VTK -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=COMPILE)
VTK_LIBS := $(shell cmake --find-package -DNAME=VTK -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=LINK)

PYTHON_CFLAGS := $(shell python-config --cflags)

## For just the python libs use
## PYTHON_LIBS := $(shell python-config --ldflags)

PROJECT = vtk_extras

OBJS = src/GmshWriter.o src/GmshReader.o src/ConsistentInterpolation.o

default: ${PROJECT}.so

${PROJECT}.so: src/${PROJECT}.o ${OBJS}
	${CXX} ${SHARED_FLAG} ${VTK_LIBS} ${OBJS} src/${PROJECT}.o -o ${PROJECT}.so

src/%.cxx : src/%.h
	cd src
	touch $@

src/%.o : src/%.cxx
	${CXX} ${VTK_FLAGS} -c src/$*.cxx -o src/$*.o

clean:
	rm -rf src/*.o *.so *.py *.pyc *~ CMakeFiles

test: ${PROJECT}.so
	PYTHONPATH=${PWD} ${PYTEST} 

.PHONY: clean test


#ifndef _INTERPOLATOR_H
#define _INTERPOLATOR_H 1

#include "vtkUnstructuredGrid.h"
#include "vtkCellLocator.h"
#include "vtkPointLocator.h"


class Interpolator {
 public:
  virtual void Delete();
  
  virtual int Interpolate(vtkUnstructuredGrid*, vtkUnstructuredGrid*);
  void SetDataSource(vtkUnstructuredGrid*);
  vtkUnstructuredGrid*  GetDataSource();
  virtual int InterpolatePoint(double[3], double*);

 protected:
  vtkCellLocator* locator;
  vtkPointLocator* plocator;
  vtkUnstructuredGrid* source;

  Interpolator();
  ~Interpolator();
};

#endif /* _INTERPOLATOR_H */

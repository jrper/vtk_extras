/* @(#)Distance.h
 */

#ifndef _DISTANCE_H
#define _DISTANCE_H 1

#include "vtkUnstructuredGrid.h"
#include "vtkTriangle.h"
#include "vtkDoubleArray.h"

#endif /* _DISTANCE_H */

class Distance {
 public:
  static Distance* New();
  void Delete();

  void SetBoundary(vtkUnstructuredGrid*);
  vtkUnstructuredGrid* GetBoundary();
  
  int CalculateDistance(vtkUnstructuredGrid*, vtkDoubleArray* distance);

 protected:
  
  vtkUnstructuredGrid* boundary;
  
  int DistanceToTriangle(vtkTriangle*, double[3], double&);

  Distance();
  ~Distance();

};

#include "vtkUnstructuredGrid.h"

class ConsistentInterpolation
{
  
 public:
  static ConsistentInterpolation *New();
  void Delete();

  int Interpolate(vtkUnstructuredGrid*, vtkUnstructuredGrid*, vtkUnstructuredGrid*);

  void SetRadius(double);
  double GetRadius();
  
 protected:

  double Radius;

  ConsistentInterpolation();
  ~ConsistentInterpolation();

};

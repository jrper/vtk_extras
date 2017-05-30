#include "Interpolator.h"

#include "vtkUnstructuredGrid.h"
#include "vtkCellLocator.h"
#include "vtkPointLocator.h"


class ConsistentInterpolator : public Interpolator 
{
  
 public:
  static ConsistentInterpolator *New();
  void Delete();

  int Interpolate(vtkUnstructuredGrid*, vtkUnstructuredGrid*);
  int InterpolatePoint(double x[3], double* val);

  void SetRadius(double);
  double GetRadius();

 protected:

  double Radius;

  ConsistentInterpolator();
  ~ConsistentInterpolator();

};

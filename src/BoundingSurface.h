#include "vtkUnstructuredGrid.h"

class BoundingSurface {
  
 public:
  static BoundingSurface *New();
  void Delete();

  int GetSurface(vtkUnstructuredGrid*,vtkUnstructuredGrid*);
  
 protected:

  BoundingSurface();
  ~BoundingSurface();
};

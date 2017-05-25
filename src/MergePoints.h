#include "vtkUnstructuredGrid.h"
#include "vtkMergePoints.h"
#include "vtkCell.h"
#include "vtkPoints.h"
#include "vtkDataArray.h"
#include "vtkIdList.h"
#include "vtkVersion.h"

enum degrees{DEGREE_UNCHANGED=-1,LINEAR=1,QUADRATIC=2};
enum continuity{DISCONTINUOUS=-1,CONTINUITY_UNCHANGED=0,CONTINUOUS=2};
enum mapper{REMAP,PROJECT};

class MergePoints {
public:
  static MergePoints* New();
  void Delete();

  int Merge(vtkUnstructuredGrid*, vtkUnstructuredGrid*);
  
  int GetDegree();
  void SetDegree(int);
  int GetContinuity();
  void SetContinuity(int);
  void SetMapper(int);


 protected:

  int Degree, Continuity;
  int (*mapper)(vtkUnstructuredGrid*, vtkUnstructuredGrid*, int);

  MergePoints();
  ~MergePoints();

  void AddQuadraticPoints(vtkMergePoints* mergePoints, vtkPoints* input_points, vtkCell* cell,vtkIdList* cellIds, int input_continuity); 

};

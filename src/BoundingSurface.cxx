#include "BoundingSurface.h"
#include "MergePoints.h"

#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkSortDataArray.h"
#include "vtkSmartPointer.h"
#include "vtkDataObject.h"
#include "vtkCell.h"
#include "vtkLine.h"

#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <memory>

struct id_hash
{
  std::size_t operator()(vtkSmartPointer<vtkIdList> const& s) const 
    {
      std::size_t h1 = std::hash<int>{}(s->GetId(0));
      std::size_t h2 = std::hash<int>{}(s->GetId(1));
      return h1 ^ (h2 << 1);
    }
};


struct id_eq {
  bool operator()(vtkSmartPointer<vtkIdList> const &v, vtkSmartPointer<vtkIdList> const& u) const{
    for (int i=0;i<v->GetNumberOfIds();++i){
      if (v->GetId(i)!=u->GetId(i)) return false;
    }
    return true;
  }
};

vtkIdList* sort(vtkCell* e) {
  vtkIdList* list = vtkIdList::New(); 
  list->Allocate(e->GetNumberOfPoints());

  if (e->IsLinear()) {
    list->SetNumberOfIds(e->GetNumberOfPoints());
    for (int i=0;i<e->GetNumberOfPoints();++i){
      list->SetId(i,e->GetPointIds()->GetId(i));
    }
    vtkSortDataArray::Sort(list);
    return list;
  } else {
    if (e->GetCellType() == VTK_QUADRATIC_EDGE) {
      list->SetNumberOfIds(2);
      for (int i=0;i<2;++i){
	list->SetId(i,e->GetPointIds()->GetId(i));
      }
      vtkSortDataArray::Sort(list);
      list->SetNumberOfIds(3);
      list->SetId(2,e->GetPointIds()->GetId(2));
      return list;
    } else if (e->GetCellType() == VTK_QUADRATIC_TRIANGLE) {
      list->SetNumberOfIds(3);
      for (int i=0;i<3;++i){
	list->SetId(i,e->GetPointIds()->GetId(i));
      }
      vtkSortDataArray::Sort(list);
      list->SetNumberOfIds(6);
      for (int i=0;i<3;++i){
	for (int j=0;i<3;++i){
	  if (list->GetId(i) == e->GetPointIds()->GetId(j)) {
	    list->SetId(i+3,e->GetPointIds()->GetId(j+3));
	    break;
	  }
	}
      }
      return list;
    }
  }    
}

BoundingSurface* BoundingSurface::New(){
  return new BoundingSurface;
}

void BoundingSurface::Delete(){delete this;};

BoundingSurface::BoundingSurface(){};
BoundingSurface::~BoundingSurface(){};

int get_bound_count(vtkCell* cell) {
  int count;
  switch(cell->GetCellDimension()) {
  case 1:
    {
      count = cell->GetNumberOfPoints();
      break;
    }
  case 2:
    {
      count = cell->GetNumberOfEdges();
      break;
    }
  case 3:
    {
      count = cell->GetNumberOfFaces();
      break;
    }
  }
  return count;
}

vtkCell* get_bound(vtkCell* cell, int i) {
  vtkCell* bnd;
  switch(cell->GetCellDimension()) {
  case 2:
    {
      bnd = cell->GetEdge(i);
      break;
    }
  case 3:
    {
      bnd = cell->GetFace(i);
      break;
    }
  }
  return bnd;
}

int BoundingSurface::GetSurface(vtkUnstructuredGrid* input, vtkUnstructuredGrid* output)
{

  vtkSmartPointer<vtkUnstructuredGrid> continuous_input = vtkSmartPointer<vtkUnstructuredGrid>::New();
  MergePoints* merger = MergePoints::New();
  merger->SetDegree(DEGREE_UNCHANGED);
  merger->SetContinuity(CONTINUOUS);
  merger->Merge(input,continuous_input);
  merger->Delete();

  vtkSmartPointer<vtkPoints> outpoints= vtkSmartPointer<vtkPoints>::New();  
  vtkSmartPointer<vtkMergePoints> mergePoints= vtkSmartPointer<vtkMergePoints>::New();
  mergePoints->SetTolerance(1.e-34);
  mergePoints->InitPointInsertion(outpoints, input->GetBounds());

  vtkIdType ids[20];
  
  int dim=0;

  for (vtkIdType i=0; i<continuous_input->GetNumberOfCells(); ++i) {
    vtkCell* cell=continuous_input->GetCell(i);
    dim = std::max(dim,cell->GetCellDimension());
  }

  for (vtkIdType i=0; i<continuous_input->GetNumberOfCells(); ++i) {
    vtkCell* cell=continuous_input->GetCell(i);
    if (cell->GetCellDimension()<dim) continue;

    for(int j=0; j< get_bound_count(cell); ++j) { 
      vtkIdList* list = vtkIdList::New();
      vtkSmartPointer<vtkCell> bnd = get_bound(cell, j);
      continuous_input->GetCellNeighbors(i,bnd->GetPointIds(), list);   
      if (list->GetNumberOfIds()==0) {
	vtkIdType id;
	for(vtkIdType k=0; k<bnd->GetNumberOfPoints(); ++k) {
	  mergePoints->InsertUniquePoint(bnd->GetPoints()->GetPoint(k),id);
	  ids[k] = id;
	} 
	output->InsertNextCell(bnd->GetCellType(), bnd->GetNumberOfPoints(), ids);
      }
      list->Delete();
    }
  }

  output->GetPointData()->CopyStructure(continuous_input->GetPointData());
  output->GetCellData()->CopyStructure(continuous_input->GetCellData());

  for (int j=0; j<output->GetPointData()->GetNumberOfArrays(); ++j) {

    output->GetPointData()->GetArray(j)->SetNumberOfTuples(output->GetNumberOfCells());

    for( vtkIdType i=0; i< continuous_input->GetNumberOfPoints(); ++i) {
      vtkIdType id = mergePoints->IsInsertedPoint(continuous_input->GetPoint(i));
      if (id >= 0) {
	output->GetPointData()->GetArray(j)->SetTuple(id, i, continuous_input->GetPointData()->GetArray(j));
      }
    }
  }


  for (int j=0; j<output->GetCellData()->GetNumberOfArrays(); ++j) {
    
    output->GetCellData()->GetArray(j)->SetNumberOfTuples(output->GetNumberOfCells());
    vtkIdType nn=0;
    for (vtkIdType i=0; i<continuous_input->GetNumberOfCells(); ++i) {
      vtkCell* cell=continuous_input->GetCell(i);
      if (cell->GetCellDimension()<dim) continue;
      for(int m=0; m< get_bound_count(cell); ++m) { 
	vtkIdList* list = vtkIdList::New();
	vtkSmartPointer<vtkCell> bnd = get_bound(cell, m);
	continuous_input->GetCellNeighbors(i, bnd->GetPointIds(), list);   
	if (list->GetNumberOfIds()==0) {
	  output->GetCellData()->GetArray(j)->SetTuple(nn++, i, continuous_input->GetCellData()->GetArray(j));
	}
	list->Delete();
      }
    }
  }

  vtkSmartPointer<vtkPoints> tmppoints= vtkSmartPointer<vtkPoints>::New();
  tmppoints->DeepCopy(mergePoints->GetPoints());
  output->SetPoints(tmppoints);

  return 1;
}

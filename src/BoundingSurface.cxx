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

int BoundingSurface::GetSurface(vtkUnstructuredGrid* input, vtkUnstructuredGrid* output)
{

  vtkSmartPointer<vtkUnstructuredGrid> continuous_input = vtkSmartPointer<vtkUnstructuredGrid>::New();
  MergePoints* merger = MergePoints::New();
  merger->SetDegree(DEGREE_UNCHANGED);
  merger->SetContinuity(CONTINUOUS);
  merger->Merge(input,continuous_input);
  merger->Delete();

  vtkSmartPointer<vtkPoints> outpoints= vtkSmartPointer<vtkPoints>::New();  
  
  int dim=0;

  for (vtkIdType i=0; i<continuous_input->GetNumberOfCells(); ++i) {
    vtkCell* cell=continuous_input->GetCell(i);
    dim = std::max(dim,cell->GetCellDimension());
  }
  switch (dim)
    {
    case 1: 
      {
	std::unordered_set<vtkIdType> boundary;
	for (vtkIdType i=0; i<continuous_input->GetNumberOfCells(); ++i) {
	  vtkCell* cell=continuous_input->GetCell(i);
	  
	  if (cell->GetCellDimension() != dim) continue;
	  
	  for (int j=0; j<2; ++j){
	    vtkIdType pt = cell->GetPointIds()->GetId(j);
	    if (boundary.count(pt)) {
	      boundary.erase(pt);
	    } else {
	      boundary.insert(pt);
	    }
	  }

	}


	std::unordered_map<vtkIdType,vtkIdType> point_map;
	for (std::unordered_set<vtkIdType>::iterator e = boundary.begin(); e != boundary.end(); ++e) {
	  point_map.emplace(*e,outpoints->InsertNextPoint(input->GetPoint(*e)));
	}
	output->SetPoints(outpoints);

	output->Allocate(boundary.size());

	for (std::unordered_set<vtkIdType>::iterator e = boundary.begin(); e != boundary.end(); ++e) {
	  vtkIdType l[1];
	  l[0] = point_map.at(*e);
	  output->InsertNextCell(VTK_VERTEX,1,l);
	}
      }
      break;
    case 2:
      {
	std::unordered_set<vtkSmartPointer<vtkIdList>, id_hash, id_eq> boundary;
	
	for (vtkIdType i=0; i<continuous_input->GetNumberOfCells(); ++i) {
	  vtkCell* cell=continuous_input->GetCell(i);
	  
	  if (cell->GetCellDimension() != dim) continue;
	  
	  
	  for (int j=0; j<cell->GetNumberOfEdges(); ++j){
	    vtkCell* edge = cell->GetEdge(j);
	    
	    vtkIdList* vertices = sort(edge);
	    
	    if (boundary.count(vertices)) {
	      boundary.erase(vertices);
	    } else {
	      boundary.insert(vertices);
	    }
	  }
	}

	std::unordered_map<vtkIdType,vtkIdType> point_map;
	for (std::unordered_set<vtkSmartPointer<vtkIdList>,id_hash , id_eq>::iterator e = boundary.begin(); e != boundary.end(); ++e) {
	  for (int j=0;j<(*e)->GetNumberOfIds();++j) {
	    if (point_map.count((*e)->GetId(j)) == 0) point_map.emplace((*e)->GetId(j),outpoints->InsertNextPoint(input->GetPoint((*e)->GetId(j))));
	  }
	}
	output->SetPoints(outpoints);

	output->Allocate(boundary.size());
	  
	for (std::unordered_set<vtkSmartPointer<vtkIdList>,id_hash , id_eq>::iterator e = boundary.begin(); e != boundary.end(); ++e  ) {
	  vtkIdType l[3];
	  for (int j=0;j<(*e)->GetNumberOfIds();++j) {
	    l[j] = point_map.at((*e)->GetId(j));
	  }
	  switch ((*e)->GetNumberOfIds()) {
	  case 2:
	    output->InsertNextCell(VTK_LINE,2,l);
	    break;
	  case 3:
	    output->InsertNextCell(VTK_QUADRATIC_EDGE,3,l);
	    break;
	  }
	}
      }
      break;
    case 3:
      {
	std::unordered_set<vtkSmartPointer<vtkIdList>, id_hash, id_eq > boundary;

	for (vtkIdType i=0; i<continuous_input->GetNumberOfCells(); ++i) {
	  vtkCell* cell=input->GetCell(i);
	  
	  if (cell->GetCellDimension() != dim) continue;
	  
	  
	  for (int j=0; j<cell->GetNumberOfFaces(); ++j){
	    vtkCell* edge = cell->GetFace(j);
	    
	    vtkSmartPointer<vtkIdList> vertices = sort(edge);
	    
	    if (boundary.count(vertices)) {
	      boundary.erase(vertices);
	    } else {
	      boundary.insert(vertices);
	    }
	  }
	}

	std::unordered_map<vtkIdType,vtkIdType> point_map;
	for (std::unordered_set<vtkSmartPointer<vtkIdList>,id_hash , id_eq>::iterator e = boundary.begin(); e != boundary.end(); ++e) {
	  for (int j=0;j<(*e)->GetNumberOfIds();++j) {
	    if (point_map.count((*e)->GetId(j))==0)  point_map.emplace((*e)->GetId(j),outpoints->InsertNextPoint(input->GetPoint((*e)->GetId(j))));
	  }
	}
	output->SetPoints(outpoints);
	
	output->Allocate(boundary.size());
	  
	for (std::unordered_set<vtkSmartPointer<vtkIdList>, id_hash, id_eq>::iterator e = boundary.begin(); e != boundary.end(); ++e  ) {
	  vtkIdType l[6];
	  for (int j=0;j<(*e)->GetNumberOfIds();++j) {
	    l[j] = point_map.at((*e)->GetId(j));
	  }
	  switch ((*e)->GetNumberOfIds()) {
	  case 3:
	    output->InsertNextCell(VTK_TRIANGLE,3,l);
	    break;
	  case 6:
	    output->InsertNextCell(VTK_QUADRATIC_TRIANGLE,6,l);
	    break;
	  }
	}
      }
      break;
    }

  output->GetPointData()->DeepCopy(continuous_input->GetPointData());

  return 1;
}

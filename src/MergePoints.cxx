#include "MergePoints.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCell.h"
#include "vtkCellType.h"
#include "vtkCellTypes.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkLine.h"
#include "vtkTriangle.h"
#include "vtkTetra.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkMergePoints.h"
#include "vtkDoubleArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkMath.h"
#include "vtkStreamingDemandDrivenPipeline.h"

int LinearType(int cellType) {
  switch (cellType) {
  case VTK_QUADRATIC_HEXAHEDRON:
    return VTK_HEXAHEDRON;
  case VTK_QUADRATIC_PYRAMID:
    return VTK_PYRAMID;
  case VTK_QUADRATIC_QUAD:
    return VTK_QUAD;
  case VTK_QUADRATIC_TETRA:
    return VTK_TETRA;
  case VTK_QUADRATIC_TRIANGLE:
    return VTK_TRIANGLE;
  case VTK_QUADRATIC_WEDGE:
    return VTK_WEDGE;
  }
  return cellType;
}

int QuadraticType(int cellType) {
  switch (cellType) {
  case VTK_HEXAHEDRON:
    return VTK_QUADRATIC_HEXAHEDRON;
  case VTK_PYRAMID:
    return VTK_QUADRATIC_PYRAMID;
  case VTK_QUAD:
    return VTK_QUADRATIC_QUAD;
  case VTK_TETRA:
    return VTK_QUADRATIC_TETRA;
  case VTK_TRIANGLE:
    return VTK_QUADRATIC_TRIANGLE;
  case VTK_WEDGE:
    return VTK_QUADRATIC_WEDGE;
  }
  return cellType;
}

void AddPoints(vtkCell* cell, vtkPoints* input_points, int id1, int id2, double* result) {
  double p1[3], p2[3];
  input_points->GetPoint(cell->GetPointId(id1),p1);
  input_points->GetPoint(cell->GetPointId(id2),p2);
  for (int i=0;i<3;i++){
    result[i]=(p1[i]+p2[i])/2.0;
  }
}

void AddData(vtkDataArray* output_data,vtkDataArray* input_data,vtkCell* output_cell,vtkCell* input_cell,int i,int j,int k) {
  double v1[input_data->GetNumberOfTuples()];
  double v2[input_data->GetNumberOfTuples()];
  double vo[input_data->GetNumberOfTuples()];

  input_data->GetTuple(input_cell->GetPointId(i),v1);
  input_data->GetTuple(input_cell->GetPointId(j),v2);

  for (int n=0; n<input_data->GetNumberOfTuples(); n++) {
    vo[n]=(v1[n]+v2[n])/2.0;
  }

  output_data->SetTuple(output_cell->GetPointId(k),vo);
}

void AddQuadraticData(vtkDataArray* output_data,vtkDataArray* input_data,vtkCell* input_cell,vtkCell* output_cell) {

  int N = output_cell->GetNumberOfPoints()-input_cell->GetNumberOfPoints();
  int N_in = input_cell->GetNumberOfPoints();

  AddData(output_data,input_data,output_cell,input_cell,0,1,N_in);
  if (N>1) {
    AddData(output_data,input_data,output_cell,input_cell,1,2,N_in+1);
    AddData(output_data,input_data,output_cell,input_cell,2,0,N_in+2);
  }
  if (N>3) {
    AddData(output_data,input_data,output_cell,input_cell,0,3,N_in+3);
    AddData(output_data,input_data,output_cell,input_cell,1,3,N_in+4);
    AddData(output_data,input_data,output_cell,input_cell,2,3,N_in+5);
  }
}

double GetMeasure(vtkCell* cell) {
  double measure;
    switch (cell->GetCellType()) {
    case VTK_LINE:
      measure=sqrt(((vtkLine*)cell)->GetLength2())/2.0;
      break;
    case VTK_TRIANGLE:
      measure=((vtkTriangle*)cell)->ComputeArea()/3.0;
      break;
    case VTK_TETRA:
      measure=vtkTetra::ComputeVolume(cell->GetPoints()->GetPoint(0),
				      cell->GetPoints()->GetPoint(1),
				      cell->GetPoints()->GetPoint(2),
				      cell->GetPoints()->GetPoint(3))/4.0;
      break;
    }
    return measure;
}

void GetMass(vtkCell* cell, double mass[]) {
  double measure;
    switch (cell->GetCellType()) {
    case VTK_LINE:
      measure=sqrt(((vtkLine*)cell)->GetLength2());
      mass[0] = measure/3.0;
      mass[1] = measure/6.0;
      break;
    case VTK_TRIANGLE:
      measure=((vtkTriangle*)cell)->ComputeArea();
      mass[0] = measure/6.0;
      mass[1] = measure/12.0;
      break;
    case VTK_TETRA:
      measure=vtkTetra::ComputeVolume(cell->GetPoints()->GetPoint(0),
				      cell->GetPoints()->GetPoint(1),
				      cell->GetPoints()->GetPoint(2),
				      cell->GetPoints()->GetPoint(3));
      mass[0] = measure/10.0;
      mass[1] = measure/20.0;
      break;
    }
    return;
}

void zero(vtkDoubleArray* data){
  for (vtkIdType i=0; i<data->GetNumberOfTuples()
	 *data->GetNumberOfComponents();++i){
    data->SetValue(i,0.0);
  }
}

int Project(vtkUnstructuredGrid* input, vtkUnstructuredGrid* output, int cnt) {

  vtkSmartPointer<vtkDoubleArray> mass= vtkSmartPointer<vtkDoubleArray>::New();
  mass->SetNumberOfComponents(1);
  mass->SetNumberOfTuples(output->GetNumberOfPoints());
  zero(mass);

  unsigned char* cellGhostLevels;
  vtkDataArray* temp = input->GetCellData()->GetArray("vtkGhostLevels");
  if (temp) cellGhostLevels =(static_cast<vtkUnsignedCharArray*>(temp))->GetPointer(0);
    
  for(vtkIdType i=0; i<output->GetNumberOfCells(); ++i) {
    vtkCell* cell = output->GetCell(i);
    double measure=GetMeasure(cell);

    for (vtkIdType j=0;j<cell->GetNumberOfPoints();++j){
      vtkIdType id = cell->GetPointId(j);
      mass->SetValue(id,mass->GetValue(id)+measure);
    } 
  } 

  for (int i=0; i<input->GetPointData()->GetNumberOfArrays();i++) {
    vtkSmartPointer<vtkDoubleArray> data= vtkSmartPointer<vtkDoubleArray>::New();
    data->SetName(input->GetPointData()->GetArray(i)->GetName());
    data->SetNumberOfComponents(input->GetPointData()->GetArray(i)->GetNumberOfComponents());
    data->SetNumberOfTuples(output->GetNumberOfPoints());
    zero(data);
    
    vtkIdType nn=0;
    for (vtkIdType j=0; j<input->GetNumberOfCells();j++) {

      if (temp) {
	if (cellGhostLevels[j]>0) {
	  continue;
	}
      }


      vtkCell* input_cell=input->GetCell(j);
      vtkCell* output_cell=output->GetCell(nn++);
      double local_mass[2];
      GetMass(output_cell,local_mass);

      for (vtkIdType k=0; k<output_cell->GetNumberOfPoints(); ++k) {
	vtkIdType id = output_cell->GetPointId(k);
	for (int c=0; c<input->GetPointData()->GetArray(i)->GetNumberOfComponents();++c) {
	  data->SetComponent(id,c,data->GetComponent(id,c)
			     +local_mass[0]*input->GetPointData()->GetArray(i)->GetComponent(input_cell->GetPointId(k),c));
	    }
       
	for (vtkIdType n=1; n<output_cell->GetNumberOfPoints(); ++n) {
	  vtkIdType id2 = output_cell->GetPointId((k+n)%output_cell->GetNumberOfPoints());
	  for (int c=0; c<input->GetPointData()->GetArray(i)->GetNumberOfComponents();++c) {
	    data->SetComponent(id2,c,data->GetComponent(id2,c)
			       +local_mass[1]*input->GetPointData()->GetArray(i)->GetComponent(input_cell->GetPointId(k),c));
	  }
	}
      }
    }
    for (vtkIdType k=0; k<output->GetNumberOfPoints(); ++k) {
      data->SetValue(k,data->GetValue(k)/mass->GetValue(k));
    }
    output->GetPointData()->AddArray(data);
  }
  return 0;
}

int Remap(vtkUnstructuredGrid* input, vtkUnstructuredGrid* output, int cnt) {

  unsigned char* cellGhostLevels;
  vtkDataArray* temp = input->GetCellData()->GetArray("vtkGhostLevels");
  if (temp) cellGhostLevels =(static_cast<vtkUnsignedCharArray*>(temp))->GetPointer(0);

  for (int i=0; i<input->GetPointData()->GetNumberOfArrays();i++) {
    vtkSmartPointer<vtkDoubleArray> data= vtkSmartPointer<vtkDoubleArray>::New();
    data->SetName(input->GetPointData()->GetArray(i)->GetName());
    data->SetNumberOfComponents(input->GetPointData()->GetArray(i)->GetNumberOfComponents());
    data->SetNumberOfTuples(output->GetNumberOfPoints());
    
    if (cnt==CONTINUITY_UNCHANGED &&
	(input->GetCell(0) && output->GetCell(0)) &&
	input->GetCell(0)->GetCellType() == output->GetCell(0)->GetCellType()) {
      data->DeepCopy(input->GetPointData()->GetArray(i));
    } else {
      vtkIdType n=0;
      for (vtkIdType j=0; j<input->GetNumberOfCells();j++) {

	if (temp) {
	  if (cellGhostLevels[j]>0) {
	    continue;
	  }
	}
	vtkCell* input_cell=input->GetCell(j);
	vtkCell* output_cell=output->GetCell(n++);
	if (LinearType(input_cell->GetCellType())==output_cell->GetCellType()
	    || input_cell->GetCellType()==output_cell->GetCellType()) {
	  for (int k=0;k<output_cell->GetNumberOfPoints();k++) {
	    vtkIdType input_id=input_cell->GetPointId(k);
	    vtkIdType output_id=output_cell->GetPointId(k);
	    data->SetTuple(output_id, input->GetPointData()->GetArray(i)->GetTuple(input_id));
	  }
	} else { 
	  // Need to remap the data
	  for (int k=0;k<input_cell->GetNumberOfPoints();k++) {
	    vtkIdType input_id=input_cell->GetPointId(k);
	    vtkIdType output_id=output_cell->GetPointId(k);
	    data->SetTuple(output_id, input->GetPointData()->GetArray(i)->GetTuple(input_id));
	  }
	  AddQuadraticData(data,input->GetPointData()->GetArray(i),input_cell,output_cell);
	} 
      }
    }
    output->GetPointData()->AddArray(data);
    
  }
  return 0;
}

MergePoints* MergePoints::New(){
  return new MergePoints;
}

void MergePoints::Delete(){
  delete this;
}

int MergePoints::GetDegree(){
  return this->Degree;
}

void MergePoints::SetDegree(int d){
  this->Degree = d;
}

int MergePoints::GetContinuity(){
  return this->Continuity;
}

void MergePoints::SetContinuity(int c){
  this->Continuity = c;
}

void MergePoints::SetMapper(int mapper){
  switch (mapper) {
  case REMAP:
    this->mapper = &Remap;
    break;
  case PROJECT:
    this->mapper = &Project;
    break;
  }
}

MergePoints::MergePoints(){
  this->Degree=DEGREE_UNCHANGED; // -1 will mean don't change, 1 Linear, 2 Quadratic.
  this->Continuity=CONTINUOUS; // 0 will mean don't change, 1 continuous, -1 discontinuous.
  this->mapper = &Remap;
};
MergePoints::~MergePoints(){};

 void MergePoints::AddQuadraticPoints(vtkMergePoints* mergePoints, vtkPoints* input_points, vtkCell* cell,vtkIdList* cellIds, int input_continuity) {
  if (cell->GetCellType() == QuadraticType(cell->GetCellType())) {
    return;
  }
  int N = cell->GetNumberOfPoints();
  N = N*(N-1)/2;
  double points[3*N];
  AddPoints(cell,input_points,0,1,&(points[0]));
  if (N>1) {
    AddPoints(cell,input_points,1,2,&(points[3]));
    AddPoints(cell,input_points,2,0,&(points[6]));
  }
  if (N>3) {
    AddPoints(cell,input_points,0,3,&(points[9]));
    AddPoints(cell,input_points,1,3,&(points[12]));
    AddPoints(cell,input_points,2,3,&(points[15]));
  }	 
  for (int j=0; j<N; j++) {
    vtkIdType id;
    if (this->Continuity == DISCONTINUOUS || ( this->Continuity==CONTINUITY_UNCHANGED && input_continuity<0)) {
      id=mergePoints->GetPoints()->InsertNextPoint(&(points[3*j]));
    } else { 
      mergePoints->InsertUniquePoint(&(points[3*j]),id);
    }
    cellIds->InsertNextId(id);
  }
}

int MergePoints::Merge(vtkUnstructuredGrid* input, vtkUnstructuredGrid* output) {
  vtkSmartPointer<vtkMergePoints> mergePoints= vtkSmartPointer<vtkMergePoints>::New();

  int input_continuity;
  
  if (!input || input->GetNumberOfCells() == 0) return -1;
  if (input->GetNumberOfPoints() == input->GetNumberOfCells() * input->GetCell(0)->GetNumberOfPoints()) {
    input_continuity=-1;
  } else {
    input_continuity=1;
  }

  vtkSmartPointer<vtkPoints> outpoints= vtkSmartPointer<vtkPoints>::New();

  mergePoints->SetTolerance(1.e-34);
  mergePoints->InitPointInsertion(outpoints, input->GetBounds());

  output->Allocate(input->GetNumberOfCells());

  unsigned char* cellGhostLevels;
  vtkDataArray* temp = input->GetCellData()->GetArray("vtkGhostLevels");
  if (temp) cellGhostLevels =(static_cast<vtkUnsignedCharArray*>(temp))->GetPointer(0);

  output->GetCellData()->CopyStructure(input->GetCellData());

  for (int i=0; i<input->GetNumberOfCells(); ++i) {
    vtkCell* cell = input->GetCell(i);
    vtkIdList* cellIds = vtkIdList::New();
    cellIds->SetNumberOfIds(cell->GetNumberOfPoints());
    vtkIdType id_map;

    

    if (temp) {
      if (cellGhostLevels[i]>0) {
	continue;
      }
    }

    for (int j=0; j<cell->GetNumberOfPoints(); ++j) {
      vtkIdType id = cell->GetPointIds()->GetId(j);
      if (this->Continuity==DISCONTINUOUS || (this->Continuity==CONTINUITY_UNCHANGED && input_continuity<0)) {
	id_map=mergePoints->GetPoints()->InsertNextPoint(input->GetPoints()->GetPoint(id));
      } else {
	mergePoints->InsertUniquePoint(input->GetPoints()->GetPoint(id),id_map);
      }
      cellIds->SetId(j,id_map);
    }

    switch (this->GetDegree())
      {
      case DEGREE_UNCHANGED:
	output->InsertNextCell(cell->GetCellType(),cellIds);
	break;
      case LINEAR:
	output->InsertNextCell(LinearType(cell->GetCellType()),cellIds);
	break;
      case QUADRATIC:	
	AddQuadraticPoints(mergePoints,input->GetPoints(),cell,cellIds, input_continuity);
	output->InsertNextCell(QuadraticType(cell->GetCellType()),cellIds);
	break;
      }
  }

  

  
  for (int j=0; j<output->GetCellData()->GetNumberOfArrays(); ++j) {

    output->GetCellData()->GetArray(j)->SetNumberOfTuples(output->GetNumberOfCells());
    vtkIdType k=0;
    for (vtkIdType i=0; i<input->GetNumberOfCells(); ++i) {
      if (temp) {
	if (cellGhostLevels[i]>0) {
	  continue;
	}
      }
      output->GetCellData()->GetArray(j)->SetTuple(k++,i,input->GetCellData()->GetArray(j));	
    }
  }

  if (temp) {
    output->GetCellData()->RemoveArray("vtkGhostLevels");
  }
      

  vtkSmartPointer<vtkPoints> tmppoints= vtkSmartPointer<vtkPoints>::New();
  tmppoints->DeepCopy(mergePoints->GetPoints());
  output->SetPoints(tmppoints);

  this->mapper(input, output, this->Continuity);

  
  return 1;
  }



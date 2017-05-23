#include "ConsistentInterpolation.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkSmartPointer.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkProbeFilter.h"
#include "vtkCellLocator.h"
#include "vtkPointLocator.h"
#include "vtkGenericCell.h"
#include "vtkDoubleArray.h"
#include "vtkMath.h"

ConsistentInterpolation* ConsistentInterpolation::New(){
  return new ConsistentInterpolation;
};

void ConsistentInterpolation::Delete(){
  delete this;
}

ConsistentInterpolation::ConsistentInterpolation(){
  this->Radius=1.0e-1;
};
ConsistentInterpolation::~ConsistentInterpolation(){};

void ConsistentInterpolation::SetRadius(double r){
  this->Radius=r;
};

double ConsistentInterpolation::GetRadius(){
  return this->Radius;
}

int ConsistentInterpolation::Interpolate(vtkUnstructuredGrid* input,
					 vtkUnstructuredGrid* source,
					 vtkUnstructuredGrid* output)
{
  vtkSmartPointer<vtkPoints> outpoints= vtkSmartPointer<vtkPoints>::New(); 

  outpoints->DeepCopy(input->GetPoints());
  output->SetPoints(outpoints);
  output->Allocate(input->GetNumberOfCells());
  output->SetCells(input->GetCellTypesArray(),
		   input->GetCellLocationsArray(),
		   input->GetCells());

  vtkSmartPointer<vtkCellLocator> locator= vtkSmartPointer<vtkCellLocator>::New();
  locator->SetDataSet(source);
  locator->BuildLocator();

  vtkSmartPointer<vtkPointLocator> plocator= vtkSmartPointer<vtkPointLocator>::New();
  plocator->SetDataSet(source);
  plocator->BuildLocator();

  output->GetPointData()->CopyStructure(source->GetPointData());
  for (vtkIdType j=0;j<source->GetPointData()->GetNumberOfArrays();++j){
    output->GetPointData()->GetArray(j)->SetNumberOfTuples(input->GetNumberOfPoints());
  }

  double w[10];
  double p[10];
  double val[50];
  

  vtkSmartPointer<vtkGenericCell> cell= vtkSmartPointer<vtkGenericCell>::New();

  for (vtkIdType i=0;i<input->GetNumberOfPoints();++i){
    
    vtkIdType cell_id = locator->FindCell(input->GetPoint(i), 0.0, cell, p, w);

    if (cell_id<0) {
      double dist2;
      vtkIdType id = plocator->FindClosestPointWithinRadius(this->Radius, input->GetPoint(i), dist2);
      for (vtkIdType j=0;j<source->GetPointData()->GetNumberOfArrays();++j) {
	vtkDataArray* data =source->GetPointData()->GetArray(j);
	int n = output->GetPointData()->GetArray(j)->GetNumberOfComponents();
	switch (data->GetDataType()) 
	case VTK_DOUBLE:
	  {
	    double val[10], val_in[10];
	    output->GetPointData()->GetArray(j)->GetTuple(i,val_in);
            if ( dist2<=this->Radius*this->Radius ) {
	      source->GetPointData()->GetArray(j)->GetTuple(id,val);
	      for (int k=0; k<n; ++k) {
		val_in[k]=val[k];	
	      }
	    } else {
	      for (int k=0; k<n; ++k) {
		val_in[k]=vtkMath::Nan();	
	      }
	    }
	    vtkDoubleArray::SafeDownCast(output->GetPointData()->GetArray(j))->SetTuple(i,val_in);
	    break;
	  }
      }
  } else {
      int N = cell->GetNumberOfPoints();
      for (vtkIdType j=0;j<source->GetPointData()->GetNumberOfArrays();++j) {
	vtkDataArray* data =source->GetPointData()->GetArray(j);
	int n = output->GetPointData()->GetArray(j)->GetNumberOfComponents();
	switch (data->GetDataType()) 
	case VTK_DOUBLE:
	  {
	    double val[10], val_in[10];
	    output->GetPointData()->GetArray(j)->GetTuple(i,val_in);
	    for (int k=0; k<n; ++k) {
              val_in[k]=0;	
	    }
	    for (int a=0; a<N; ++a) {
	      vtkIdType id = cell->GetPointIds()->GetId(a);
	      data->GetTuple(id, val);
	      for (int k=0; k<n; ++k) {
                val_in[k]=val_in[k]+w[a]*val[k];	
	      }
	    }
	    vtkDoubleArray::SafeDownCast(output->GetPointData()->GetArray(j))->SetTuple(i,val_in);
	    break;
	  }
      }
    }	  
  }

  //  vtkSmartPointer<vtkProbeFilter> filter = vtkSmartPointer<vtkProbeFilter>::New();
  //filter->SetInputData(input);
  //filter->SetSourceData(source);
  //filter->SetTolerance(0.0);
  //filter->Update();
  //out->DeepCopy(filter->GetOutput());

  return 1;
}
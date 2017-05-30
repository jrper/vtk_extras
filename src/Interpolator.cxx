#include "Interpolator.h"
#include "vtkExtrasErrors.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkSmartPointer.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkProbeFilter.h"
#include "vtkGenericCell.h"
#include "vtkDoubleArray.h"
#include "vtkMath.h"


void Interpolator::Delete(){
  delete this;
}

Interpolator::Interpolator(){
  this->locator = vtkCellLocator::New();
  this->plocator = vtkPointLocator::New();
  this->source = vtkUnstructuredGrid::New();
};

Interpolator::~Interpolator(){
  this->locator->Delete();
  this->plocator->Delete();
  this->source->Delete();
};

void Interpolator::SetDataSource(vtkUnstructuredGrid* source){
  this->source->ShallowCopy(source);
  this->locator->SetDataSet(source);
  this->locator->BuildLocator();
  this->plocator->SetDataSet(source);
  this->plocator->BuildLocator();
}

vtkUnstructuredGrid* Interpolator::GetDataSource(){
  return this->source;
}

int Interpolator::Interpolate(vtkUnstructuredGrid* input, vtkUnstructuredGrid* output){
  return RETURN_FAIL_NOT_IMPLEMENTED;
}

int Interpolator::InterpolatePoint(double x[3], double* val){
return RETURN_FAIL_NOT_IMPLEMENTED;
}

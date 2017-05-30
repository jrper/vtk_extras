#include "Distance.h"

#include "vtkCell.h"
#include "vtkLine.h"
#include "vtkPlane.h"

#include <math.h>
#include <limits.h>
#include <algorithm>

Distance* Distance::New(){ return new Distance;}
void Distance::Delete(){ delete this;}

Distance::Distance(){this->boundary=NULL;};
Distance::~Distance(){if(this->boundary) this->boundary->Delete();}

void Distance::SetBoundary(vtkUnstructuredGrid* b) {
  if(this->boundary) this->boundary->Delete();
  this->boundary = vtkUnstructuredGrid::New();
  vtkPoints* pts = vtkPoints::New();
  pts->DeepCopy(b->GetPoints());
  this->boundary->SetPoints(pts);
  pts->Delete();
  this->boundary->SetCells(b->GetCellTypesArray(),
			   b->GetCellLocationsArray(),
			   b->GetCells());
}

vtkUnstructuredGrid* Distance::GetBoundary(){
  return this->boundary;
}

int Distance::DistanceToTriangle(vtkTriangle* cell, double x[3], double& dist)
{
  int i, j;
  double pt1[3], pt2[3], pt3[3], n[3], fabsn;
  double rhs[2], c1[2], c2[2];
  double det, dist2;
  double maxComponent;
  double pcoords[3];
  int idx=0, indices[2];
  double dist2Point, dist2Line1, dist2Line2;
  double cp[3];
  double weights[3];
  double closestPoint[3], closestPoint1[3], closestPoint2[3];

  pcoords[2] = 0.0;

  // Get normal for triangle, only the normal direction is needed, i.e. the
  // normal need not be normalized (unit length)
  //
  cell->Points->GetPoint(1, pt1);
  cell->Points->GetPoint(2, pt2);
  cell->Points->GetPoint(0, pt3);

  vtkTriangle::ComputeNormalDirection(pt1, pt2, pt3, n);

  // Project point to plane
  //
  vtkPlane::GeneralizedProjectPoint(x,pt1,n,cp);

  // Construct matrices.  Since we have over determined system, need to find
  // which 2 out of 3 equations to use to develop equations. (Any 2 should
  // work since we've projected point to plane.)
  //
  for (maxComponent=0.0, i=0; i<3; i++)
  {
    // trying to avoid an expensive call to fabs()
    if (n[i] < 0)
    {
      fabsn = -n[i];
    }
    else
    {
      fabsn = n[i];
    }
    if (fabsn > maxComponent)
    {
      maxComponent = fabsn;
      idx = i;
    }
  }
  for (j=0, i=0; i<3; i++)
  {
    if ( i != idx )
    {
      indices[j++] = i;
    }
  }

  for (i=0; i<2; i++)
  {
    rhs[i] = cp[indices[i]] - pt3[indices[i]];
    c1[i] = pt1[indices[i]] - pt3[indices[i]];
    c2[i] = pt2[indices[i]] - pt3[indices[i]];
  }

  if ( (det = vtkMath::Determinant2x2(c1,c2)) == 0.0 )
  {
    pcoords[0] = pcoords[1] = 0.0;
    dist = -1.0;
    return -1;
  }

  pcoords[0] = vtkMath::Determinant2x2(rhs,c2) / det;
  pcoords[1] = vtkMath::Determinant2x2(c1,rhs) / det;

  // Okay, now find closest point to element
  //
  weights[0] = 1 - (pcoords[0] + pcoords[1]);
  weights[1] = pcoords[0];
  weights[2] = pcoords[1];

  if ( weights[0] >= 0.0 && weights[0] <= 1.0 &&
       weights[1] >= 0.0 && weights[1] <= 1.0 &&
       weights[2] >= 0.0 && weights[2] <= 1.0 )
  {
    //projection distance

    dist = sqrt(vtkMath::Distance2BetweenPoints(cp,x));
    return 1;
  }
  else
  {
    double t;
    if ( weights[1] < 0.0 && weights[2] < 0.0 )
      {
        dist2Point = vtkMath::Distance2BetweenPoints(x,pt3);
        dist2Line1 = vtkLine::DistanceToLine(x,pt1,pt3,t,closestPoint1);
        dist2Line2 = vtkLine::DistanceToLine(x,pt3,pt2,t,closestPoint2);
        if (dist2Point < dist2Line1)
        {
          dist2 = dist2Point;
        }
        else
	  {
          dist2 = dist2Line1;
        }
        if (dist2Line2 < dist2)
        {
          dist2 = dist2Line2;
        }
      }
      else if ( weights[2] < 0.0 && weights[0] < 0.0 )
      {
        dist2Point = vtkMath::Distance2BetweenPoints(x,pt1);
        dist2Line1 = vtkLine::DistanceToLine(x,pt1,pt3,t,closestPoint1);
        dist2Line2 = vtkLine::DistanceToLine(x,pt1,pt2,t,closestPoint2);
        if (dist2Point < dist2Line1)
        {
          dist2 = dist2Point;
        }
        else
        {
          dist2 = dist2Line1;
        }
        if (dist2Line2 < dist2)
        {
          dist2 = dist2Line2;
        }
      }
      else if ( weights[1] < 0.0 && weights[0] < 0.0 )
      {
        dist2Point = vtkMath::Distance2BetweenPoints(x,pt2);
        dist2Line1 = vtkLine::DistanceToLine(x,pt2,pt3,t,closestPoint1);
        dist2Line2 = vtkLine::DistanceToLine(x,pt1,pt2,t,closestPoint2);
        if (dist2Point < dist2Line1)
        {
          dist2 = dist2Point;
        }
        else
        {
          dist2 = dist2Line1;
        }
        if (dist2Line2 < dist2)
        {
          dist2 = dist2Line2;
        }
      }
      else if ( weights[0] < 0.0 )
      {
        dist2 = vtkLine::DistanceToLine(x,pt1,pt2,t,closestPoint);
      }
      else if ( weights[1] < 0.0 )
      {
        dist2 = vtkLine::DistanceToLine(x,pt2,pt3,t,closestPoint);
      }
      else if ( weights[2] < 0.0 )
      {
        dist2 = vtkLine::DistanceToLine(x,pt1,pt3,t,closestPoint);
      }
    dist = sqrt(dist2);
    return 0;
  }
}

int Distance::CalculateDistance(vtkUnstructuredGrid* mesh, vtkDoubleArray* distance) {

  if (!this->boundary) return -1;

  distance->SetNumberOfComponents(1);
  distance->SetNumberOfTuples(mesh->GetNumberOfPoints());
  distance->SetName("Distance");

  for (vtkIdType i=0; i<mesh->GetNumberOfPoints(); ++i) {

    double x[3];

    double dist = std::numeric_limits<double>::infinity();
    mesh->GetPoint(i,x);
    for (vtkIdType j=0; j<this->boundary->GetNumberOfCells();++j){
      
      vtkCell* cell = this->boundary->GetCell(j);
      double t, p0[3], p1[3], y[3];

      switch(cell->GetCellType())
	{
	case VTK_VERTEX:
	  cell->GetPoints()->GetPoint(0,y);
	  dist = std::min(dist,sqrt((x[0]-y[0])*(x[0]-y[0])
				    +(x[1]-y[1])*(x[1]-y[1])));
	  break;
	case VTK_LINE:
	  cell->GetPoints()->GetPoint(0,p0);
	  cell->GetPoints()->GetPoint(1,p1);
	  ((vtkLine*) cell)->DistanceToLine(x,p0,p1,t,y);
	  dist = std::min(dist,sqrt((x[0]-y[0])*(x[0]-y[0])
				    +(x[1]-y[1])*(x[1]-y[1])));
	  break;
	case VTK_TRIANGLE:
	  this->DistanceToTriangle((vtkTriangle*) cell, x, dist);
	  break;
	}

    }

    distance->SetValue(i,dist);
  }

  return 1;
}

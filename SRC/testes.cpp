// WIM, SIBGRAPI, SVR

#include <cstdlib>
#include "testes.hpp"

#include <vtkCubeSource.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkCell.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include <vtkVoxel.h>

//-----------------------------------------------------------------------------
// Name: CylTest_CapsFirst
// Orig: Greg James - gjames@NVIDIA.com
// Lisc: Free code - no warranty & no money back.  Use it all you want
// Desc: 
//    This function tests if the 3D point 'testpt' lies within an arbitrarily
// oriented cylinder.  The cylinder is defined by an axis from 'pt1' to 'pt2',
// the axis having a length squared of 'lengthsq' (pre-compute for each cylinder
// to avoid repeated work!), and radius squared of 'radius_sq'.
//    The function tests against the end caps first, which is cheap -> only 
// a single dot product to test against the parallel cylinder caps.  If the
// point is within these, more work is done to find the distance of the point
// from the cylinder axis.
//    Fancy Math (TM) makes the whole test possible with only two dot-products
// a subtract, and two multiplies.  For clarity, the 2nd mult is kept as a
// divide.  It might be faster to change this to a mult by also passing in
// 1/lengthsq and using that instead.
//    Elminiate the first 3 subtracts by specifying the cylinder as a base
// point on one end cap and a vector to the other end cap (pass in {dx,dy,dz}
// instead of 'pt2' ).
//
//    The dot product is constant along a plane perpendicular to a vector.
//    The magnitude of the cross product divided by one vector length is
// constant along a cylinder surface defined by the other vector as axis.
//
// Return:  -1.0 if point is outside the cylinder
// Return:  distance squared from cylinder axis if point is inside.
//
//-----------------------------------------------------------------------------
double CylTest_CapsFirst( const CVector3 & pt1, const CVector3 & pt2, double lengthsq, double radius_sq, const CVector3 & testpt )
{
	double dx, dy, dz;	// vector d  from line segment point 1 to point 2
	double pdx, pdy, pdz;	// vector pd from point 1 to test point
	double dot, dsq;

	dx = pt2.x - pt1.x;	// translate so pt1 is origin.  Make vector from
	dy = pt2.y - pt1.y;     // pt1 to pt2.  Need for this is easily eliminated
	dz = pt2.z - pt1.z;

	pdx = testpt.x - pt1.x;		// vector from pt1 to test point.
	pdy = testpt.y - pt1.y;
	pdz = testpt.z - pt1.z;

	// Dot the d and pd vectors to see if point lies behind the 
	// cylinder cap at pt1.x, pt1.y, pt1.z

	dot = pdx * dx + pdy * dy + pdz * dz;

	// If dot is less than zero the point is behind the pt1 cap.
	// If greater than the cylinder axis line segment length squared
	// then the point is outside the other end cap at pt2.

	if( dot < 0.0f || dot > lengthsq )
	{
		return( -1.0f );
	}
	else 
	{
		// Point lies within the parallel caps, so find
		// distance squared from point to line, using the fact that sin^2 + cos^2 = 1
		// the dot = cos() * |d||pd|, and cross*cross = sin^2 * |d|^2 * |pd|^2
		// Carefull: '*' means mult for scalars and dotproduct for vectors
		// In short, where dist is pt distance to cyl axis: 
		// dist = sin( pd to d ) * |pd|
		// distsq = dsq = (1 - cos^2( pd to d)) * |pd|^2
		// dsq = ( 1 - (pd * d)^2 / (|pd|^2 * |d|^2) ) * |pd|^2
		// dsq = pd * pd - dot * dot / lengthsq
		//  where lengthsq is d*d or |d|^2 that is passed into this function 

		// distance squared to the cylinder axis:

		dsq = (pdx*pdx + pdy*pdy + pdz*pdz) - dot*dot/lengthsq;

		if( dsq > radius_sq )
		{
			return( -1.0f );
		}
		else
		{
			return( dsq );		// return distance squared to axis
		}
	}
}


inline double getRandomColor()
{
	return rand() / static_cast<double>(RAND_MAX);
}


void criaActorByBounds( vtkRenderer *aRenderer, double *volBounds, double r, double g, double b )
{
	vtkCubeSource *cubeSource = vtkCubeSource::New();
	cubeSource->SetXLength( volBounds[1] - volBounds[0] );
	cubeSource->SetYLength( volBounds[3] - volBounds[2] );
	cubeSource->SetZLength( volBounds[5] - volBounds[4] );

	const double SCALE_FACTOR = 1;
	cubeSource->SetXLength( cubeSource->GetXLength() * SCALE_FACTOR );
	cubeSource->SetYLength( cubeSource->GetYLength() * SCALE_FACTOR );
	cubeSource->SetZLength( cubeSource->GetZLength() * SCALE_FACTOR );

	vtkActor *cubeActor = vtkActor::New();
	vtkPolyDataMapper *polyMapper = vtkPolyDataMapper::New();
	polyMapper->SetInputConnection( cubeSource->GetOutputPort(0) );
	cubeActor->SetMapper( polyMapper );
	cubeActor->GetProperty()->SetOpacity(0.7);
	cubeActor->GetProperty()->SetColor( r, g, b );

	cubeActor->SetPosition((volBounds[0] + volBounds[1]) / 2,
		( volBounds[2] + volBounds[3] ) / 2,
		( volBounds[4] + volBounds[5] ) / 2 );

	aRenderer->AddActor( cubeActor );
}

void getVoxelBounds( vtkImageData *imgData, int cellX, int cellY, int cellZ, double *output )
{
	int celPos[] = { cellX, cellY, cellZ };
	vtkCell *cell = imgData->GetCell( imgData->ComputeCellId( celPos ) );
	vtkVoxel *voxel = dynamic_cast<vtkVoxel *>(cell);

	voxel->GetBounds( output );
}


void criaActorFromCellToCell( vtkRenderer *aRenderer, vtkImageData *imgData,
			   int cellX1, int cellY1, int cellZ1,
			   int cellX2, int cellY2, int cellZ2,
			   double r, double g, double b)
{
	double bounds[2][6];
	getVoxelBounds( imgData, cellX1, cellY1, cellZ1, bounds[0] );
	getVoxelBounds( imgData, cellX2, cellY2, cellZ2, bounds[1] );

	double bestBounds[6];
	for ( int i = 0; i < 6; i++ )
	{
		if ( i % 2 == 0 )
			bestBounds[i] = std::min( bounds[0][i], bounds[1][i] );
		else
			bestBounds[i] = std::max( bounds[0][i], bounds[1][i] );
	}
	criaActorByBounds( aRenderer, bestBounds, r, g, b );
}

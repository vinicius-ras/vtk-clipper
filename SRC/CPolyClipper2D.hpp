#ifndef CPOLYCLIPPER2D_HPP__
#define CPOLYCLIPPER2D_HPP__

#include "CClippingAlgorithm.hpp"
#include "CVector3.hpp"
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>

/** 2D Polygonal Clipper algorithm. This algorithm allows the user to draw a polygon on the screen and then select
 * either the internal or the external area of the polygon. The algorithm then clips out every voxel that lies on the
 * selected area of the given polygon. Note: the current implementation only works with convex polygons.
 */
class CPolyClipper2D : public CClippingAlgorithm
{
private:
	// PROPERTIES
	TVector3Array m_polyPoints;   ///< The points that define the polygon to be tested.
	bool m_erasePositives;        ///< A flag indicating which voxels should be removed: the ones from the positive or the negative sides of the poly-points.


	// METHODS
	bool verifyPointInsidePoly( const CVector3 &testPoint );

protected:
	// METHODS
	CPolyClipper2D();
	~CPolyClipper2D();


public:
	// STATIC METHODS
	static CPolyClipper2D *New();

	// METHODS
	void setConfiguration( const TVector3Array &polyPoints, const CVector3 &testPoint );

	// VIRTUAL METHOD IMPLEMENTATIONS: CClippingAlgorithm
	virtual bool hideVoxel( double cellX, double cellY, double cellZ );
};

#endif

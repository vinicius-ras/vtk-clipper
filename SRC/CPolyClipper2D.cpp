#include "CPolyClipper2D.hpp"
#include <cassert>

// STATIC METHODS
/** Instantiates a #CPolyClipper2D object.
 * @return Returns the new instance. */
CPolyClipper2D *CPolyClipper2D::New()
{
	return new CPolyClipper2D();
}


// METHODS
/** Constructor. */
CPolyClipper2D::CPolyClipper2D()
	: m_polyPoints(),
	m_erasePositives(false)
{
}


/** Destructor. */
CPolyClipper2D::~CPolyClipper2D()
{
}


/** Verifies if a point is inside the polygon defined by the member "m_polyPoints".
 * @param testPoint The point that will be tested to see if it lies inside or outside the poly.
 * @return Returns true if the point lies on the same side of every plane defined in "m_polyPoints".
 */
bool CPolyClipper2D::verifyPointInsidePoly( const CVector3 &testPoint )
{
	bool result = false;
	const size_t totalPoints = m_polyPoints.size();
	for ( size_t p = 0; p < totalPoints; p += 2 )
	{
		// Each 3 points on the vector form a plane. Choose the third point for the plane...
		CVector3 *planeThirdPoint;
		if ( p + 2 == totalPoints )
			planeThirdPoint = &m_polyPoints[0];
		else
			planeThirdPoint = &m_polyPoints[p+2];

		// Calculate plane normal
		CVector3 v1 = m_polyPoints[p+1] - m_polyPoints[p];
		CVector3 v2 = *planeThirdPoint - m_polyPoints[p];
		CVector3 planeNormal = CVector3::cross( v1, v2 );
		planeNormal.normalize();


		double sign = CVector3::dot( planeNormal, testPoint - m_polyPoints[p] );
		if ( p == 0 )
			result = (sign > 0);
		else if ( sign > 0 != result )
			return false;   // lies outside the polygon
	}
	return true;
}


/** Configures the algorithm to run.
 * @param polyPoints An array of points that shall be used to define the polygon to test.
 *    This array should be made up of pairs of points. When the user clicks a point on the screen,
 *    this point shall be converted into two points: one corresponding point lying on the near clipping plane,
 *    and the other one on the far clipping plane.
 * @param testPoint A point that lies into the region of the polygon that should be erased.
 */
void CPolyClipper2D::setConfiguration( const TVector3Array &polyPoints, const CVector3 &testPoint )
{
	m_polyPoints = polyPoints;

	// verify which side of the polygons should be tested
	assert( polyPoints.size() % 2 == 0 );

	m_erasePositives = verifyPointInsidePoly( testPoint );
}


// VIRTUAL METHOD IMPLEMENTATIONS: CClippingAlgorithm
bool CPolyClipper2D::hideVoxel( double cellX, double cellY, double cellZ )
{
	// Test the side of the voxel, relative to the plane
	CVector3 voxelPos( cellX, cellY, cellZ );

	return (verifyPointInsidePoly( voxelPos ) == m_erasePositives);
}

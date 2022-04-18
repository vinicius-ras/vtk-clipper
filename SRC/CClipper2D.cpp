#include "CClipper2D.hpp"
#include <cassert>

// STATIC METHODS
/** Instantiates a #CClipper2D object.
 * @return Returns the new instance. */
CClipper2D *CClipper2D::New()
{
	return new CClipper2D();
}

// METHODS
/** Constructor. */
CClipper2D::CClipper2D()
	: m_planeNormal(),
	m_pointOnPlane(),
	m_erasePositives(false),
	m_isInitialized(false)
{
}


/** Destructor. */
CClipper2D::~CClipper2D()
{
}


/** Configures/initializes the Clipper algorithm before it can be executed.
 * @param planeNormal The vector that represents the normal of the clipping plane.
 * @param pointOnPlane A point that lies on the clipping plane.
 * @param pointOnSideToErase A point that does not lie on the clipping plane, but lies on a side of the clipping plane.
 *    The voxels that lie on the same side as this point will be hidden by the Clipper 2D algorithm.
 */
void CClipper2D::setConfiguration( const CVector3 &planeNormal, const CVector3 &pointOnPlane,
								  const CVector3 &pointOnSideToErase )
{
	// Initialize plane data
	m_planeNormal = planeNormal;
	m_planeNormal.normalize();

	m_pointOnPlane = pointOnPlane;

	// Verify which side of the plane should be selected by the Clipper algorithm: the positive or the negative side?
	double sign = CVector3::dot( m_planeNormal, pointOnSideToErase - m_pointOnPlane );
	m_erasePositives = ( sign > 0 );

	// Mark the object as initialized
	m_isInitialized = true;
}


// OVERRIDEN METHODS: CClippingAlgorithm
bool CClipper2D::hideVoxel( double cellX, double cellY, double cellZ )
{
#ifdef DEBUG
	assert( m_isInitialized );
#endif

	// Test the side of the voxel, relative to the plane
	CVector3 voxelPos( cellX, cellY, cellZ );

	double sign = CVector3::dot( m_planeNormal, voxelPos - m_pointOnPlane );
	return (m_erasePositives == (sign > 0));
}

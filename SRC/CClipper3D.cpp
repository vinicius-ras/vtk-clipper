#include "CClipper3D.hpp"
#include <cassert>

// STATIC METHODS
/** Instantiates a #CClipper3D object.
 * @return Returns the new instance. */
CClipper3D *CClipper3D::New()
{
	return new CClipper3D();
}


// METHODS
/** Constructor. */
CClipper3D::CClipper3D()
	: m_planeNormal(),
	m_pointOnPlane(),
	m_isInitialized( false )
{
}


/** Destructor. */
CClipper3D::~CClipper3D()
{
}


/** Configures the clipping algorithm.
 * @param planeNormal A vector indicating the plane's normal.
 * @param pointOnPlane An arbitrary point which lies in the plane. */
void CClipper3D::setConfiguration( const CVector3 &planeNormal, const CVector3 &pointOnPlane )
{
	m_planeNormal = planeNormal;
	m_pointOnPlane = pointOnPlane;

	m_isInitialized = true;
}


// OVERRIDEN METHODS: CClippingAlgorithm
bool CClipper3D::hideVoxel( double cellX, double cellY, double cellZ )
{
	assert( m_isInitialized );

	// Test the side of the voxel, relative to the plane
	CVector3 voxelPos( cellX, cellY, cellZ );

	double sign = CVector3::dot( m_planeNormal, voxelPos - m_pointOnPlane );
	return (sign > 0);
}

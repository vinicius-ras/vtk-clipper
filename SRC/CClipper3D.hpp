#ifndef CCLIPLER3D_HPP__
#define CCLIPLER3D_HPP__

#include "CClippingAlgorithm.hpp"
#include "CVector3.hpp"

/** 3D Clipper algorithm. This algorithm allows the user to control a plane, represented by a quadrilateral polygon.
 * When the algorithm is fired, every voxel that lies on the "clipping side" of the plane is clipped out. The
 * "clipping side" is indicated to the user by drawing a red line at the side of the polygon which will have the
 * eliminated voxels. */
class CClipper3D : public CClippingAlgorithm
{
private:
	// PROPERTIES
	CVector3 m_planeNormal;    ///< The normal that is used to define the plane.
	CVector3 m_pointOnPlane;   ///< An arbitrary point that lies on the plane.

	bool m_isInitialized;      ///< A flag that indicates if the algorithm was initialized, in debug mode.

	// METHODS
	CClipper3D();
	~CClipper3D();

public:
	// STATIC METHODS
	static CClipper3D *New();

	// METHODS
	void setConfiguration( const CVector3 &planeNormal, const CVector3 &pointOnPlane );

	// VIRTUAL METHOD IMPLEMENTATIONS: CClippingAlgorithm
	virtual bool hideVoxel( double cellX, double cellY, double cellZ );
};

#endif

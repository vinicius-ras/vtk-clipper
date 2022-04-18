#ifndef CCLIPPER2D_HPP__
#define CCLIPPER2D_HPP__

#include "CClippingAlgorithm.hpp"
#include "CVector3.hpp"

/** 2D Clipper algorithm. This algorithm allows the user to define 2 points on the screen, which form a line.
 * After forming that line, the user selects one side of it to be clipped off from the image - every voxel which lies
 * on the selected side is clipped out. */
class CClipper2D : public CClippingAlgorithm
{
private:
	// PROPERTIES
	CVector3 m_planeNormal;    ///< The normal that is used to define the plane.
	CVector3 m_pointOnPlane;   ///< An arbitrary point that lies on the plane.
	bool m_erasePositives;     ///< A flag indicating if the points that lie on the positive side of the plane should be erased or kept.

	bool m_isInitialized;      ///< A flag used for debug purposes, indicating if the user code has initialized the object before it is used.


protected:
	// METHODS
	CClipper2D();
	virtual ~CClipper2D();


public:
	// STATIC METHODS
	static CClipper2D *New();

	// METHODS
	void setConfiguration( const CVector3 &planeNormal, const CVector3 &pointOnPlane, const CVector3 &pointOnSideToErase );

	// VIRTUAL METHOD IMPLEMENTATIONS: CClippingAlgorithm
	virtual bool hideVoxel( double cellX, double cellY, double cellZ );

};

#endif

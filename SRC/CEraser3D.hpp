#ifndef CERASER3D_HPP__
#define CERASER3D_HPP__

#include "CClippingAlgorithm.hpp"
#include "CLine.hpp"

/** 3D Eraser algorithm. Allows the user to control a cylinder through the scene. When the clipping algorithm is
 * fired, every voxel that lies within the cylinder's area is removed. It is important to notice that the cylinder
 * has infinite height, even if it is not drawn infinite in the scene to the user.
 */
class CEraser3D : public CClippingAlgorithm
{
private:
	// ENUMERATIONS
	/** Flags indicating what has been initialized from the Eraser3D algorithm.
	 * There are the indices of m_isInitialized vector. */
	enum EInitializableProperty
	{
		evCylinderAxis,
		evCylinderRadius,

		TOTAL_INITIALIZABLE_PROPERTIES
	};

	// PROPERTIES
	bool m_isInitialized[TOTAL_INITIALIZABLE_PROPERTIES];   ///< Used in debug mode to verify if all properties of the algorithm have been initialized before its executions.
	CLine m_cylinderAxis;        ///< The axis of the cylinder that will be used for clippping.
	double m_cylinderRadiusSq;   ///< The squared radius of the cylinder that will be used for clipping.


protected:
	CEraser3D();
	~CEraser3D();

public:
	// CONSTANTS
	static const double DEFAULT_CURSOR_RADIUS;

	// STATIC METHODS
	static CEraser3D *New();

	// METHODS
	void setCylinderAxis( const CLine &cylAxis );
	void setCylinderRadius( double cylinderRadius );
	double getCylinderRadius( void ) const;

	// VIRTUAL METHOD IMPLEMENTATIONS: CClippingAlgorithm
	virtual bool hideVoxel( double cellX, double cellY, double cellZ );
};

#endif

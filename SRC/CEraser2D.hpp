#ifndef CERASER2D_HPP__
#define CERASER2D_HPP__

#include "CClippingAlgorithm.hpp"
#include "CVector3.hpp"
#include "CLine.hpp"
#include <vtkRenderer.h>

/** 2D Eraser algorithm. This algorithm allows the user to position a circumference on the screen. When the algorithm
 * is fired, every voxel that lies inside the given circunference's projection is clipped out. */
class CEraser2D : public CClippingAlgorithm
{
private:
	// PROPERTIES
	bool m_isInitialized;       ///< A flag indicating if the algorithm was correctly initialized, for debug purposes.
	CLine m_coneAxis;           ///< A line that represents the axis of the cone which will be used for voxel selection.
	CVector3 m_coneHeadPoint;   ///< A point that represents the vertex of the cone which will be used for voxel selection.
	double m_cursorSize;        ///< Represents the size of the circular cursor.

protected:
	CEraser2D();
	~CEraser2D();

public:
	// CONSTANTS
	static const double DEFAULT_ERASER2D_SIZE;

	// STATIC METHODS
	static CEraser2D *New();

	// METHODS
	void setScreenReferencePoint( vtkRenderer *, int mx, int my );

	double getCursorSize( void ) const;
	void setCursorSize( double cursorSize );

	// VIRTUAL METHOD IMPLEMENTATIONS: CClippingAlgorithm
	virtual bool hideVoxel( double cellX, double cellY, double cellZ );
};

#endif

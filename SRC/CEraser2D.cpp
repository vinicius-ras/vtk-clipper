#include "CEraser2D.hpp"
#include "CGeometryAlgorithms.hpp"
#include <vtkCamera.h>
#include <cassert>


double TEMP_sliceRadiusDistanceFactorSq = 0;

// CLASS' CONSTANTS
const double CEraser2D::DEFAULT_ERASER2D_SIZE = 0.06;


// STATIC METHODS
/** Instantiates a #CEraser2D object.
 * @return Returns the new instance. */
CEraser2D *CEraser2D::New()
{
	return new CEraser2D();
}

// METHODS
/** Constructor. */
CEraser2D::CEraser2D()
	: m_isInitialized(false),
	m_coneAxis(),
	m_coneHeadPoint(),
	m_cursorSize( DEFAULT_ERASER2D_SIZE )
{
}


/** Destructor. */
CEraser2D::~CEraser2D()
{
}


/** Updates the reference point of the algorithm to the given point.
 * The reference point for the CEraser2D algorithm is the position, on the screen,
 * where the user has clicked. This method effectivelly updates the internal data
 * which is used to select voxels during the processing of the voxels.
 * @param mx The X coordinate of the reference point (mouse click) on the screen.
 * @param my The Y coordinate of the reference point (mouse click) on the screen.
 */
void CEraser2D::setScreenReferencePoint( vtkRenderer *renderer, int mx, int my )
{
	double nearClip, farClip;   // here, we will retrieve the distances to the Near and Far Clipping Planes.
	renderer->GetActiveCamera()->GetClippingRange( nearClip, farClip );   

	// Get the coordinates of the mouse's click point, into the near and far clipping planes.
	double mouseCoordsNear[4];   // coordinates of the point where the mouse was clicked, into the Near Clipping Plane.
	renderer->SetDisplayPoint( mx, my, 0 );
	renderer->DisplayToWorld();
	renderer->GetWorldPoint( mouseCoordsNear );

	double mouseCoordsFar[4];   // coordinates of the point where the mouse was clicked, into the Far Clipping Plane.
	renderer->SetDisplayPoint( mx, my, 1 );
	renderer->DisplayToWorld();
	renderer->GetWorldPoint( mouseCoordsFar );

	// Calculate the line that represents the axis of the selection cone
	m_coneAxis.p0 = CVector3( mouseCoordsNear[0], mouseCoordsNear[1], mouseCoordsNear[2] );
	m_coneAxis.p1 = CVector3( mouseCoordsFar[0], mouseCoordsFar[1], mouseCoordsFar[2] );

	// Calculate the point that represents the vertex of the cone
	CVector3 coneAxisDirection = m_coneAxis.p1 - m_coneAxis.p0;
	coneAxisDirection.normalize();

	m_coneHeadPoint = m_coneAxis.p0 - coneAxisDirection * nearClip;

	// Set the algorithm as "initialized"
	m_isInitialized = true;



	TEMP_sliceRadiusDistanceFactorSq = (m_cursorSize/nearClip) * (m_cursorSize/nearClip);
}


/** Retrieves the size of the Eraser 2D cursor.
 * @return Returns a value representing the size of the cursor. */
double CEraser2D::getCursorSize( void ) const
{
	return m_cursorSize;
}


/** Modifies the size of the Eraser 2D cursor.
 * @param cursorSize The new size for the cursor. */
void CEraser2D::setCursorSize( double cursorSize )
{
	m_cursorSize = cursorSize;
}


// METHODS IMPLEMENTATION: CClippingAlgorithm
bool CEraser2D::hideVoxel(double cellX, double cellY, double cellZ)
{
#ifdef DEBUG
	assert(m_isInitialized);
#endif

	CVector3 cellPoint( cellX, cellY, cellZ );

	CVector3 basePoint;
	double distCellToConeAxisSq = CGeometryAlgorithms::getPointToLineSqDistance( cellPoint, m_coneAxis, basePoint );
	double distProjectionToConeHeadSq = CGeometryAlgorithms::getPointToPointSqDistance( m_coneHeadPoint, basePoint );

	const double sliceRadiusSq = TEMP_sliceRadiusDistanceFactorSq * distProjectionToConeHeadSq;

	if ( distCellToConeAxisSq <= sliceRadiusSq )
		return true;
	return false;
}

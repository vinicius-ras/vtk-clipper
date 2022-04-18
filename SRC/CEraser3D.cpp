#include "CEraser3D.hpp"
#include "CGeometryAlgorithms.hpp"
#include <cassert>

#include <wx/msgdlg.h>

// CLASS' CONSTANTS
/** The default Radius for the 3D Eraser cursor. */
const double CEraser3D::DEFAULT_CURSOR_RADIUS = 36 * 36;

// STATIC METHODS
/** Instantiates a #CEraser3D object.
 * @return Returns the new instance. */
CEraser3D *CEraser3D::New()
{
	return new CEraser3D();
}

// METHODS
/** Constructor. */
CEraser3D::CEraser3D()
	: m_cylinderAxis(),
	m_cylinderRadiusSq( DEFAULT_CURSOR_RADIUS )
{
	for ( int f = static_cast<int>( TOTAL_INITIALIZABLE_PROPERTIES ) - 1; f >= 0; f-- )
		m_isInitialized[f] = false;
}


/** Destructor. */
CEraser3D::~CEraser3D()
{
}


/** Updates (and initializes) the algorithm by defining a line that will be used as the cylinder of
 * the Eraser3D algorithm.
 * @param cylAxis A line whose 2 points lie on the cylinder axis. This defines the cylinder to be used by the algorithm.
 */
void CEraser3D::setCylinderAxis( const CLine &cylAxis )
{
	m_cylinderAxis = cylAxis;
	m_isInitialized[evCylinderAxis] = true;
}


/** Updates (and initializes) the algorithm by defining the radius for the cylinder that will be used by the
 * Eraser3D algorithm.
 * @param cylinderRadius The radius of the cylinder which will be used for clipping.
 */
void CEraser3D::setCylinderRadius( double cylinderRadius )
{
	m_cylinderRadiusSq = cylinderRadius * cylinderRadius;
	m_isInitialized[evCylinderRadius] = true;
}


/** Retrieves the radius of the algorithm's cursor.
 * @return Returns the radius of the algorithm's 3D cylinder (cursor).
 */
double CEraser3D::getCylinderRadius( void ) const
{
	return sqrt( m_cylinderRadiusSq );
}


// METHODS IMPLEMENTATION: CClippingAlgorithm
bool CEraser3D::hideVoxel(double cellX, double cellY, double cellZ)
{
#ifdef DEBUG
	assert( m_isInitialized[evCylinderAxis] );
#endif

	CVector3 tempBasePoint;
	const double cellToCylinderAxisDistanceSq = CGeometryAlgorithms::getPointToLineSqDistance( CVector3(cellX,cellY,cellZ), m_cylinderAxis, tempBasePoint );
	assert(cellToCylinderAxisDistanceSq >= 0);

	return ( cellToCylinderAxisDistanceSq <= m_cylinderRadiusSq );
}

#include "CKinectMenuItem.hpp"
#include <wx/image.h>
#include <cassert>
#include <cmath>

#include <vtkPNGReader.h>
#include <vtkImageMapper.h>
#include <vtkRegularPolygonSource.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkFollower.h>
#include <vtkImageData.h>
#include <vtkCellData.h>
#include <vtkProperty2D.h>


// METHODS
/** Constructor.
 * @param imageFilePath A path to an image to be used for the Menu Item.
 * @param itemId The unique ID through which the menu item can be identified inside its containing menu.
 * @param groupId An identifier for the group this menu item belongs to, if any. Should be a positive value.
 *    The value #CKinectMenuItem::GROUP_ID_NONE can be specified if the Menu Item does not belong to any group.
 *    When a menu item belongs to a group, it is automatically treated as a RADIO Menu Item, allowing the user to
 *    select it by touching it. Only one Menu Item in a given group can be selected at a given time: the others are
 *    automatically set as "not active".
 * @param posX The position, into the Depth Map, where the */
CKinectMenuItem::CKinectMenuItem( const wxString &imageFilePath, int itemId, int groupId, int posX, int posY, int radius )
	: m_itemId( itemId ),
	m_groupId( groupId ),
	m_actor2D(),
	m_backActor2D(),
	m_isEnabled( true ),
	m_isActive( false ),
	m_isBirthLocked( true ),
	m_posX( posX ),
	m_posY( posY ),
	m_radius( radius ),
	m_imgWidth( 0 ),
	m_imgHeight( 0 )
{
	// The item's ID must be positive, or one of the special values (GROUP_ID_NONE is currently the only special value available)
	assert( itemId >= 0 || ( itemId == GROUP_ID_NONE ) );

	// Read the PNG file
	vtkSmartPointer<vtkPNGReader> pngReader = vtkSmartPointer<vtkPNGReader>::New();

	pngReader->SetFileName( imageFilePath.c_str() );
	pngReader->Update();

	// Get image's size
	vtkImageData *imgData = pngReader->GetOutput();
	int *imgDimensions = imgData->GetDimensions();
	m_imgWidth = imgDimensions[0];
	m_imgHeight = imgDimensions[1];

	// Create a mapper for the read image
	vtkSmartPointer<vtkImageMapper> imgMapper = vtkSmartPointer<vtkImageMapper>::New();
	imgMapper->SetInput( imgData );
	imgMapper->SetColorWindow( 255 );
	imgMapper->SetColorLevel( 127.5 );
	imgMapper->Update();

	// Create the Actor 2D which is used to render the Menu Item through VTK
	m_actor2D = vtkSmartPointer<vtkActor2D>::New();
	m_actor2D->SetMapper( imgMapper );

	// Creates a 2D "circunference"
	vtkSmartPointer<vtkRegularPolygonSource> polySrc = vtkSmartPointer<vtkRegularPolygonSource>::New();
	polySrc->SetRadius( radius );
	polySrc->SetNumberOfSides(15);
	polySrc->GeneratePolygonOn();
	polySrc->GeneratePolylineOff();

	// Use a 2D mapper for the circunference
	vtkPolyData *polyData = polySrc->GetOutput();

	vtkSmartPointer<vtkPolyDataMapper2D> polyMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
	polyMapper->SetInput( polyData );

	// Configure an Actor2D to use the circunference
	m_backActor2D = vtkSmartPointer<vtkActor2D>::New();
	m_backActor2D->SetMapper( polyMapper );
	this->updateBackgroundColor();

	// Update item's position
	this->setPosition( m_posX, m_posY );
}


/** Retrieves the identifier of the menu item.
 * @return Returns the identifier of the menu item. */
int CKinectMenuItem::getId( void ) const
{
	return m_itemId;
}


/** Retrieves the identifier of the menu item's group.
 * @return Returns the identifier of the menu item's group. */
int CKinectMenuItem::getGroupId( void ) const
{
	return m_groupId;
}


/** Sets the flag which indicates if an item is active or not.
 * @param bActive The flag specifying if the item is active or not. */
void CKinectMenuItem::setActive( bool bActive )
{
	m_isActive = bActive;
	updateBackgroundColor();
}


/** Verifies if the item is active.
 * @return Returns a flag specifying if the item is currently active. */
bool CKinectMenuItem::isActive( void ) const
{
	// Birth locking flag always forces returning FALSE
	if ( m_isBirthLocked )
		return false;

	// Disabled menu items also always return false
	if ( m_isEnabled == false )
		return false;

	// Return the ACTIVE flag value
	return m_isActive;
}


/** Verifies if the Menu Item is currently birth locked.
 * When a Menu Item is instanced, it automatically sets the Birth Locking Flag, which prevents it from
 * being activated. The Birth Locking Flag is only deactivated at the first time the application detects that there's
 * no collision between the hands of the player and the Menu Item. @n
 * @n
 * The purpose of the Birth Locking Flag is to prevent that a Menu Item gets instantly activated during the transition from
 * one Kinect Menu to other one, because it might happen that the player's hand position touch the items in the other menu
 * during this transition.
 * @return Returns a flag indicating if the Menu Item is birth locked. */
bool CKinectMenuItem::isBirthLocked( void ) const
{
	return m_isBirthLocked;
}


/** Turns the Birth Locking flag of the Menu Item off.
 * @see #CKinectMenuItem::isBirthLocked() */
void CKinectMenuItem::birthUnlock( void )
{
	m_isBirthLocked = false;
}



/** Modifies the position of the menu item.
 * @param posX The position for the menu item (X coordinate). */
void CKinectMenuItem::setX( int posX )
{
	double *actorPos = m_actor2D->GetPosition();
	this->setPosition( posX, actorPos[1] );
}


/** Modifies the position of the menu item.
 * @param posY The position for the menu item (Y coordinate). */
void CKinectMenuItem::setY( int posY )
{
	double *actorPos = m_actor2D->GetPosition();
	this->setPosition( actorPos[0], posY );
}


/** Modifies the position of the menu item.
 * @param posX The position for the menu item (X coordinate).
 * @param posY The position for the menu item (Y coordinate). */
void CKinectMenuItem::setPosition( int posX, int posY )
{
	m_posX = posX;
	m_posY = posY;

	m_actor2D->SetPosition( posX, posY );
	m_backActor2D->SetPosition( this->getItemCenterX(), this->getItemCenterY() );
}


/** Positions the item from the center of a circunference that is positioned at the middle of
 * the Kinect Rendering Panel.
 * @param degrees The degrees at which the item should be positioned from the circunference.
 * @param distanceFromCenter The distance from the center of the circunference. */
void CKinectMenuItem::setRadialPosition( double degrees, double distanceFromCenter )
{
	// Calculate the position from the center
	const int centerX = 320 / 2,
		centerY = 240 / 2,
		imgHalfWidth = m_imgWidth / 2,
		imgHalfHeight = m_imgHeight / 2;

	const double radianAngle = degrees * (3.14159265 / 180);
	const int newX = centerX - imgHalfWidth + static_cast<int>( cos( radianAngle ) * distanceFromCenter );
	const int newY = centerY - imgHalfHeight + static_cast<int>( sin( radianAngle ) * distanceFromCenter );

	// Apply the new positioning
	this->setPosition( newX, newY );
}


/** Retrieves the position of the menu item.
 * @return Returns the position of the menu item (X coordinate). */
int CKinectMenuItem::getX( void ) const
{
	return m_posX;
}


/** Retrieves the position of the menu item.
 * @return Returns the position of the menu item (Y coordinate). */
int CKinectMenuItem::getY( void ) const
{
	return m_posY;
}


/** Retrives the coordinates of the center of the Menu Item.
 * @return Returns the X coordinate of the center of the Menu Item. */
int CKinectMenuItem::getItemCenterX( void ) const
{
	return m_posX + m_imgWidth / 2;
}


/** Retrives the coordinates of the center of the Menu Item.
 * @return Returns the Y coordinate of the center of the Menu Item. */
int CKinectMenuItem::getItemCenterY( void ) const
{
	return m_posY + m_imgHeight / 2;
}


/** Modifies the radius of the menu item.
 * @param radius The radius for the menu item. */
void CKinectMenuItem::setRadius( int radius )
{
	m_radius = radius;
}


/** Retrieves the radius of the menu item.
 * @return Returns the radius for the menu item. */
int CKinectMenuItem::getRadius( void ) const
{
	return m_radius;
}


/** Enables or disables the menu item.
 * @param bEnabled A flag indicating if the menu item is enabled or disabled. */
void CKinectMenuItem::setEnabled( bool bEnabled )
{
	m_isEnabled = bEnabled;
}


/** Verifies if the menu item is currently enabled or disabled.
 * @return Returns a flag indicating if the menu item is currently enabled. */
bool CKinectMenuItem::isEnabled( void ) const
{
	return m_isEnabled;
}


/** Retrieves the VTK's Actor 2D which represents the Menu Item's loaded image, for
 * rendering it through VTK.
 * @return Returns a smart pointer which references the VTK's Actor 2D through which
 *    the Menu Item is rendered. */
vtkSmartPointer<vtkActor2D> CKinectMenuItem::getActor2D( void ) const
{
	return m_actor2D;
}


/** Retrieves the VTK's Actor 2D which represents the Menu Item's background.
 * @return Returns a smart pointer which references the VTK's Actor 2D through which
 *    the Menu Item's background is rendered. */
vtkSmartPointer<vtkActor2D> CKinectMenuItem::getBackgroundActor2D( void ) const
{
	return m_backActor2D;
}


/** Updates the color of the background of the menu item, according to its activation state. */
void CKinectMenuItem::updateBackgroundColor( void )
{
	vtkProperty2D *property2D = m_backActor2D->GetProperty();
	if ( m_isActive )
		property2D->SetColor( 0, 1, 0 );
	else
		property2D->SetColor( 0, 0.4, 0 );
}


/** Used when a menu is being destructed, to remove all the actors its' menu items have created.
 * @param renderer The renderer used by the menu. */
void CKinectMenuItem::removeActors( vtkSmartPointer<vtkRenderer> renderer )
{
	renderer->RemoveActor2D( m_actor2D );
	renderer->RemoveActor2D( m_backActor2D );
}

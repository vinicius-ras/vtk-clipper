#ifndef CKINECTMENUITEM_HPP__
#define CKINECTMENUITEM_HPP__

#include <wx/string.h>
#include <wx/dc.h>
#include <wx/bitmap.h>
#include <vtkSmartPointer.h>
#include <vtkActor2D.h>
#include <vtkRenderer.h>

/** Represents a item for a Kinect Menu (drawn into the Kinect Rendering Panel).
 * These menus are composed of circular buttons which can be "touched" by the user, while in Kinect mode, to activate these
 * items (and their corresponding actions). */
class CKinectMenuItem
{
private:
	// PROPERTIES
	int m_itemId;           /**< An identifier for the Menu Item, which must be unique inside the Menu the item belongs to. */
	int m_groupId;          /**< An identifier for a group, used by Radio Menu Items. */
	vtkSmartPointer<vtkActor2D> m_actor2D;       /**< The VTK's Actor2D object which represents the Menu Item to be rendered. */
	vtkSmartPointer<vtkActor2D> m_backActor2D;   /**< The VTK's Actor2D object which represents the Menu Item's background. */
	bool m_isEnabled;       /**< A flag indicating if the menu item is enabled or disabled. */
	bool m_isActive;        /**< A flag indicating if the item is active (user has activated its action). */
	bool m_isBirthLocked;   /**< A flag used to lock the Menu Item's "active state", in case the item gets instanced
							 * at a place in which it intersects one or more of the player's hands. This is used so that, when
							 * there's a transition between two menus, the Menu Items of the new menu do not get instantly activated
							 * because of the player's hand intersection with it. */

	int m_posX;         /**< The position (X coordinate) in the Depth Map of the circular button used by the menu item. */
	int m_posY;         /**< The position (Y coordinate) in the Depth Map of the circular button used by the menu item. */
	int m_radius;       /**< The radius of the circular button used by the menu item. */
	int m_imgWidth;     /**< The width of the image used by this menu item. */
	int m_imgHeight;    /**< The height of the image used by this menu item. */


	// METHODS
	CKinectMenuItem( const CKinectMenuItem & ) { /* Cannot be used. */ }

	void updateBackgroundColor( void );
public:
	// CONSTANTS
	/** This constant indicates a Menu Item does not belong to any group. */
	static const int GROUP_ID_NONE = -1;
	/** Specified the default radius for a menu item. */
	static const int DEFAULT_MENU_ITEM_RADIUS = 20;
	

	// METHODS
	explicit CKinectMenuItem( const wxString &imageFilePath, int itemId, int groupId = GROUP_ID_NONE,
		int posX = 0, int posY = 0, int radius = DEFAULT_MENU_ITEM_RADIUS );

	int getId( void ) const;
	int getGroupId( void ) const;

	void setActive( bool bActive );
	bool isActive( void ) const;

	bool isBirthLocked( void ) const;
	void birthUnlock( void );

	void setX( int posX );
	void setY( int posY );
	void setPosition( int posX, int posY );
	void setRadialPosition( double degrees, double distanceFromCenter );
	int getX( void ) const;
	int getY( void ) const;
	int getItemCenterX( void ) const;
	int getItemCenterY( void ) const;

	void setRadius( int radius );
	int getRadius( void ) const;

	void setEnabled( bool bEnabled );
	bool isEnabled( void ) const;

	vtkSmartPointer<vtkActor2D> getActor2D( void ) const;
	vtkSmartPointer<vtkActor2D> getBackgroundActor2D( void ) const;

	void removeActors( vtkSmartPointer<vtkRenderer> renderer );
};

#endif

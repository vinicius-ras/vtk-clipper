#ifndef CKINECTMENU_HPP__
#define CKINECTMENU_HPP__

#include <map>
#include <memory>
#include <wx/dc.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>

class CKinectMenuItem;

/** Represents a Kinect Menu, which is a menu composed by circular buttons and drawn into the Kinect Rendering Panel.
 * A menu contains several CKinectMenuItem objects, which describe the items that belong to the menu. */
class CKinectMenu
{
public:
	// TYPEDEFS
	/** Shared pointer to a Kinect Menu Item. */
	typedef std::shared_ptr<CKinectMenuItem> TKinectMenuItemSharedPtr;

	// METHODS
	explicit CKinectMenu( vtkSmartPointer<vtkRenderer> renderer );
	virtual ~CKinectMenu();

	void addMenuItem( TKinectMenuItemSharedPtr menuItem );
	CKinectMenuItem &getMenuItem( int itemId ) const;

	void activateMenuItem( int itemId, bool bActivate );
	bool isMenuItemActive( int itemId ) const;

	void updateMenuState( void );

	vtkSmartPointer<vtkRenderer> getRenderer( void ) const;

	// VIRTUAL METHODS (overridable)
	virtual void processMenuItems( void );

private:
	// TYPEDEFS
	/** A map which associates Kinect Menu Item IDs to their corresponding shared pointers. */
	typedef std::map<int, TKinectMenuItemSharedPtr> TKinectMenuItemMap;
	/** A map that will be used to keep which items belong to each group. Key: group ID. Value: menu item. */
	typedef std::multimap<int, TKinectMenuItemSharedPtr> TItemGroupsMap;

	// PROPERTIES
	TKinectMenuItemMap m_itemsMap;   /**< A map containing all the items that have been added to the menu, with their Ids as the map keys. */
	TItemGroupsMap m_groupMembers;   /**< A map which will keep which Menu Items belong to each group. */
	vtkSmartPointer<vtkRenderer> m_kinectVTKRenderer;   /** Reference to the renderer used by the Menu. */

	// METHODS
	CKinectMenu( const CKinectMenu & ) { /* Cannot be used. */ }
};

#endif

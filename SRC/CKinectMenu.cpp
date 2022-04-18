#include "CKinectMenu.hpp"
#include "CKinectMenuItem.hpp"

#include "CApp.hpp"
#include "CKinectInputProcessor.hpp"
#include "inputDevices.hpp"

#include <set>





#include "CAppConsole.hpp"

#include <wx/wx.h>

// METHODS
/** Constructor. */
CKinectMenu::CKinectMenu( vtkSmartPointer<vtkRenderer> renderer )
	: m_itemsMap(),
	m_groupMembers(),
	m_kinectVTKRenderer( renderer )
{
}


/** Destructor. */
CKinectMenu::~CKinectMenu()
{
	TKinectMenuItemMap::iterator it = m_itemsMap.begin(), endIt = m_itemsMap.end();
	for ( ; it != endIt; it++ )
		it->second->removeActors( m_kinectVTKRenderer );
}


/** Adds an item to the menu.
 * @param menuItem The item to be added to the menu. */
void CKinectMenu::addMenuItem( TKinectMenuItemSharedPtr menuItem )
{
	bool insResult = m_itemsMap.insert( std::make_pair( menuItem->getId(), menuItem ) ).second;
	assert( insResult );  // another Menu Item with the same ID has already been inserted!

	// For elements that belong to a group, also insert them into the group members map
	const int groupId = menuItem->getGroupId();
	if ( groupId >= 0 )
		m_groupMembers.insert( std::make_pair( groupId, menuItem ) );

	// Add the corresponding actor to the renderer, correcting its Y position
	menuItem->setY( menuItem->getY() );
	m_kinectVTKRenderer->AddActor2D( menuItem->getBackgroundActor2D() );
	m_kinectVTKRenderer->AddActor2D( menuItem->getActor2D() );
}


/** Retrieves a Menu Item from the Menu.
 * @param itemId The identifier of the Menu Item to be retrieved.
 * @return Returns the object which represents the given Menu Item. */
CKinectMenuItem &CKinectMenu::getMenuItem( int itemId ) const
{
	TKinectMenuItemMap::const_iterator it = m_itemsMap.find( itemId );
	assert( it != m_itemsMap.end() );
	return *it->second;
}


/** Activates the given menu item.
 * @param itemId The identifier for the item to be activated.
 * @param bActivate A flag indicating if the item should be activated or deactivated. */
void CKinectMenu::activateMenuItem( int itemId, bool bActivate )
{
	CKinectMenuItem &menuItem = getMenuItem( itemId );

	// If the item is being activated and if it belongs to a group, deactivate all items from the
	// group before activating the current one
	const int groupId = menuItem.getGroupId();
	if ( bActivate && groupId >= 0 )
	{
		TItemGroupsMap::iterator it = m_groupMembers.find( groupId ), endIt = m_groupMembers.end();
		for ( ; it != endIt && it->second->getGroupId() == groupId; it++ )
			it->second->setActive( false );
	}
	menuItem.setActive( bActivate );
}


/** Verifies if the given menu item is active or not.
 * @param itemId The identifier for the item to be verified.
 * @return Returns a flag indicating if the Menu Item is active or not. */
bool CKinectMenu::isMenuItemActive( int itemId ) const
{
	CKinectMenuItem &menuItem = getMenuItem( itemId );
	return menuItem.isActive();
}


/** Updates the Menu state, by processing the current position of user's hands and updating the state of
 * the Menu Items according to those positions. */
void CKinectMenu::updateMenuState()
{
	// Only process if there's a Kinect device in use
	CApp &app = wxGetApp();
	if ( app.getCurrentInputDeviceId() != evInputDeviceKinect )
		return;

	// Retrieve user's hands' positions
	CInputDeviceProcessor &inputProcessor = app.getCurrentInputDeviceProcessor();

#ifdef DEBUG
	CKinectInputProcessor &kinectProcessor = dynamic_cast<CKinectInputProcessor &>( inputProcessor );
#else
	CKinectInputProcessor &kinectProcessor = static_cast<CKinectInputProcessor &>( inputProcessor );
#endif

	int handsPosX[2], handsPosY[2];
	kinectProcessor.getHandPosition( true, handsPosX[0], handsPosY[0] );
	kinectProcessor.getHandPosition( false, handsPosX[1], handsPosY[1] );

	// Verify if the user's hands touch some menus, and activate them
	std::set<int> itemsInTouch;   // keeps the items that are being touched by the user

	const int totalUserHands = 2;   // yes man, maybe you have 5 hands, IDK
	for ( int h = 0; h < totalUserHands; h++ )
	{
		// Verify if hand position is invalid
		const int handX = handsPosX[h], handY = handsPosY[h];
		if ( handX == CKinectInputProcessor::INVALID_HAND_POSITION || handY == CKinectInputProcessor::INVALID_HAND_POSITION )
			continue;

		const int handRadius = kinectProcessor.getHandCursorRadius();

		// Does the hand touch any of the menu items?
		TKinectMenuItemMap::const_iterator it, endIt = m_itemsMap.end();
		for ( it = m_itemsMap.begin(); it != endIt; it++ )
		{
			// Retrieve current item's properties
			const CKinectMenuItem &item = *it->second;
			const int itemPosX = item.getItemCenterX(),
				itemPosY = item.getItemCenterY(),
				itemRadius = item.getRadius(),
				itemId = item.getId();

			const int squaredRadiusSum = (handRadius + itemRadius) * (handRadius + itemRadius);

			// Verify for intersection
			const int squareDistX = (itemPosX - handX) * (itemPosX - handX);
			const int squareDistY = (itemPosY - handY) * (itemPosY - handY);
			const int squaredDist = squareDistX + squareDistY;

			if ( squaredDist <= squaredRadiusSum )
			{
				itemsInTouch.insert( itemId );
				this->activateMenuItem( itemId, true );
			}
		}
	}

	// Process the items we're not touching
	TKinectMenuItemMap::const_iterator it, endIt = m_itemsMap.end();
	for ( it = m_itemsMap.begin(); it != endIt; it++ )
	{
		// Ignore touched items
		if ( itemsInTouch.count( it->first ) > 0 )
			continue;

		// If the item has got its Birth Lock Flag set, unset it
		CKinectMenuItem &item = *it->second;
		if ( item.isBirthLocked() )
			item.birthUnlock();

		// If the item belongs to no group, then deactivate the item
		if ( item.getGroupId() < 0 )
			this->activateMenuItem( item.getId(), false );
	}

}


/** Inheriting menus should override this method to process the actions associated to each of its menu items. */
void CKinectMenu::processMenuItems( void )
{
}


/** Retrieves the VTK's Renderer used by the Kinect Menu.
 * @return Returns a reference to the VTK renderer. */
vtkSmartPointer<vtkRenderer> CKinectMenu::getRenderer( void ) const
{
	return m_kinectVTKRenderer;
}

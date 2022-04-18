#include "CKinectMenuSelectTool.hpp"
#include "CKinectMenuItem.hpp"

#include "CApp.hpp"
#include "CMainFrame.hpp"
#include "CKinectRenderer.hpp"
#include "CKinectMenuRotationStage.hpp"
#include "clippingAlgorithms.hpp"

#include <vector>
#include <set>

// FILE ENUMERATIONS
/** An enumeration providing all the identifiers for the Menu Items available into the Select Tool Menu. */
enum EKinectMenuSelectToolItems
{
	evMenuItemToolEraser,
	evMenuItemToolClipper,
	evMenuItemButtonNext,
};

/** An enumeration providing all the identifiers for the Menu Items' Groups available into the Select Tool Menu. */
enum EKinectMenuSelectToolGroups
{
	evMenuItemGroupTools,
};


// METHODS
/** Constructor. */
CKinectMenuSelectTool::CKinectMenuSelectTool( vtkSmartPointer<vtkRenderer> renderer )
	: CKinectMenu( renderer ),
	m_lastClippingAlgorithm( evAlgorithmNone )
{
	// Create the Menu Items
	std::vector<TKinectMenuItemSharedPtr> menuItems;

	TKinectMenuItemSharedPtr menuItemEraser( new CKinectMenuItem( _T( "MEDIA/tbEraser2D.png" ), evMenuItemToolEraser, evMenuItemGroupTools ) );
	TKinectMenuItemSharedPtr menuItemClipper2D( new CKinectMenuItem( _T( "MEDIA/tbClipper2D.png" ), evMenuItemToolClipper, evMenuItemGroupTools ) );
	TKinectMenuItemSharedPtr menuItemNext( new CKinectMenuItem( _T( "MEDIA/buttonNext.png" ), evMenuItemButtonNext, CKinectMenuItem::GROUP_ID_NONE ) );

	menuItems.push_back( menuItemEraser );
	menuItems.push_back( menuItemClipper2D );
	menuItems.push_back( menuItemNext );

	menuItemNext->setRadialPosition( 0, 100 );
	menuItemEraser->setRadialPosition( 45, 100 );
	menuItemClipper2D->setRadialPosition( 135, 100 );

	// Add all the items to the menu
	const size_t totalItems = menuItems.size();
	for ( size_t i = 0; i < totalItems; i++ )
		this->addMenuItem( menuItems[i] );

	// NEXT button (menu item) starts disabled
	this->getMenuItem( evMenuItemButtonNext ).setEnabled( false );
}

/** Processes the menu items' actions. */
void CKinectMenuSelectTool::processMenuItems( void )
{
	CKinectMenuItem &menuItemEraserTool = this->getMenuItem( evMenuItemToolEraser ),
		&menuItemClipperTool = this->getMenuItem( evMenuItemToolClipper ),
		&menuItemButtonNext = this->getMenuItem( evMenuItemButtonNext );

	// Detect the clipping algorithm the user is currently touching
	EClippingAlgorithm touchingClippingAlgorithm = evAlgorithmNone;
	if ( menuItemEraserTool.isActive() )
		touchingClippingAlgorithm = evAlgorithmEraser3D;
	else if ( menuItemClipperTool.isActive() )
		touchingClippingAlgorithm = evAlgorithmClipper3D;

	// Whenever there's a valid algorithm enabled, the NEXT button should be also enabled.
	menuItemButtonNext.setEnabled( touchingClippingAlgorithm != evAlgorithmNone );

	// Has the algorithm changed? Update GUI, if so.
	CApp &app = wxGetApp();
	CMainFrame &mainFrame = app.getMainFrame();
	if ( m_lastClippingAlgorithm != touchingClippingAlgorithm )
	{
		mainFrame.updateSelectedClippingAlgorithm( touchingClippingAlgorithm );
		m_lastClippingAlgorithm = touchingClippingAlgorithm;
	}

	// If the user has selected a valid algorithm, allow him to proceed
	std::set<EClippingAlgorithm> validAlgorithms;   // set of valid algorithms
	validAlgorithms.insert( evAlgorithmClipper3D );
	validAlgorithms.insert( evAlgorithmEraser3D );

	if ( validAlgorithms.count( touchingClippingAlgorithm ) > 0 && menuItemButtonNext.isActive() )
	{
		CKinectRenderer::TKinectMenuSharedPtr menuPtr( new CKinectMenuRotationStage( this->getRenderer() ) );
		mainFrame.getKinectRenderer().setKinectMenu( menuPtr );
	}
}

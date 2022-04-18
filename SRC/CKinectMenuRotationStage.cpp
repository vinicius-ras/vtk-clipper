#include "CKinectMenuRotationStage.hpp"
#include "CKinectMenuItem.hpp"

#include "CApp.hpp"
#include "CMainFrame.hpp"
#include "CVolumeClipperAppInteractorStyle.hpp"
#include "CKinectRenderer.hpp"
#include "CKinectMenuSelectTool.hpp"
#include "CKinectMenuTranslationStage.hpp"

#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkTextSource.h>
#include <vtkPolyDataMapper2D.h>

#include <vector>




#include "CAppConsole.hpp"

// FILE ENUMERATIONS
/** An enumeration providing all the identifiers for the Menu Items available into the Rotation Stage Menu. */
enum EKinectMenuRotationStageItems
{
	evMenuItemButtonX,
	evMenuItemButtonY,
	evMenuItemButtonZ,
	evMenuItemButtonInc,
	evMenuItemButtonDec,
	evMenuItemButtonBack,
	evMenuItemButtonNext,
};

/** An enumeration providing all the identifiers for the Menu Items' Groups available into the Rotation Stage Menu. */
enum EKinectMenuRotationStageGroups
{
	evMenuItemGroupAxes,
};


// METHODS
/** Constructor. */
CKinectMenuRotationStage::CKinectMenuRotationStage( vtkSmartPointer<vtkRenderer> renderer )
	: CKinectMenu( renderer ),
	m_textActor()
{
	// Create the Menu Items
	std::vector<TKinectMenuItemSharedPtr> menuItems;

	TKinectMenuItemSharedPtr menuItemButtonX( new CKinectMenuItem( _T( "MEDIA/buttonX.png" ), evMenuItemButtonX, evMenuItemGroupAxes, 40, 95 ) ),
		menuItemButtonY( new CKinectMenuItem( _T( "MEDIA/buttonY.png" ), evMenuItemButtonY, evMenuItemGroupAxes, 90, 50 ) ),
		menuItemButtonZ( new CKinectMenuItem( _T( "MEDIA/buttonZ.png" ), evMenuItemButtonZ, evMenuItemGroupAxes, 150, 20 ) ),
		menuItemButtonInc( new CKinectMenuItem( _T( "MEDIA/buttonInc.png" ), evMenuItemButtonInc, CKinectMenuItem::GROUP_ID_NONE, 230, 20 ) ),
		menuItemButtonDec( new CKinectMenuItem( _T( "MEDIA/buttonDec.png" ), evMenuItemButtonDec, CKinectMenuItem::GROUP_ID_NONE, 265, 55 ) ),
		menuItemButtonPrev( new CKinectMenuItem( _T( "MEDIA/buttonPrevious.png" ), evMenuItemButtonBack, CKinectMenuItem::GROUP_ID_NONE, 80, 120 ) ),
		menuItemButtonNext( new CKinectMenuItem( _T( "MEDIA/buttonNext.png" ), evMenuItemButtonNext, CKinectMenuItem::GROUP_ID_NONE, 240, 120 ) );

	menuItems.push_back( menuItemButtonX );
	menuItems.push_back( menuItemButtonY );
	menuItems.push_back( menuItemButtonZ );
	menuItems.push_back( menuItemButtonInc );
	menuItems.push_back( menuItemButtonDec );
	menuItems.push_back( menuItemButtonPrev );
	menuItems.push_back( menuItemButtonNext );

	menuItemButtonX->setRadialPosition( 130, 100 );
	menuItemButtonY->setRadialPosition( 160, 100 );
	menuItemButtonZ->setRadialPosition( 190, 100 );
	menuItemButtonInc->setRadialPosition( 50, 100 );
	menuItemButtonDec->setRadialPosition( 20, 100 );
	menuItemButtonPrev->setRadialPosition( 220, 100 );
	menuItemButtonNext->setRadialPosition( -40, 100 );

	// Add all the items to the menu
	const size_t totalItems = menuItems.size();
	for ( size_t i = 0; i < totalItems; i++ )
		this->addMenuItem( menuItems[i] );

	// Select X axis by default
	this->activateMenuItem( evMenuItemButtonX, true );

	// Add a text indicating the Menu's Name
	m_textActor = vtkSmartPointer<vtkActor2D>::New();

	vtkSmartPointer<vtkTextSource> textSource = vtkSmartPointer<vtkTextSource>::New();
	textSource->SetText( "ROTATING CURSOR" );
	textSource->BackingOff();
	textSource->SetForegroundColor( 1, 0, 0 );

	vtkSmartPointer<vtkPolyDataMapper2D> textMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
	textMapper->SetInput( textSource->GetOutput() );
	
	m_textActor = vtkSmartPointer<vtkActor2D>::New();
	m_textActor->SetMapper( textMapper );
	renderer->AddActor2D( m_textActor );
}


/** Destructor. */
CKinectMenuRotationStage::~CKinectMenuRotationStage()
{
	this->getRenderer()->RemoveActor2D( m_textActor );
}


/** Applies the rotation to the current Clipping Tool's Cursor.
 * @param axis The axis we're rotating around.
 * @param camTransform A transformation which represents the camera's orientation.
 * @param rotationValue The value used to rotate the cursor around the given axis. */
void CKinectMenuRotationStage::applyCursorRotation( ERotationAxes axis, vtkSmartPointer<vtkTransform> camTransform, double rotationValue )
{
	// Gather necessary data
	CApp &app = wxGetApp();
	CMainFrame &mainFrame = app.getMainFrame();
	CVolumeClipperAppInteractorStyle &interactorStyle = *mainFrame.getInteractorStyle();
	vtkProp3D *cursorActor = static_cast<vtkProp3D *>( interactorStyle.getCursorActor(0).GetPointer() );

	CAppConsole &console = CAppConsole::getInstance();
	console.clearLog();
	console.log( wxString::Format( "Rodando no eixo %s, %s", (axis == evAxisX ? "X" : (axis == evAxisY ? "Y" : "Z") ), rotationValue > 0 ? "INC" : "DEC" ) );

	// Calculate a vector for rotation
	double relativeRotVector[4] = { 0, 0, 0, 1 }, resultingRotVector[4];

	switch ( axis )
	{
	case evAxisX:
		relativeRotVector[0] = 1;
		break;
	case evAxisY:
		relativeRotVector[1] = 1;
		break;
	case evAxisZ:
		relativeRotVector[2] = 1;
		break;
	}

	camTransform->MultiplyPoint( relativeRotVector, resultingRotVector );

	// Rotate the cursor
	cursorActor->RotateWXYZ( rotationValue, resultingRotVector[0], resultingRotVector[1], resultingRotVector[2] );

	// Update screen (with reduced quality for optimized frame rate, if necessary)
	interactorStyle.startRefreshRateChange();
}


/** Processes the menu items' actions. */
void CKinectMenuRotationStage::processMenuItems( void )
{
	CApp &app = wxGetApp();
	CMainFrame &mainFrame = app.getMainFrame();

	CKinectMenuItem &menuButtonX = this->getMenuItem( evMenuItemButtonX ),
		&menuButtonY = this->getMenuItem( evMenuItemButtonY ),
		&menuButtonZ = this->getMenuItem( evMenuItemButtonZ ),
		&menuButtonInc = this->getMenuItem( evMenuItemButtonInc ),
		&menuButtonDec = this->getMenuItem( evMenuItemButtonDec ),
		&menuButtonBack = this->getMenuItem( evMenuItemButtonBack ),
		&menuButtonNext = this->getMenuItem( evMenuItemButtonNext );

	// Has the user pressed the NEXT or BACK buttons?
	if ( menuButtonBack.isActive() )
	{
		// Go back to the SELECT TOOL menu
		CKinectRenderer::TKinectMenuSharedPtr menuPtr( new CKinectMenuSelectTool( this->getRenderer() ) );
		mainFrame.getKinectRenderer().setKinectMenu( menuPtr );

		return;
	}

	if ( menuButtonNext.isActive() )
	{
		// Go to the TRANSLATION STAGE menu
		CKinectRenderer::TKinectMenuSharedPtr menuPtr( new CKinectMenuTranslationStage( this->getRenderer() ) );
		mainFrame.getKinectRenderer().setKinectMenu( menuPtr );

		return;
	}

	// Verify which axis is selected
	ERotationAxes selectedAxis;

	if ( menuButtonX.isActive() )
		selectedAxis = evAxisX;
	else if ( menuButtonY.isActive() )
		selectedAxis = evAxisY;
	else
		selectedAxis = evAxisZ;

	// Retrieve camera's information
	vtkSmartPointer<vtkRenderer> renderer = mainFrame.getRenderer();

	vtkCamera *cam = renderer->GetActiveCamera();
	double *camOrient = cam->GetOrientation();
	double *camPos = cam->GetPosition();

	// Create a transformation to match the camera's angle
	vtkSmartPointer<vtkTransform> camTransform = vtkSmartPointer<vtkTransform>::New();
	camTransform->RotateY( -camOrient[1] );
	camTransform->RotateX( -camOrient[0] );
	camTransform->RotateZ( -camOrient[2] );

	// Increment or decrement rotation in the given axis
	const double rotationAngle = 5;
	const double sensitivity = 1;
	const double finalRotationValue = rotationAngle * sensitivity;

	if ( menuButtonInc.isActive() )
		this->applyCursorRotation( selectedAxis, camTransform, finalRotationValue );
	if ( menuButtonDec.isActive() )
		this->applyCursorRotation( selectedAxis, camTransform, -finalRotationValue );
}

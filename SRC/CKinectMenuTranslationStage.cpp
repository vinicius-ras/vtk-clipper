#include "CKinectMenuTranslationStage.hpp"
#include "CKinectMenuItem.hpp"

#include "CApp.hpp"
#include "CMainFrame.hpp"
#include "CVolumeClipperAppInteractorStyle.hpp"
#include "CKinectRenderer.hpp"
#include "CKinectMenuRotationStage.hpp"
#include "CKinectMenuSelectTool.hpp"

#include "CLine.hpp"
#include "CEraser3D.hpp"
#include "CClipper3D.hpp"

#include "clippingAlgorithms.hpp"

#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkTextSource.h>
#include <vtkPolyDataMapper2D.h>

#include <vector>




#include "CAppConsole.hpp"

// FILE ENUMERATIONS
/** An enumeration providing all the identifiers for the Menu Items available into the Translation Stage Menu. */
enum EKinectMenuTranslationStageItems
{
	evMenuItemButtonX,
	evMenuItemButtonY,
	evMenuItemButtonZ,
	evMenuItemButtonInc,
	evMenuItemButtonDec,
	evMenuItemButtonBack,
	evMenuItemButtonAccept,
};

/** An enumeration providing all the identifiers for the Menu Items' Groups available into the Translation Stage Menu. */
enum EKinectMenuTranslationStageGroups
{
	evMenuItemGroupAxes,
};


// METHODS
/** Constructor. */
CKinectMenuTranslationStage::CKinectMenuTranslationStage( vtkSmartPointer<vtkRenderer> renderer )
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
		menuItemButtonNext( new CKinectMenuItem( _T( "MEDIA/buttonAccept.png" ), evMenuItemButtonAccept, CKinectMenuItem::GROUP_ID_NONE, 240, 120 ) );

	menuItemButtonX->setRadialPosition( 130, 100 );
	menuItemButtonY->setRadialPosition( 160, 100 );
	menuItemButtonZ->setRadialPosition( 190, 100 );
	menuItemButtonInc->setRadialPosition( 50, 100 );
	menuItemButtonDec->setRadialPosition( 20, 100 );
	menuItemButtonPrev->setRadialPosition( 220, 100 );
	menuItemButtonNext->setRadialPosition( -40, 100 );

	menuItems.push_back( menuItemButtonX );
	menuItems.push_back( menuItemButtonY );
	menuItems.push_back( menuItemButtonZ );
	menuItems.push_back( menuItemButtonInc );
	menuItems.push_back( menuItemButtonDec );
	menuItems.push_back( menuItemButtonPrev );
	menuItems.push_back( menuItemButtonNext );


	// Add all the items to the menu
	const size_t totalItems = menuItems.size();
	for ( size_t i = 0; i < totalItems; i++ )
		this->addMenuItem( menuItems[i] );

	// Select X axis by default
	this->activateMenuItem( evMenuItemButtonX, true );

	// Add a text indicating the Menu's Name
	m_textActor = vtkSmartPointer<vtkActor2D>::New();

	vtkSmartPointer<vtkTextSource> textSource = vtkSmartPointer<vtkTextSource>::New();
	textSource->SetText( "TRANSLATING CURSOR" );
	textSource->BackingOff();
	textSource->SetForegroundColor( 1, 0, 0 );

	vtkSmartPointer<vtkPolyDataMapper2D> textMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
	textMapper->SetInput( textSource->GetOutput() );
	
	m_textActor = vtkSmartPointer<vtkActor2D>::New();
	m_textActor->SetMapper( textMapper );
	renderer->AddActor2D( m_textActor );
}


/** Destructor. */
CKinectMenuTranslationStage::~CKinectMenuTranslationStage()
{
	this->getRenderer()->RemoveActor2D( m_textActor );
}


/** Applies the translation to the current Clipping Tool's Cursor.
 * @param axis The axis we're translation on.
 * @param camTransform A transformation which represents the camera's orientation.
 * @param translationValue The value used to translate the cursor in the given axis. */
void CKinectMenuTranslationStage::applyCursorTranslation( ETranslationAxes axis, vtkSmartPointer<vtkTransform> camTransform, double translationValue )
{
	// Gather necessary data
	CApp &app = wxGetApp();
	CMainFrame &mainFrame = app.getMainFrame();
	CVolumeClipperAppInteractorStyle &interactorStyle = *mainFrame.getInteractorStyle();
	vtkProp3D *cursorActor = static_cast<vtkProp3D *>( interactorStyle.getCursorActor(0).GetPointer() );

	CAppConsole &console = CAppConsole::getInstance();
	console.clearLog();
	console.log( wxString::Format( "Transladando no eixo %s, %s", (axis == evAxisX ? "X" : (axis == evAxisY ? "Y" : "Z") ), translationValue > 0 ? "INC" : "DEC" ) );

	// Calculate a vector for translation
	double relativeTransVector[4] = { 0, 0, 0, 1 }, resultingTransVector[4];

	switch ( axis )
	{
	case evAxisX:
		relativeTransVector[0] = translationValue;
		break;
	case evAxisY:
		relativeTransVector[1] = translationValue;
		break;
	case evAxisZ:
		relativeTransVector[2] = translationValue;
		break;
	}

	camTransform->MultiplyPoint( relativeTransVector, resultingTransVector );

	// Rotate the cursor
	cursorActor->AddPosition( resultingTransVector[0], resultingTransVector[1], resultingTransVector[2] );

	// Update screen (with reduced quality for optimized frame rate, if necessary)
	interactorStyle.startRefreshRateChange();
}


/** Configures and starts the currently selected clipping algorithm. */
void CKinectMenuTranslationStage::startClippingAlgorithm( void )
{
	// Gather necessary data
	CApp &app = wxGetApp();
	CMainFrame &mainFrame = app.getMainFrame();
	CVolumeClipperAppInteractorStyle &interactorStyle = *mainFrame.getInteractorStyle();
	
	EClippingAlgorithm curClippingAlgorithmId = static_cast<EClippingAlgorithm>( interactorStyle.getClippingAlgorithmId() );
	vtkProp3D *cursorActor = static_cast<vtkProp3D *>( interactorStyle.getCursorActor(0).GetPointer() );
	CClippingAlgorithm &curAlgorithm = interactorStyle.getClippingAlgorithm();

	// Verify which algorithm we're trying to start
	switch ( curClippingAlgorithmId )
	{
	case evAlgorithmEraser3D:
		{
			// Calculate a point into the axis of the cylinder (will be used below)
			// We will use the cylinder's transformation matrix to calculate a point using the cylinder's orientation.
			const double relativePointPosP0[4] = { 0, 0, 0, 1 };
			const double relativePointPosP1[4] = { 0, 100, 0, 1 };
			double p0[4];
			double p1[4];

			cursorActor->GetMatrix()->MultiplyPoint( relativePointPosP0, p0 );
			cursorActor->GetMatrix()->MultiplyPoint( relativePointPosP1, p1 );

			// Define the axis of the Eraser3D cylinder.
			// The axis is a line, defined by two given points: p0 and p1.
			// We will be using p0 as the center of the cylinder, which always lies on the axis' line.
			// And p1 has been calculated above.
			CLine cylAxis;
			cylAxis.p0 = CVector3( p0[0], p0[1], p0[2] );
			cylAxis.p1 = CVector3( p1[0], p1[1], p1[2] );

			CEraser3D &eraser3D = static_cast<CEraser3D &>(curAlgorithm);
			eraser3D.setCylinderAxis( cylAxis );

			// Execute the clipping algorithm
			interactorStyle.startCurrentClippingAlgorithm();

			// This resets the clipping algorithm
			interactorStyle.setClippingAlgorithm( evAlgorithmNone );
			interactorStyle.setClippingAlgorithm( evAlgorithmEraser3D );
		}
		break;
	case evAlgorithmClipper3D:
		{
            // Calculate a point that lies in the plane and plane normal
            const double relativePointOnPlane[4] = { 0, 0, 0, 1 },
                relativePlaneNormal[4] = { 0, 0, 100, 1 };
            double coordPointOnPlane[4], coordsPlaneNormal[4];

            cursorActor->GetMatrix()->MultiplyPoint( relativePointOnPlane, coordPointOnPlane );
            cursorActor->GetMatrix()->MultiplyPoint( relativePlaneNormal, coordsPlaneNormal );

            CVector3 pointOnPlane( coordPointOnPlane[0], coordPointOnPlane[1], coordPointOnPlane[2] );
            CVector3 planeNormal( coordsPlaneNormal[0], coordsPlaneNormal[1], coordsPlaneNormal[2] );
            planeNormal = planeNormal - pointOnPlane;
            planeNormal.normalize();

			CClipper3D &clipper3D = static_cast<CClipper3D &>(curAlgorithm);
            clipper3D.setConfiguration( planeNormal, pointOnPlane );


            // Execute the clipping algorithm
			interactorStyle.startCurrentClippingAlgorithm();

            // This resets the clipping algorithm
            interactorStyle.setClippingAlgorithm( evAlgorithmNone );
            interactorStyle.setClippingAlgorithm( evAlgorithmClipper3D );
		}
		break;
	default:
		// Trying to start/process an unknown algorithm!
		assert( false );
		break;
	}

	// Force window repaint
	interactorStyle.startRefreshRateChange();
}


/** Processes the menu items' actions. */
void CKinectMenuTranslationStage::processMenuItems( void )
{
	CApp &app = wxGetApp();
	CMainFrame &mainFrame = app.getMainFrame();

	CKinectMenuItem &menuButtonX = this->getMenuItem( evMenuItemButtonX ),
		&menuButtonY = this->getMenuItem( evMenuItemButtonY ),
		&menuButtonZ = this->getMenuItem( evMenuItemButtonZ ),
		&menuButtonInc = this->getMenuItem( evMenuItemButtonInc ),
		&menuButtonDec = this->getMenuItem( evMenuItemButtonDec ),
		&menuButtonBack = this->getMenuItem( evMenuItemButtonBack ),
		&menuButtonAccept = this->getMenuItem( evMenuItemButtonAccept );

	// Has the user pressed the ACCEPT or BACK buttons?
	if ( menuButtonBack.isActive() )
	{
		// Go back to the ROTATION STAGE menu
		CKinectRenderer::TKinectMenuSharedPtr menuPtr( new CKinectMenuRotationStage( this->getRenderer() ) );
		mainFrame.getKinectRenderer().setKinectMenu( menuPtr );

		return;
	}

	if ( menuButtonAccept.isActive() )
	{
		// Go to the SELECT TOOL menu
		CKinectRenderer::TKinectMenuSharedPtr menuPtr( new CKinectMenuSelectTool( this->getRenderer() ) );
		mainFrame.getKinectRenderer().setKinectMenu( menuPtr );

		// Start current clipping algorithm
		this->startClippingAlgorithm();

		CApp &app = wxGetApp();
		CMainFrame &mainFrame = app.getMainFrame();
		CVolumeClipperAppInteractorStyle &interactorStyle = *mainFrame.getInteractorStyle();
		interactorStyle.setClippingAlgorithm( evAlgorithmNone );

		return;
	}

	// Verify which axis is selected
	ETranslationAxes selectedAxis;

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

	// Translate in the given axis
	const double translationAmount = 5;
	const double sensitivity = 1;
	const double finalTranslationValue = translationAmount * sensitivity;

	if ( menuButtonInc.isActive() )
		this->applyCursorTranslation( selectedAxis, camTransform, finalTranslationValue );
	if ( menuButtonDec.isActive() )
		this->applyCursorTranslation( selectedAxis, camTransform, -finalTranslationValue );
}

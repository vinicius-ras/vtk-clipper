#include <windows.h>
#include <NuiApi.h>

#include "CKinectRenderer.hpp"
#include "CApp.hpp"
#include "CKinectInputProcessor.hpp"

#include "inputDevices.hpp"

#include <algorithm>

#include "testes.hpp"

#include "CKinectMenuSelectTool.hpp"
#include "wxVTKRenderWindowInteractor.h"



#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkImageMapper.h>
#include <vtkPNGReader.h>
#include <vtkJPEGReader.h>
#include <vtkImageReader.h>
#include <vtkSphereSource.h>
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkTexturedActor2D.h>
#include <vtkImageData.h>
#include <vtkRegularPolygonSource.h>
#include <vtkProperty2D.h>
#include <vtkPlaneSource.h>

#include <sstream>
#include "CAppConsole.hpp"



// FILE STATIC CONSTANTS
/** The period, given in miliseconds, for a repainting of the Kinect Renderer to happen. */
static const int KINECT_RENDERER_TIMER_PERIOD = 1000 / APPLICATION_DESIRED_FPS;
/** An identifier for the Kinect Render Panel Repainting Timer. */
static const int KINECT_RENDERER_TIMER_ID = 0xABCD;

// EVENT TABLE
BEGIN_EVENT_TABLE( CKinectRenderer, wxEvtHandler )
	EVT_TIMER( KINECT_RENDERER_TIMER_ID, OnRenderAndProcessingTimer )
END_EVENT_TABLE()


// METHODS
/** Constructor. */
CKinectRenderer::CKinectRenderer( wxPanel *parent )
	: wxEvtHandler(),
	m_kinectVTKRenderer(),
	m_pKinectVTKWindow( NULL ),
	m_updateTimer(),
	m_pDepthData( NULL ),
	m_pDepthDataLength( 0 ),
	m_renderingType( evRenderingTypeFull ),
	m_menu(),
	m_isMouseLeftDown( false ),
	m_isMouseRightDown( false ),
	m_actorLeftHand(),
	m_actorRightHand()
{
	m_pKinectVTKWindow = new wxVTKRenderWindowInteractor( parent, wxID_ANY );
	parent->GetSizer()->Add( m_pKinectVTKWindow, 1, wxEXPAND );

	m_pKinectVTKWindow->wxWindowBase::SetSize( parent->GetSize() );

	// Configures the Kinect VTK widget
	m_kinectVTKRenderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWndKinect = m_pKinectVTKWindow->GetRenderWindow();

	renderWndKinect->AddRenderer( m_kinectVTKRenderer );
	m_pKinectVTKWindow->SetInteractorStyle( this );

	// Create background canvas, for drawing the user (through Kinect's Depth Map)
	const int canvasWidth = 320;
	const int canvasHeight = 240;

	m_backgroundCanvas = vtkSmartPointer<vtkImageCanvasSource2D>::New();
	m_backgroundCanvas->SetScalarTypeToUnsignedChar();
	m_backgroundCanvas->SetExtent(0, canvasWidth, 0, canvasHeight, 0, 0);
	m_backgroundCanvas->SetNumberOfScalarComponents(3);

	vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();   /* Canvas will be drawn into the texture of a plane. */
	plane->SetCenter(0.0, 0.0, 0.0);
	plane->SetNormal(0.0, 0.0, 1.0);
	plane->SetPoint1( canvasWidth, 0, 0 );
	plane->SetPoint2( 0, canvasHeight, 0 );
 
	vtkSmartPointer<vtkTexture> canvasTexture = vtkSmartPointer<vtkTexture>::New();  /* canvasTexture where the canvas is drawn */
	canvasTexture->SetInputConnection( m_backgroundCanvas->GetOutputPort() );

	vtkSmartPointer<vtkPolyDataMapper2D> canvasMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
	canvasMapper->SetInputConnection( plane->GetOutputPort() );

	vtkSmartPointer<vtkTexturedActor2D> canvasActor = vtkSmartPointer<vtkTexturedActor2D>::New();   /* Use a Textured Actor2D to combine the created plane with our "canvas canvasTexture". */
	canvasActor->SetMapper( canvasMapper );
	canvasActor->SetTexture( canvasTexture );
	m_kinectVTKRenderer->AddActor2D( canvasActor );

	// Create user's hand cursors
	m_actorLeftHand = this->createUserHandCursor();
	m_actorRightHand = this->createUserHandCursor();

	m_actorLeftHand->GetProperty()->SetColor( 1, 1, 0 );
	m_actorRightHand->GetProperty()->SetColor( 0, 1, 1 );

	m_kinectVTKRenderer->AddActor2D( m_actorLeftHand );
	m_kinectVTKRenderer->AddActor2D( m_actorRightHand );

	// Instantiate the initial menu
	m_menu.reset( new CKinectMenuSelectTool( m_kinectVTKRenderer ) );

	// Initialize updating timer
	m_updateTimer.reset( new wxTimer( this, KINECT_RENDERER_TIMER_ID ) );
	m_updateTimer->Start( KINECT_RENDERER_TIMER_PERIOD );
}


/** Destructor. */
CKinectRenderer::~CKinectRenderer()
{
	delete [] m_pDepthData;
	m_pKinectVTKWindow->SetInteractorStyle( NULL );
}


/** Copies Kinect's depth data to a buffer for rendering on the Kinect Renderer Panel during the next Paint event.
 * @param imgWidth The width of the depth image.
 * @param imgHeight The height of the depth image.
 * @param depthData An array containing the depth data. */
void CKinectRenderer::copyDepthDataToBuffer( int imgWidth, int imgHeight, const void * const depthData )
{
	// Retrieve the size of the panel being painted, initialize default drawing colors and variables
	const int devWidth = 320,   // Not the way we should do it..... oh well.
		devHeight = 240;

	const double playerColors[6][4] = {
		{ 255, 0, 0, 255 },
		{ 0, 255, 0, 255 },
		{ 0, 0, 255, 255 },
		{ 255, 255, 0, 255 },
		{ 255, 0, 255, 255 },
		{ 0, 255, 255, 255 },
	};

	// Clear the background canvas to black
	double blackColor[4] = { 0, 0, 0, 255 };
	double leftHandColor[4] = { 0, 255, 255, 255 };
	double rightHandColor[4] = { 255, 255, 0, 255 };

	m_backgroundCanvas->SetDrawColor( blackColor );
	m_backgroundCanvas->FillBox( 0, devWidth, 0, devHeight );

	// Draw per-pixel data
	const USHORT * depthDataIt = static_cast<const USHORT *>( depthData );
	if ( depthData != NULL )
	{
		for ( int row = 0; row < devHeight; row++ )
		{
			// Render depth map (+ colored identified users)
			for ( int col = 0; col < devWidth; col++, depthDataIt++ )
			{
				// Retrieve depth data
				USHORT packedDepthData = *depthDataIt;
				USHORT realDepth = min( NuiDepthPixelToDepth( packedDepthData ), 5000 );
				USHORT playerIndex = NuiDepthPixelToPlayerIndex( packedDepthData );

				// Render!
				if ( playerIndex != 0 && m_renderingType != evRenderingTypeHandsOnly )
					m_backgroundCanvas->SetDrawColor( playerColors[playerIndex-1][0], playerColors[playerIndex-1][1], playerColors[playerIndex-1][2], playerColors[playerIndex-1][3] );
				else if ( playerIndex == 0 )
				{
					if ( m_renderingType == evRenderingTypeFull )
					{
						double colorBase = static_cast<double>( (realDepth / 5000.0) * 255.0 );
						m_backgroundCanvas->SetDrawColor( colorBase, colorBase, colorBase, 255 );
					}
				}

				// Draw pixel
				m_backgroundCanvas->DrawPoint( col, devHeight - row );
			}
		}
	}

	// Render skeleton data (player hands)
	CApp &app = wxGetApp();
	if ( app.getCurrentInputDeviceId() == evInputDeviceKinect )
	{
		CInputDeviceProcessor &curInputDeviceProcessor = app.getCurrentInputDeviceProcessor();

#ifdef DEBUG
		CKinectInputProcessor &kinectProcessor = dynamic_cast<CKinectInputProcessor &>( curInputDeviceProcessor );
#else
		CKinectInputProcessor &kinectProcessor = static_cast<CKinectInputProcessor &>( curInputDeviceProcessor );
#endif
		std::pair<int, int> leftHandPos, rightHandPos;
		kinectProcessor.getHandPosition( true, leftHandPos.first, leftHandPos.second );
		kinectProcessor.getHandPosition( false, rightHandPos.first, rightHandPos.second );

		const int handCursorRadius = kinectProcessor.getHandCursorRadius();

		if ( leftHandPos.first != CKinectInputProcessor::INVALID_HAND_POSITION && leftHandPos.second != CKinectInputProcessor::INVALID_HAND_POSITION )
		{
			m_backgroundCanvas->SetDrawColor( leftHandColor[0], leftHandColor[1], leftHandColor[2], leftHandColor[3] );
			m_backgroundCanvas->DrawCircle( leftHandPos.first, leftHandPos.second, handCursorRadius );
		}
		if ( rightHandPos.first != CKinectInputProcessor::INVALID_HAND_POSITION && rightHandPos.second != CKinectInputProcessor::INVALID_HAND_POSITION )
		{
			m_backgroundCanvas->SetDrawColor( rightHandColor[0], rightHandColor[1], rightHandColor[2], rightHandColor[3] );
			m_backgroundCanvas->DrawCircle( rightHandPos.first, rightHandPos.second, handCursorRadius );
		}
	}
}


/** Retrieves the currently set rendering type of the Kinect Renderer.
 * @return Returns the rendering type of the Kinect Renderer, as provided by
 *    the #CKinectRenderer::ERenderingType enumeration. */
CKinectRenderer::ERenderingType CKinectRenderer::getRenderingType( void ) const
{
	return m_renderingType;
}


/** Modifies the way the Kinect Renderer should render data to the user.
 * This may affect the performance of the application, depending on the amount of details each
 * Rendering Type provides.
 * @param renderingType The type of rendering for the Kinect Renderer. */
void CKinectRenderer::setRenderingType( CKinectRenderer::ERenderingType renderingType )
{
	m_renderingType = renderingType;
}


/** Modifies the menu used by the Kinect renderer.
 * @param menu The new menu to be used by the Kinect renderer. */
void CKinectRenderer::setKinectMenu( CKinectRenderer::TKinectMenuSharedPtr menu )
{
	m_menu = menu;
}


/** Retrieves the menu currently in use by the Kinect renderer. 
 * @return Returns a shared pointer to the current Kinect Renderer Menu. */
CKinectRenderer::TKinectMenuSharedPtr CKinectRenderer::getKinectMenu( void ) const
{
	return m_menu;
}


/** This method is called by a timer to process the current Kinect Menu events and redraw it periodically. */
void CKinectRenderer::OnRenderAndProcessingTimer( wxTimerEvent & )
{
	// Process menu
	if ( m_menu.get() != NULL )
	{
		TKinectMenuSharedPtr curMenu = m_menu;

		curMenu->updateMenuState();
		curMenu->processMenuItems();
	}

	// Render Kinect Panel
	m_pKinectVTKWindow->Render();
}


/** Called to process mouse movement events into the Renderer window. */
void CKinectRenderer::OnMouseMove( void )
{
	this->onMouseEvent();
}


/** Called to process mouse's left button up/down events into the Renderer window. */
void CKinectRenderer::OnLeftButtonDown( void )
{
	m_isMouseLeftDown = true;
	this->onMouseEvent();
}


/** Called to process mouse's right button up/down events into the Renderer window. */
void CKinectRenderer::OnRightButtonDown( void )
{
	m_isMouseRightDown = true;
	this->onMouseEvent();
}




/** Called to process mouse's left button up/down events into the Renderer window. */
void CKinectRenderer::OnLeftButtonUp( void )
{
	m_isMouseLeftDown = false;
	this->onMouseEvent();
}


/** Called to process mouse's right button up/down events into the Renderer window. */
void CKinectRenderer::OnRightButtonUp( void )
{
	m_isMouseRightDown = false;
	this->onMouseEvent();
}


/** A generic method used to process the mouse events received by the renderer. */
void CKinectRenderer::onMouseEvent()
{
	// Only process if Kinect Simulation Mode is enabled, and if there is a Kinect Processor enabled.
	CApp &app = wxGetApp();
	if ( app.isKinectSimulationModeEnabled() == false )
		return;

	if ( app.getCurrentInputDeviceId() != evInputDeviceKinect )
		return;

	// Get mouse position
	vtkRenderWindowInteractor *iren = this->GetInteractor();
	int evtX, evtY;
	iren->GetEventPosition( evtX, evtY );

	// Retrieve the Kinect Input Processor
	CInputDeviceProcessor &curInputDeviceProcessor = app.getCurrentInputDeviceProcessor();
#ifdef DEBUG
	CKinectInputProcessor &kinectProcessor = dynamic_cast<CKinectInputProcessor &>( curInputDeviceProcessor );
#else
	CKinectInputProcessor &kinectProcessor = static_cast<CKinectInputProcessor &>( curInputDeviceProcessor );
#endif

	// Left button controls left hand, right button controls the right hand.
	if ( m_isMouseLeftDown )
	{
		kinectProcessor.updateHandDepthMapPosition( true, evtX, evtY );
		m_actorLeftHand->SetPosition( evtX, evtY );
	}

	if ( m_isMouseRightDown )
	{
		kinectProcessor.updateHandDepthMapPosition( false, evtX, evtY );
		m_actorRightHand->SetPosition( evtX, evtY );
	}
}


/** Utility method to create the user's hand cursors, which are represented by 2D circles.
 * @return Returns a smart pointer to a VTK's Actor2D object which represents the created 2D circle. */
vtkSmartPointer<vtkActor2D> CKinectRenderer::createUserHandCursor( void )
{
	// Creates a 2D "circunference"
	vtkSmartPointer<vtkRegularPolygonSource> polySrc = vtkSmartPointer<vtkRegularPolygonSource>::New();
	polySrc->SetRadius( 10 );
	polySrc->SetNumberOfSides(15);
	polySrc->GeneratePolygonOn();
	polySrc->GeneratePolylineOff();

	// Use a 2D mapper for the circunference
	vtkPolyData *polyData = polySrc->GetOutput();

	vtkSmartPointer<vtkPolyDataMapper2D> polyMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
	polyMapper->SetInput( polyData );

	// Configure an Actor2D to use the circunference and return it
	vtkSmartPointer<vtkActor2D> createdActor = vtkSmartPointer<vtkActor2D>::New();
	createdActor->SetMapper( polyMapper );
	return createdActor;
}

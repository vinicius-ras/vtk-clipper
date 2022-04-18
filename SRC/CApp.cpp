#include <windows.h>
#include <NuiApi.h>

#include "CApp.hpp"
#include "CMainFrame.hpp"
#include "CVolumeClipperAppInteractorStyle.hpp"
#include "inputDevices.hpp"

#include "CKinectInputProcessor.hpp"

#include <wx/image.h>
#include <wx/xrc/xmlres.h>
#include <wx/msgdlg.h>
#include <algorithm>

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include "testes.hpp"
#include "CAppConsole.hpp"


IMPLEMENT_APP(CApp);

// CONSTANTS
/** An identifier for the timer that is responsible for updating the state of the currently selected Input Device. */
static const int INPUT_DEVICE_UPDATING_TIMER_ID = 1234;
/** The interval for updating the currently selected Input Device. */
static const int INPUT_DEVICE_UPDATING_TIMER_INTERVAL_MS = 1000 / APPLICATION_DESIRED_FPS;
/** The maximum time to wait for receiving a new Depth Stream's Frame from Kinect. */
static const DWORD KINECT_DEPTH_STREAM_WAIT_TIME = 100;


// FILE GLOBALS
static wxString RESOURCES_XRC_FILE( _T("MEDIA/vtkclippergui.xrc") );
static std::auto_ptr<wiimote> WIIMOTE_INSTANCE;

static INuiSensor *KINECT_INSTANCE;
static HANDLE KINECT_DEPTH_STREAM_HANDLE;


// EVENT TABLE (wxWidgets)
BEGIN_EVENT_TABLE( CApp, wxApp )
	EVT_TIMER( INPUT_DEVICE_UPDATING_TIMER_ID, OnInputDeviceUpdateTimer )
END_EVENT_TABLE()


// METHODS
/** The method which is called when the application starts.
 * @return Returns true if the application was successfully initialized, or false otherwise.
 */
bool CApp::OnInit()
{
	// Initialize properties
	m_currentInputDeviceId = evInputDeviceNone;

	m_bSimulateKinect = false;
	m_bKinectSmoothMovement = true;
	m_isSimulatedKinectConnected = false;

	// Initialize image handlers
	wxInitAllImageHandlers();

	// Initialize XRC resources handler
	wxXmlResource &resources = *wxXmlResource::Get();
	resources.InitAllHandlers();
	resources.Load( RESOURCES_XRC_FILE );

	// Creates and displays the main application's frame
	const wxSize DEFAULT_MAIN_FRAME_SIZE(800, 600);

	m_mainFrame = new CMainFrame();
	wxSize mainFrameSize = m_mainFrame->GetSize();
	if ( mainFrameSize.GetWidth() < DEFAULT_MAIN_FRAME_SIZE.GetWidth() ||
		mainFrameSize.GetHeight() < DEFAULT_MAIN_FRAME_SIZE.GetHeight() )
		m_mainFrame->SetSize( DEFAULT_MAIN_FRAME_SIZE );
	m_mainFrame->Show();

	// Initialize WiiMote updating timer
	m_wiimoteUpdateTimer.reset( new wxTimer( this, INPUT_DEVICE_UPDATING_TIMER_ID ) );
	m_wiimoteUpdateTimer->Start( INPUT_DEVICE_UPDATING_TIMER_INTERVAL_MS );

	m_lockWiiMoteUpdate = false;   

    return true;
}


/** Retrieves a reference to the application's Main Frame.
 * @return Returns the reference to the application's Main Frame.
 */
CMainFrame &CApp::getMainFrame()
{
	return *m_mainFrame;
}


/** Destroys the timer used to update the WiiMote state. */
void CApp::destroyUpdateTimer( void )
{
	m_wiimoteUpdateTimer.reset();
}


/** Retreives a reference to the wiimote object that represents the WiiMote and its state.
 * @return Returns the reference to the WiiMote being used by the application.
 */
wiimote &CApp::getWiiMote()
{
	if (WIIMOTE_INSTANCE.get() == NULL)
	{
		WIIMOTE_INSTANCE.reset( new wiimote() );
		WIIMOTE_INSTANCE->ChangedCallback = &CApp::OnWiiMoteStateChanged;
		WIIMOTE_INSTANCE->CallbackTriggerFlags = static_cast<state_change_flags>( CONNECTED | CONNECTION_LOST | MOTIONPLUS_CHANGED );
	}
	return *WIIMOTE_INSTANCE;
}


/** Destroys the current instance of the wiimote object used by the application, if any. */
void CApp::destroyCurrentWiiMoteInstance( void )
{
	bool objectExists = (WIIMOTE_INSTANCE.get() != NULL);
	if ( objectExists && WIIMOTE_INSTANCE->IsConnected() )
		WIIMOTE_INSTANCE->Disconnect();
	WIIMOTE_INSTANCE.reset();

	if ( objectExists )
		warnListenersAboutInputDeviceConnection( false );
}


/** Creates a Kinect sensor handler object.
 * @return Returns a flag indicating the success of the operation. */
bool CApp::createKinect( void )
{
	// Cannot create a Kinect instance if there is already an instance created
	assert(
		(m_bSimulateKinect == false && KINECT_INSTANCE == NULL) ||   /* condition for when Simulated Kinect Mode is disabled. */
		(m_bSimulateKinect == true && m_isSimulatedKinectConnected == false)          /* condition for Simulated Kinect Mode. */
	);

	bool kinectCreationSuccess = false;
	if ( m_bSimulateKinect == false )
	{
		// NOT IN "SIMULATED KINECT MODE"...
		// Connect to the firstly available Kinect sensor
		HRESULT opResult;

		opResult = NuiCreateSensorByIndex( 0, &KINECT_INSTANCE );
		kinectCreationSuccess = ( opResult == S_OK );

		// Kinect initialization sequence
		if ( kinectCreationSuccess )
		{
			// Initialize Kinect
			DWORD initFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON
				|  NUI_INITIALIZE_FLAG_USES_COLOR;

			opResult = KINECT_INSTANCE->NuiInitialize( initFlags );
			if ( opResult != S_OK )
			{
				destroyCurrentKinectInstance();
				return false;
			}

			// Enable Kinect's skeletal engine
			if ( HasSkeletalEngine( KINECT_INSTANCE ) == false
				|| KINECT_INSTANCE->NuiSkeletonTrackingEnable( NULL, 0 ) != S_OK )
			{
				destroyCurrentKinectInstance();
				return false;
			}

			// Open a Depth image stream from Kinect
			opResult = KINECT_INSTANCE->NuiImageStreamOpen( NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_320x240,
				NULL, 2, NULL, &KINECT_DEPTH_STREAM_HANDLE );

			if ( opResult != S_OK )
			{
				destroyCurrentKinectInstance();
				return false;
			}

		}
	}
	else
	{
		// IN "SIMULATED KINECT MODE"...
		// Simulated Kinect creation always succeeds
		kinectCreationSuccess = true;
		m_isSimulatedKinectConnected = true;
	}

	// If successfull, update application's input device
	if ( kinectCreationSuccess )
	{
		CApp &app = wxGetApp();

		app.setCurrentInputDeviceId( evInputDeviceKinect );
		app.warnListenersAboutInputDeviceConnection( true );
	}

	// Return result
	return kinectCreationSuccess;
}


/** Retreives a reference to the object that represents the Kinect sensor.
 * @return Returns the reference to the Kinect sensor being used by the application.
 */
INuiSensor &CApp::getKinect()
{
	// This method only works when Kinect Simulation Mode is disabled.
	assert( m_bSimulateKinect == false );

	// Kinect must have already been created, via CApp::createKinect()
	assert(KINECT_INSTANCE != NULL);
	return *KINECT_INSTANCE;
}


/** Destroys the current instance of the Kinect sensor by the application, if any. */
void CApp::destroyCurrentKinectInstance( void )
{
	bool objectExists;
	if ( m_bSimulateKinect == false )
	{
		// IF WE'RE NOT IN "KINECT SIMULATION MODE"...
		objectExists = (KINECT_INSTANCE != NULL);
		if ( objectExists )
			KINECT_INSTANCE->NuiShutdown();

		KINECT_INSTANCE = NULL;
	}
	else
	{
		// IN "KINECT SIMULATION MODE"...
		objectExists = m_isSimulatedKinectConnected;
		m_isSimulatedKinectConnected = false;
	}

	if ( objectExists )
		warnListenersAboutInputDeviceConnection( false );
}


/** Verifies if the application has connected to a Kinect sensor.
 * @return Returns a flag indicating if the Kinect sensor has been connected. */
bool CApp::isKinectConnected( void ) const
{
	// Simulated Kinect only returns our virtual Kinect Sensor state, which is kept into a flag
	if ( m_bSimulateKinect )
		return m_isSimulatedKinectConnected;

	// If we're not into "Simulate Kinect Mode", verify if we really have a physical Kinect Sensor connected to our app
	return (KINECT_INSTANCE != NULL);
}


/** Retrieves the identifier of the input device the application is currently using.
 * @return Returns the identifier of the input device, as defined in the #EInputDevices enumeration. */
EInputDevices CApp::getCurrentInputDeviceId( void ) const
{
	return static_cast<EInputDevices>( m_currentInputDeviceId );
}


/** Modifies the identifier of the input device the application is currently using, after the device has been connected.
 * @param deviceId The new identifier for the input device which is being used by the application. */
void CApp::setCurrentInputDeviceId( EInputDevices deviceId )
{
	m_currentInputDeviceId = static_cast<int>( deviceId );
}



/** Retrieves the current Input Device Processor which is being used to interact with user's input, through special
 * devices (WiiMote, Kinect).
 * @return Returns a reference to the current Input Device Processor. */
CInputDeviceProcessor &CApp::getCurrentInputDeviceProcessor( void ) const
{
	return m_mainFrame->getInteractorStyle()->getInputDeviceProcessor();
}



/** Adds a listener to be called when the WiiMote state changes.
 * @param listener The listener to be added.
 */
void CApp::addWiiMoteUpdateListener( CInputDeviceEventsListener *listener )
{
	m_wiimoteUpdateListeners.push_back(listener);
}


/** Removes a listener from the list of listeners to be called when the WiiMote state changes.
 * @param listener The listener to be removed.
 */
bool CApp::removeWiiMoteUpdateListener( CInputDeviceEventsListener *listener )
{
	const size_t totalListeners = m_wiimoteUpdateListeners.size();
	for ( size_t l = 0; l < totalListeners; l++ )
	{
		if ( m_wiimoteUpdateListeners[l] == listener )
		{
			m_wiimoteUpdateListeners.erase( m_wiimoteUpdateListeners.begin() + l );
			return true;
		}
	}
	return false;
}


/** Pauses or unpauses the WiiMote updating timer.
 * @param bPause A flag indicating if the WiiMote updating should be paused or unpaused.
 */
void CApp::pauseWiiMoteUpdateTimer( bool bPause )
{
	if ( bPause )
	{
		m_wiimoteUpdateTimer->Stop();
		m_lockWiiMoteUpdate = true;
	}
	else
	{
		m_wiimoteUpdateTimer->Start();
		m_lockWiiMoteUpdate = false;
	}
}


/** Verifies if the WiiMote updating timer is currently running.
 * @return Returns a flag indicating if the timer is running.
 */
bool CApp::isWiiMoteUpdateTimerRunning( void ) const
{
	return m_wiimoteUpdateTimer->IsRunning();
}


/** Verifies if the Kinect Simulation Mode is enabled.
 * @return Returns a flag indicating if Kinect Simulation Mode is enabled. */
bool CApp::isKinectSimulationModeEnabled( void ) const
{
	return m_bSimulateKinect;
}


/** Enables or disables Kinect Simulation Mode. This mode is used for debugging purposes, allowing us to control
 * the user's Hand Positions into the Kinect Depth Map by using a combination of clicks and mouse dragging, without the
 * need of a physical Kinect sensor to be enabled for the application.
 * @param bEnable A flag indicating if Kinect simulation mode should be enabled or disabled. */
void CApp::setKinectSimulationMode( bool bEnable )
{
	m_bSimulateKinect = bEnable;

	CAppConsole &console = CAppConsole::getInstance();
	console.log( wxString::Format( _T( "Kinect Simulation Mode: %s" ), bEnable ? _T( "enabled" ) : _T( "disabled" ) ) );
}


/** Verifies if the Kinect's Movement Smoothing feature is enabled.
 * @return Returns a flag indicating if Kinect's tracked joints' movement are being smoothed. */
bool CApp::isKinectSmoothMovementEnabled( void ) const
{
	return m_bKinectSmoothMovement;
}


/** Enables or disables Kinect's joints movement smoothing feature.
 * @param bEnable A flag indicating if the smoothing should be activated or not. */
void CApp::setKinectSmoothMovement( bool bEnable )
{
	m_bKinectSmoothMovement = bEnable;

	CAppConsole &console = CAppConsole::getInstance();
	console.log( wxString::Format( _T( "Kinect Smoothing: %s" ), bEnable ? _T( "enabled" ) : _T( "disabled" ) ) );
}


/** Calls every registered WiiMote Update Listener, informing that the WiiMote has been disconnected.
 * @param bConnected A flag indicating if the input device has been connected or disconnected. */
void CApp::warnListenersAboutInputDeviceConnection( bool bConnected )
{
	const size_t totalListeners = m_wiimoteUpdateListeners.size();
	for ( size_t l = 0; l < totalListeners; l++ )
		m_wiimoteUpdateListeners[l]->OnInputDeviceConnectionChanged( bConnected );
}


void CApp::warnListenersAboutInputDeviceStateUpdated()
{
	// Call the Input Device's state listeners
	const size_t totalListeners = m_wiimoteUpdateListeners.size();
	for ( size_t l = 0; l < totalListeners; l++ )
	{
		assert(m_lockWiiMoteUpdate == false);
		m_wiimoteUpdateListeners[l]->OnInputDeviceStateUpdated();
	}
}



/** Called by a timer to update the current Input Device's state. */
void CApp::OnInputDeviceUpdateTimer( wxTimerEvent & )
{
	switch ( this->getCurrentInputDeviceId() )
	{
	case evInputDeviceWiiMote:
		if ( m_lockWiiMoteUpdate )
			return;

		// Update WiiMote state
		if ( WIIMOTE_INSTANCE.get() != NULL && WIIMOTE_INSTANCE->IsConnected() )
		{
			WIIMOTE_INSTANCE->RefreshState();

			// Call the Input Device's state listeners
			warnListenersAboutInputDeviceStateUpdated();
		}
		break;
	case evInputDeviceKinect:
		{
			bool kinectStateUpdated = false;

			int depthMapWidth = -1, depthMapHeight = -1;
			BYTE *depthData = NULL;

			if ( m_bSimulateKinect == false )
			{
				// IF WE'RE NOT IN "KINECT SIMULATED MODE"...
				HRESULT opResult;
				NUI_IMAGE_FRAME imgFrame;
				NUI_SKELETON_FRAME skeletonFrame;

				// Is there a Depth frame ready for us to process?
				/* After getting a depth frame, asserts the result is one of the expected results, not an error.
				 * E_NUI_FRAME_NO_DATA is acceptable, and indicates the frame is not ready for processing yet.
				 */
				opResult = KINECT_INSTANCE->NuiImageStreamGetNextFrame( KINECT_DEPTH_STREAM_HANDLE, INFINITE, &imgFrame );
				assert( SUCCEEDED( opResult ) || opResult == E_NUI_FRAME_NO_DATA );

				// A frame from Kinect has been acquired successfully, and is ready for processing
				if ( opResult == S_OK )
				{
					// Is there a Skeleton frame ready for us to process?
					opResult = KINECT_INSTANCE->NuiSkeletonGetNextFrame( INFINITE, &skeletonFrame );
					assert( SUCCEEDED( opResult ) || opResult == E_NUI_FRAME_NO_DATA );

					if ( opResult == S_OK )
					{
						// If user has enabled the Kinect Smoothing feature, smooth tracked joints' movements
						if ( m_bKinectSmoothMovement )
							KINECT_INSTANCE->NuiTransformSmooth( &skeletonFrame, NULL );

						// Get the Kinect Input Processor
						CInputDeviceProcessor &curInputDeviceProcessor = getCurrentInputDeviceProcessor();

#ifdef DEBUG
						CKinectInputProcessor &kinectInputProcessor = dynamic_cast<CKinectInputProcessor &>( curInputDeviceProcessor );
#else
						CKinectInputProcessor &kinectInputProcessor = static_cast<CKinectInputProcessor &>( curInputDeviceProcessor );
#endif

						// Retrieve the position of the users
						for ( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
						{
							NUI_SKELETON_DATA &curSkeletonData = skeletonFrame.SkeletonData[i];
							if( curSkeletonData.eTrackingState == NUI_SKELETON_TRACKED )
							{
								// Update LEFT and RIGHT hand positions for the Kinect's input processor
								FLOAT fPosX, fPosY;
								if ( curSkeletonData.eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_LEFT] != NUI_SKELETON_POSITION_NOT_TRACKED )
								{
									NuiTransformSkeletonToDepthImage( curSkeletonData.SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT], &fPosX, &fPosY );
									kinectInputProcessor.updateHandDepthMapPosition( true, static_cast<int>( fPosX ), static_cast<int>( fPosY ) );
								}
								else
									kinectInputProcessor.unsetHandDepthMapPosition( true );

								if ( curSkeletonData.eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_RIGHT] != NUI_SKELETON_POSITION_NOT_TRACKED )
								{
									NuiTransformSkeletonToDepthImage( curSkeletonData.SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT], &fPosX, &fPosY );
									kinectInputProcessor.updateHandDepthMapPosition( false, static_cast<int>( fPosX ), static_cast<int>( fPosY ) );
								}
								else
									kinectInputProcessor.unsetHandDepthMapPosition( false );

								break;
							}
						}

						// The Kinect's state has been successfully updated
						kinectStateUpdated = true;

						// Send depth data to be drawn by the Kinect renderer
						INuiFrameTexture * pTexture = imgFrame.pFrameTexture;
						NUI_LOCKED_RECT lockedRect;
						pTexture->LockRect( 0, &lockedRect, NULL, 0 );

						if ( lockedRect.Pitch != 0 )
						{
							DWORD imgWidth, imgHeight;
							NuiImageResolutionToSize( imgFrame.eResolution, imgWidth, imgHeight );

							depthMapWidth = static_cast<int>( imgWidth );
							depthMapHeight = static_cast<int>( imgHeight );
							depthData = lockedRect.pBits;
						}

						// Releases the acquired frame
						KINECT_INSTANCE->NuiImageStreamReleaseFrame( KINECT_DEPTH_STREAM_HANDLE, &imgFrame );
					}
				}
			}
			else
			{
				// IF WE'RE IN "KINECT SIMULATED MODE"...
				// Kinect State is always marked as Updated.
				kinectStateUpdated = true;
			}

			// Call the Input Device's state listeners
			if ( kinectStateUpdated )
			{
				// NOTE: In "Kinect Simulated Mode", depthData = NULL at this point
				m_mainFrame->refreshKinectRenderPanel( depthMapWidth, depthMapHeight, depthData );
				warnListenersAboutInputDeviceStateUpdated();
			}
		}
		break;
	}
}


/** Called when the WiiMote state gets changed. This is called in some events which are not related to updating
 * the accelerometer data, like the stablishment of a connection or the loss of a connection to the WiiMote or the
 * user plugging or unplugging extensions to the WiiMote. */
void CApp::OnWiiMoteStateChanged(wiimote &remote, state_change_flags changed, const wiimote_state &new_state)
{
	// Verifies which states have changed
	bool bWiiMoteHasConnected = ((changed & CONNECTED) == CONNECTED);
	bool bWiiMoteHasDisconnected = ((changed & CONNECTION_LOST) == CONNECTION_LOST);
	bool bWiiMotionPlusDetected = ((changed & MOTIONPLUS_DETECTED) == MOTIONPLUS_DETECTED);

	if ( bWiiMotionPlusDetected && remote.MotionPlusEnabled() == false )
	{
		bool enableResult = remote.EnableMotionPlus();
		assert( enableResult );
	}

	if ( bWiiMoteHasConnected == false && bWiiMoteHasDisconnected == false )
		return;

	// Update application's device id
	CApp &app = wxGetApp();
	if ( bWiiMoteHasConnected )
		app.setCurrentInputDeviceId( evInputDeviceWiiMote );

	// Calls the listeners
	app.warnListenersAboutInputDeviceConnection( bWiiMoteHasConnected );
}

#include "CWiiMoteInputProcessor.hpp"
#include "CApp.hpp"
#include "CMainFrame.hpp"
#include "CVolumeClipperAppInteractorStyle.hpp"
#include "CClippingAlgorithm.hpp"
#include "CEraser3D.hpp"
#include "CClipper3D.hpp"

#include "clippingAlgorithms.hpp"

#include <vtkMatrix4x4.h>
#include <vtkCamera.h>
#include <vtkTransform.h>
#include <vtkRenderWindow.h>

#include <set>

// FILE CONSTANTS
static const float MOTION_PLUS_MINIMUM_YAW_SPEED_THRESHOLD = 0.075f;
static const float MOTION_PLUS_MINIMUM_PITCH_SPEED_THRESHOLD = 0.075f;
static const float MOTION_PLUS_MINIMUM_ROLL_SPEED_THRESHOLD = 0.075f;
static const float MOTION_PLUS_CORRECTION_FACTOR = 0.05f;

/** Control state for the CEraser3D algorithm: user is rotating the cylinder (cursor). */
static const std::string CONTROL_STATES_ERASER3D_CYLINDER_ROTATING = std::string("CylinderRotating");
/** Control state for the CEraser3D algorithm: user is translating the cylinder (cursor). */
static const std::string CONTROL_STATES_ERASER3D_CYLINDER_TRANSLATING = std::string("CylinderTranslating");

/** Control state for the CClipper3D algorithm: user is rotating the plane (cursor). */
static const std::string CONTROL_STATES_CLIPPER3D_PLANE_ROTATING = std::string("PlaneRotating");
/** Control state for the CClipper3D algorithm: user is translating the plane (cursor). */
static const std::string CONTROL_STATES_CLIPPER3D_PLANE_TRANSLATING = std::string("PlaneTranslating");


// FILE TYPEDEFS
/** A vector of strings which represent the states of the clipping algorithms. */
typedef std::vector<const std::string> TControlStatesVector;


// GLOBAL FILE VARIABLES
/** This vector's elements represent the actions the user should perform before executing the CEraser3D algorithm. */
static TControlStatesVector CONTROL_STATES_ERASER3D;
/** This vector's elements represent the actions the user should perform before executing the CClipper3D algorithm. */
static TControlStatesVector CONTROL_STATES_CLIPPER3D;
/** A set containing the mask of the buttons that were pressed on the last updating of the WiiMote state. */
static std::set<int> WIIMOTE_PRESSED_BUTTON_MASKS;


// METHODS
/** Constructor. */
CWiiMoteInputProcessor::CWiiMoteInputProcessor( void )
{
	// Initializes the vector of actions of some algorithms
	if ( CONTROL_STATES_ERASER3D.empty() )
	{
		CONTROL_STATES_ERASER3D.push_back( CONTROL_STATES_ERASER3D_CYLINDER_ROTATING );
		CONTROL_STATES_ERASER3D.push_back( CONTROL_STATES_ERASER3D_CYLINDER_TRANSLATING );

		CONTROL_STATES_CLIPPER3D.push_back( CONTROL_STATES_CLIPPER3D_PLANE_ROTATING );
		CONTROL_STATES_CLIPPER3D.push_back( CONTROL_STATES_CLIPPER3D_PLANE_TRANSLATING );
	}
}


/** Called when the user selects a new Clipping Algorithm.
 * @param clippingAlgorithmId The identifier for the newly selected clipping algorithm. */
void CWiiMoteInputProcessor::OnClippingAlgorithmChanged( int clippingAlgorithmId )
{
	switch ( clippingAlgorithmId )
	{
	case evAlgorithmEraser3D:
		m_curControlState = CONTROL_STATES_ERASER3D[0];
		break;
	case evAlgorithmClipper3D:
		m_curControlState = CONTROL_STATES_CLIPPER3D[0];
		break;
	}
}


/** Called when the WiiMote state is refreshed. */
void CWiiMoteInputProcessor::OnInputDeviceStateUpdated( void )
{
	// Retrieve WiiMote reference
	CApp &app = wxGetApp();
	wiimote &remote = app.getWiiMote();

    // Retrieve data about the Wii Motion Plus accessory.
    // This data is only to be considered valid if bEnableMotionPlus is true.
	CMainFrame &mainFrame = app.getMainFrame();
    const bool bEnableMotionPlus = (remote.MotionPlusConnected() && remote.MotionPlusEnabled() && mainFrame.isWiiMotionPlusChecked());

    const float motionPlusYawSpeed = remote.MotionPlus.Speed.Yaw * MOTION_PLUS_CORRECTION_FACTOR;
    const float motionPlusPitchSpeed = remote.MotionPlus.Speed.Pitch * MOTION_PLUS_CORRECTION_FACTOR;
    const float motionPlusRollSpeed = remote.MotionPlus.Speed.Roll * MOTION_PLUS_CORRECTION_FACTOR;

    const float motionPlusYawDelta = m_lastMotionPlusYaw - motionPlusYawSpeed;
    const float motionPlusPitchDelta = m_lastMotionPlusPitch - motionPlusPitchSpeed;
    const float motionPlusRollDelta = m_lastMotionPlusRoll - motionPlusRollSpeed;

    const bool isMotionPlusYawSpeedValid = (fabs(motionPlusYawDelta) > MOTION_PLUS_MINIMUM_YAW_SPEED_THRESHOLD );
    const bool isMotionPlusPitchSpeedValid = (fabs(motionPlusPitchDelta) > MOTION_PLUS_MINIMUM_PITCH_SPEED_THRESHOLD );
    const bool isMotionPlusRollSpeedValid = (fabs(motionPlusRollDelta) > MOTION_PLUS_MINIMUM_ROLL_SPEED_THRESHOLD );

	vtkSmartPointer<vtkRenderer> renderer = mainFrame.getRenderer();

	// Process according to currently selected algorithm
	CVolumeClipperAppInteractorStyle &interactorStyle = *mainFrame.getInteractorStyle();
	EClippingAlgorithm curClippingAlgorithmId = static_cast<EClippingAlgorithm>( interactorStyle.getClippingAlgorithmId() );
	switch ( curClippingAlgorithmId )
	{
	case evAlgorithmEraser3D:
		{
			// Retrieve a reference to the Eraser3D algorithm and the cursor (cylinder)
			CClippingAlgorithm &curAlgorithm = interactorStyle.getClippingAlgorithm();
			CEraser3D &eraser3D = static_cast<CEraser3D &>(curAlgorithm);

			vtkProp3D *cylinderActor = static_cast<vtkProp3D *>( interactorStyle.getCursorActor(0).GetPointer() );

			// Verify which buttons were pressed
			bool btPressedPrev = remote.Button.One();
			bool btPressedNext = remote.Button.Two() || remote.Button.A();
			bool btPressedB = remote.Button.B();

			static bool BUTTONS_LOCKED = false;
			bool bDoUpdateCursor = true;

			if ( BUTTONS_LOCKED == false && (btPressedNext || btPressedPrev) )
			{
				BUTTONS_LOCKED = true;

				// Get the index of the state of the algorithm
				TControlStatesVector::const_iterator it = std::find( CONTROL_STATES_ERASER3D.begin(), CONTROL_STATES_ERASER3D.end(), m_curControlState );
				int curStateIndex = it - CONTROL_STATES_ERASER3D.begin();
				assert( curStateIndex >= 0 );
				assert( curStateIndex < static_cast<int>(CONTROL_STATES_ERASER3D.size()) );

				// Go to the next state, or apply the clipping algorithm if there are no more states left.
				if ( btPressedNext )   // if user pressed the "Two" button
				{
					if ( curStateIndex < static_cast<int>( CONTROL_STATES_ERASER3D.size() ) - 1 )
						m_curControlState = CONTROL_STATES_ERASER3D[curStateIndex + 1];
					else
					{
						// Calculate a point into the axis of the cylinder (will be used below)
						// We will use the cylinder's transformation matrix to calculate a point using the cylinder's orientation.
						const double relativePointPosP0[4] = { 0, 0, 0, 1 };
						const double relativePointPosP1[4] = { 0, 100, 0, 1 };
						double p0[4];
						double p1[4];

						cylinderActor->GetMatrix()->MultiplyPoint( relativePointPosP0, p0 );
						cylinderActor->GetMatrix()->MultiplyPoint( relativePointPosP1, p1 );

						// Define the axis of the Eraser3D cylinder.
						// The axis is a line, defined by two given points: p0 and p1.
						// We will be using p0 as the center of the cylinder, which always lies on the axis' line.
						// And p1 has been calculated above.
						CLine cylAxis;
						cylAxis.p0 = CVector3( p0[0], p0[1], p0[2] );
						cylAxis.p1 = CVector3( p1[0], p1[1], p1[2] );

						eraser3D.setCylinderAxis( cylAxis );

						// Execute the clipping algorithm
						bDoUpdateCursor = false;
						interactorStyle.startCurrentClippingAlgorithm();

						// This resets the clipping algorithm
						interactorStyle.setClippingAlgorithm( evAlgorithmNone );
						interactorStyle.setClippingAlgorithm( evAlgorithmEraser3D );
					}
				}
				else if ( curStateIndex > 0 )   // if the user has pressed the "One" button and if we can go back one state...
					m_curControlState = CONTROL_STATES_ERASER3D[curStateIndex - 1];
			}

			BUTTONS_LOCKED = (btPressedPrev || btPressedNext);

			// Only apply transformations to the cursor if the user is holding the B key
			vtkRenderWindow *renderWnd = renderer->GetRenderWindow();
			if ( btPressedB == false )
				break;

			// Keep volume's low level of detail (this should be called constantly while user is pressing B)
			interactorStyle.startRefreshRateChange();

            // Apply transformations
			if ( bDoUpdateCursor )
			{
				// Retrieve the WiiMote's Sensitivity Factor
				const float wiimoteSensitivityFactor = mainFrame.getWiiMoteSensitivity();

				// Retrieve the pitch and roll from the WiiMote
				float orientX = -remote.Acceleration.Orientation.X;
				float orientY = remote.Acceleration.Orientation.Y;

				// Update the Cylinder actor
				if ( m_curControlState == CONTROL_STATES_ERASER3D_CYLINDER_ROTATING )
				{
					// Retrieve camera's information
					vtkCamera *cam = renderer->GetActiveCamera();
					double *camOrient = cam->GetOrientation();
					double *camPos = cam->GetPosition();

					// Create a transformation to match the camera's angle
					vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
					transform->RotateY( -camOrient[1] );
					transform->RotateX( -camOrient[0] );
					transform->RotateZ( -camOrient[2] );

					// Calculate a vector for X rotation
					double relXRotVector[4] = { 1, 0, 0, 1 }, xRotVector[4];
					transform->MultiplyPoint( relXRotVector, xRotVector );

					double relZRotVector[4] = { 0, 0, 1, 1 }, zRotVector[4];
					transform->MultiplyPoint( relZRotVector, zRotVector );

					double rotX = 0;
					double rotZ = 0;

					if ( bEnableMotionPlus == false )
					{
						rotX = -remote.Acceleration.X;
						rotZ = remote.Acceleration.Y;
					}
					else
					{
						if ( isMotionPlusYawSpeedValid )
							rotZ = -motionPlusYawSpeed;
						if ( isMotionPlusPitchSpeedValid )
							rotX = motionPlusPitchSpeed;
					}

					// Apply sensitivity factor
					rotX *= wiimoteSensitivityFactor;
					rotZ *= wiimoteSensitivityFactor;

					// Rotate the cursor
					cylinderActor->RotateWXYZ( rotX, xRotVector[0], xRotVector[1], xRotVector[2] );
					cylinderActor->RotateWXYZ( rotZ, zRotVector[0], zRotVector[1], zRotVector[2] );
				}
				else if ( m_curControlState == CONTROL_STATES_ERASER3D_CYLINDER_TRANSLATING )
				{
					// Retrieve camera's information
					vtkCamera *cam = renderer->GetActiveCamera();
					double *camOrient = cam->GetOrientation();
					double *camPos = cam->GetPosition();

					// Create a transformation to match the camera's angle
					vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
					transform->RotateY( -camOrient[1] );
					transform->RotateX( -camOrient[0] );
					transform->RotateZ( -camOrient[2] );

					double translationX = 0;
					double translationY = 0;

					if ( bEnableMotionPlus == false )
                    {
					    const double TRANSLATION_SENSITIVITY = 3;
						translationX = -orientY * TRANSLATION_SENSITIVITY;
						translationY = -orientX * TRANSLATION_SENSITIVITY;
                    }
                    else
                    {
						// Use the created transformation to move the cursor into camera's "local X/Y axes"
						if ( isMotionPlusYawSpeedValid )
							translationX = motionPlusYawSpeed;
						if ( isMotionPlusPitchSpeedValid )
							translationY = motionPlusPitchSpeed;
                    }

					// Apply WiiMote's Sensitivity Factor
					translationX *= wiimoteSensitivityFactor;
					translationY *= wiimoteSensitivityFactor;

					// Move the cursor
					double relTranslation[4] = { translationX, translationY, 0, 1 }, resultPos[4];
					transform->MultiplyPoint( relTranslation, resultPos );

					cylinderActor->AddPosition( resultPos );
				}
			}

            // Update render window
            renderWnd->Render();
		}
        break;
    case evAlgorithmClipper3D:
        {
            // Retrieve a reference to the Clipper3D algorithm and the cursor (plane)
            CClippingAlgorithm &curAlgorithm = interactorStyle.getClippingAlgorithm();
            CClipper3D &clipper3D = static_cast<CClipper3D &>(curAlgorithm);

			vtkProp3D *planeActor = static_cast<vtkProp3D *>( interactorStyle.getCursorActor(0).GetPointer() );

            // Verify which buttons were pressed
            bool btPressedPrev = remote.Button.One();
            bool btPressedNext = remote.Button.Two() || remote.Button.A();
            bool btPressedB = remote.Button.B();

			static bool BUTTONS_LOCKED = false;
            bool bDoUpdateCursor = true;

            if ( BUTTONS_LOCKED == false && (btPressedNext || btPressedPrev) )
            {
                BUTTONS_LOCKED = true;

                // Get the index of the state of the algorithm
                TControlStatesVector::const_iterator it = std::find( CONTROL_STATES_CLIPPER3D.begin(), CONTROL_STATES_CLIPPER3D.end(), m_curControlState );
                int curStateIndex = it - CONTROL_STATES_CLIPPER3D.begin();
                assert( curStateIndex >= 0 );
                assert( curStateIndex < static_cast<int>(CONTROL_STATES_CLIPPER3D.size()) );

                // Go to the next state, or apply the clipping algorithm if there are no more states left.
                if ( btPressedNext )   // if user pressed the "Two" button
                {
                    if ( curStateIndex < static_cast<int>( CONTROL_STATES_ERASER3D.size() ) - 1 )
                        m_curControlState = CONTROL_STATES_CLIPPER3D[curStateIndex + 1];
                    else
                    {
                        // Calculate a point that lies in the plane and plane normal
                        const double relativePointOnPlane[4] = { 0, 0, 0, 1 },
                            relativePlaneNormal[4] = { 0, 0, 100, 1 };
                        double coordPointOnPlane[4], coordsPlaneNormal[4];

                        planeActor->GetMatrix()->MultiplyPoint( relativePointOnPlane, coordPointOnPlane );
                        planeActor->GetMatrix()->MultiplyPoint( relativePlaneNormal, coordsPlaneNormal );

                        CVector3 pointOnPlane( coordPointOnPlane[0], coordPointOnPlane[1], coordPointOnPlane[2] );
                        CVector3 planeNormal( coordsPlaneNormal[0], coordsPlaneNormal[1], coordsPlaneNormal[2] );
                        planeNormal = planeNormal - pointOnPlane;
                        planeNormal.normalize();

                        clipper3D.setConfiguration( planeNormal, pointOnPlane );


                        // Execute the clipping algorithm
                        bDoUpdateCursor = false;
						interactorStyle.startCurrentClippingAlgorithm();

                        // This resets the clipping algorithm
                        interactorStyle.setClippingAlgorithm( evAlgorithmNone );
                        interactorStyle.setClippingAlgorithm( evAlgorithmClipper3D );
                    }
                }
                else if ( curStateIndex > 0 )   // if the user has pressed the "One" button and if we can go back one state...
                    m_curControlState = CONTROL_STATES_CLIPPER3D[curStateIndex - 1];
            }

            BUTTONS_LOCKED = (btPressedPrev || btPressedNext);

            // Only apply transformations to the cursor if the user is holding the B key
            vtkRenderWindow *renderWnd = renderer->GetRenderWindow();
			if ( btPressedB == false )
				break;

			// Keep volume's low level of detail (this should be called constantly while user is pressing B)
			interactorStyle.startRefreshRateChange();

			// Apply transformations
			if ( bDoUpdateCursor )
			{
				// Retrieve the pitch and roll from the WiiMote
				float orientX = -remote.Acceleration.Orientation.X;
				float orientY = remote.Acceleration.Orientation.Y;

				// Update the Cylinder actor
				if ( m_curControlState == CONTROL_STATES_CLIPPER3D_PLANE_ROTATING )
				{
					// Retrieve camera's information
					vtkCamera *cam = renderer->GetActiveCamera();
					double *camOrient = cam->GetOrientation();
					double *camPos = cam->GetPosition();

					// Create a transformation to match the camera's angle
					vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
					transform->RotateY( -camOrient[1] );
					transform->RotateX( -camOrient[0] );
					transform->RotateZ( -camOrient[2] );

					// Calculate a vector for X rotation
					double relXRotVector[4] = { 1, 0, 0, 1 }, xRotVector[4];
					transform->MultiplyPoint( relXRotVector, xRotVector );

					double relYRotVector[4] = { 0, 1, 0, 1 }, yRotVector[4];
					transform->MultiplyPoint( relYRotVector, yRotVector );

					double rotX = 0;
					double rotY = 0;

					if ( bEnableMotionPlus == false )
					{
						rotX = -remote.Acceleration.X;
						rotY = -remote.Acceleration.Y;
					}
					else
					{
						if ( isMotionPlusYawSpeedValid )
							rotY = motionPlusYawSpeed;
						if ( isMotionPlusPitchSpeedValid )
							rotX = motionPlusPitchSpeed;
					}

					// Rotate the cursor
					planeActor->RotateWXYZ( rotX, xRotVector[0], xRotVector[1], xRotVector[2] );
					planeActor->RotateWXYZ( rotY, yRotVector[0], yRotVector[1], yRotVector[2] );
				}
				else if ( m_curControlState == CONTROL_STATES_CLIPPER3D_PLANE_TRANSLATING )
				{
					// Retrieve camera's information
					vtkCamera *cam = renderer->GetActiveCamera();
					double *camOrient = cam->GetOrientation();
					double *camPos = cam->GetPosition();

					// Create a transformation to match the camera's angle
					vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
					transform->RotateY( -camOrient[1] );
					transform->RotateX( -camOrient[0] );
					transform->RotateZ( -camOrient[2] );

					double translationX = 0;
					double translationY = 0;

					if ( bEnableMotionPlus == false )
					{
						const double TRANSLATION_SENSITIVITY = 3;
						translationX = -orientY * TRANSLATION_SENSITIVITY;
						translationY = -orientX * TRANSLATION_SENSITIVITY;
					}
					else
					{
						// Use the created transformation to move the cursor into camera's "local X/Y axes"
						if ( isMotionPlusYawSpeedValid )
							translationX = motionPlusYawSpeed;
						if ( isMotionPlusPitchSpeedValid )
							translationY = motionPlusPitchSpeed;
					}

					// Move the cursor
					double relTranslation[4] = { translationX, translationY, 0, 1 }, resultPos[4];
					transform->MultiplyPoint( relTranslation, resultPos );

					planeActor->AddPosition( resultPos );
				}

                // Update the guider line of the plane
                vtkProp3D *lineActor = static_cast<vtkProp3D *>( interactorStyle.getCursorActor(1).GetPointer() );
                double desiredRelativePos[4] = { 0, 0, 0, 1 }, worldPos[4];
                planeActor->GetMatrix()->MultiplyPoint( desiredRelativePos, worldPos );
                lineActor->SetPosition( worldPos );

                lineActor->SetOrientation( planeActor->GetOrientation() );
            }

            // Update render window
            renderWnd->Render();
        }
		break;
	}

    // Update "last speed" data
    m_lastMotionPlusYaw = motionPlusYawSpeed;
    m_lastMotionPlusPitch = motionPlusPitchSpeed;
    m_lastMotionPlusRoll = motionPlusRollSpeed;
}

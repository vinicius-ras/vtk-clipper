#include "CKinectInputProcessor.hpp"
#include <wx/wx.h>

// FILE CONSTANTS
/** Specifies the default radius for the user's hand cursor. */
static const int DEFAULT_KINECT_HAND_CURSOR_RADIUS = 10;


// METHODS
/** Constructor. */
CKinectInputProcessor::CKinectInputProcessor( void )
	: m_leftHandPosX( INVALID_HAND_POSITION ),
	m_leftHandPosY( INVALID_HAND_POSITION ),
	m_rightHandPosX( INVALID_HAND_POSITION ),
	m_rightHandPosY( INVALID_HAND_POSITION )
{
}


/** Destructor. */
CKinectInputProcessor::~CKinectInputProcessor( void )
{
}


/** Updates the position of the user's hands into the Kinect's Depth Map. This allows for the rendering of
 * user's hand cursors and processing of Kinect interaction operations.
 * @param bLeftHand A flag indicating which hand to be updated.
 * @param x The X coordinate for the user's hand position into the Depth Map.
 * @param y The Y coordinate for the user's hand position into the Depth Map. */
void CKinectInputProcessor::updateHandDepthMapPosition( bool bLeftHand, int x, int y )
{
	int &targetX = (bLeftHand ? m_leftHandPosX : m_rightHandPosX);
	int &targetY = (bLeftHand ? m_leftHandPosY : m_rightHandPosY);

	targetX = x;
	targetY = 240 - y;
}


/** This method updates the user's hands position into the Kinect's Depth Map to an invalid value, indicating
 * the user's hand has been lost (should not be rendered or processed for user's interaction with the application).
 * @param bLeftHand A flag indicating which hand has been lost. */
void CKinectInputProcessor::unsetHandDepthMapPosition( bool bLeftHand )
{
	updateHandDepthMapPosition( bLeftHand, INVALID_HAND_POSITION, INVALID_HAND_POSITION );
}


/** Retrieves the current registered position for one of the user's hands.
 * @param bLeftHand A flag indicating which of the hand's data to query.
 * @param x A reference to a variable that will receive the X coordinate of the given hand.
 *    Notice that returned value can be #CKinectInputProcessor::INVALID_HAND_POSITION.
 * @param y A reference to a variable that will receive the Y coordinate of the given hand.
 *    Notice that returned value can be #CKinectInputProcessor::INVALID_HAND_POSITION. */
void CKinectInputProcessor::getHandPosition( bool bLeftHand, int &x, int &y ) const
{
	const int &targetX = (bLeftHand ? m_leftHandPosX : m_rightHandPosX);
	const int &targetY = (bLeftHand ? m_leftHandPosY : m_rightHandPosY);

	x = targetX;
	y = targetY;
}


/** Retrieves the radius of the user's hand cursor. This is the cursor that follows users' hands and
 * allows him to touch Kinect Menu Items.
 * @return Returns the radius for the user's hand cursor. */
int CKinectInputProcessor::getHandCursorRadius( void ) const
{
	return DEFAULT_KINECT_HAND_CURSOR_RADIUS;
}


/** Called when the user selects a new Clipping Algorithm.
 * @param clippingAlgorithmId The identifier for the newly selected clipping algorithm. */
void CKinectInputProcessor::OnClippingAlgorithmChanged( int clippingAlgorithmId )
{
}


/** Called when the WiiMote state is refreshed. */
void CKinectInputProcessor::OnInputDeviceStateUpdated( void )
{
}

#ifndef KINECTINPUTPROCESSOR_HPP__
#define KINECTINPUTPROCESSOR_HPP__

#include "CInputDeviceProcessor.hpp"

/** Processes the states of the WiiMote during the Application's execution. */
class CKinectInputProcessor : public CInputDeviceProcessor
{
private:
	// METHODS
	CKinectInputProcessor( const CKinectInputProcessor & ) { /* not usable */ }

	// PROPERTIES
	int m_leftHandPosX;    /**< The Depth Map position for the user's left hand, when it is tracked by Kinect (X coordinate). */
	int m_leftHandPosY;    /**< The Depth Map position for the user's left hand, when it is tracked by Kinect (Y coordinate). */
	int m_rightHandPosX;   /**< The Depth Map position for the user's left hand, when it is tracked by Kinect (X coordinate). */
	int m_rightHandPosY;   /**< The Depth Map position for the user's left hand, when it is tracked by Kinect (Y coordinate). */

public:
	// CONSTANTS
	/** A special position (coordinate) into Kinect's Depth Map, which is set to indicate the player's hand is considered to
	 * be NOT tracked by Kinect. */
	static const int INVALID_HAND_POSITION = -0xAABB;

	// METHODS
	CKinectInputProcessor( void );
	~CKinectInputProcessor( void );

	void updateHandDepthMapPosition( bool bLeftHand, int x, int y );
	void unsetHandDepthMapPosition( bool bLeftHand );
	void getHandPosition( bool bLeftHand, int &x, int &y ) const;
	int getHandCursorRadius( void ) const;

	// VIRTUAL METHODS: CInputDeviceProcessor
	virtual void OnClippingAlgorithmChanged( int clippingAlgorithmId );

	// VIRTUAL METHODS: CInputDeviceEventsListener
	//virtual void OnInputDeviceConnectionChanged( bool bConnected );
	virtual void OnInputDeviceStateUpdated( void );
};

#endif

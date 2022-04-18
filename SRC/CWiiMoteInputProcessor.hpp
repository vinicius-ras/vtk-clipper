#ifndef WIIMOTEINPUTPROCESSOR_HPP__
#define WIIMOTEINPUTPROCESSOR_HPP__

#include "CInputDeviceProcessor.hpp"

/** Processes the states of the WiiMote during the Application's execution. */
class CWiiMoteInputProcessor : public CInputDeviceProcessor
{
private:
	// METHODS
	CWiiMoteInputProcessor( const CWiiMoteInputProcessor & ) { /* not usable */ }


	// PROPERTIES
	std::string m_curControlState;   ///< A string representing what the user should be doing right now.

    float m_lastMotionPlusYaw;       ///< Keeps the last Yaw speed value of the Wii Motion Plus accessory.
    float m_lastMotionPlusPitch;     ///< Keeps the last Pitch speed value of the Wii Motion Plus accessory.
    float m_lastMotionPlusRoll;      ///< Keeps the last Roll speed value of the Wii Motion Plus accessory.

public:
	// METHODS
	CWiiMoteInputProcessor( void );

	// VIRTUAL METHODS: CInputDeviceProcessor
	virtual void OnClippingAlgorithmChanged( int clippingAlgorithmId );

	// VIRTUAL METHODS: CInputDeviceEventsListener
	//virtual void OnInputDeviceConnectionChanged( bool bConnected );
	virtual void OnInputDeviceStateUpdated( void );
};

#endif

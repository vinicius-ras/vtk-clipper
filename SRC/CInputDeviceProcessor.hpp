#ifndef INPUTDEVICEPROCESSOR_HPP__
#define INPUTDEVICEPROCESSOR_HPP__

#include "CInputDeviceEventsListener.hpp"
#include <vtkObject.h>

/** This abstract class represents the behaviour executed while processing an Input Device (e.g.: WiiMote, Kinect)
 * for the Application. */
class CInputDeviceProcessor : public CInputDeviceEventsListener, public vtkObject
{
public:
	// VIRTUAL METHODS
	virtual void OnClippingAlgorithmChanged( int clippingAlgorithmId );

	// VIRTUAL METHODS: CInputDeviceEventsListener
	virtual void OnInputDeviceConnectionChanged( bool bConnected );
	virtual void OnInputDeviceStateUpdated( void );


protected:
	// METHODS
	CInputDeviceProcessor( void );
	CInputDeviceProcessor( const CInputDeviceProcessor & );
	virtual ~CInputDeviceProcessor( void );
};

#endif

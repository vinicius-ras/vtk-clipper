#ifndef CWIIMOTEUPDATEDLISTENER_HPP__
#define CWIIMOTEUPDATEDLISTENER_HPP__

#include "wiimote/wiimote.h"

/** Objects from this class can be registered with CApp to be informed whenever the current Input Controller's state has changed. */
class CInputDeviceEventsListener
{
public:
	virtual void OnInputDeviceConnectionChanged( bool bConnected ) = 0;
	virtual void OnInputDeviceStateUpdated( void ) = 0;
};

#endif

#ifndef CAPP_HPP__
#define CAPP_HPP__

#include <wx/app.h>
#include <wx/timer.h>
#include "wiimote/wiimote.h"
#include <memory>

#include "CInputDeviceEventsListener.hpp"

class wiimote;
class CMainFrame;
class CInputDeviceProcessor;

struct INuiSensor;

enum EInputDevices;

/** The class that represents the application itself. This class manages the main application
 * resources and its execution flow. */
class CApp : public wxApp
{
private:
	// PROPERTIES
	CMainFrame *m_mainFrame;
	std::auto_ptr<wxTimer> m_wiimoteUpdateTimer;
	std::vector<CInputDeviceEventsListener *> m_wiimoteUpdateListeners;
	bool m_lockWiiMoteUpdate;

	bool m_bSimulateKinect;
	bool m_bKinectSmoothMovement;
	bool m_isSimulatedKinectConnected;

	int m_currentInputDeviceId;

	// STATIC METHODS
	static void OnWiiMoteStateChanged(wiimote &remote, state_change_flags changed, const wiimote_state &new_state);



	// METHODS
	void warnListenersAboutInputDeviceConnection( bool bConnected );
	void warnListenersAboutInputDeviceStateUpdated();

    void OnInputDeviceUpdateTimer( wxTimerEvent & );

public:
	// METHODS
	CMainFrame &getMainFrame();
	void destroyUpdateTimer( void );

	wiimote &getWiiMote( void );
	void destroyCurrentWiiMoteInstance( void );

	bool createKinect( void );
	INuiSensor &getKinect( void );
	void destroyCurrentKinectInstance( void );
	bool isKinectConnected( void ) const;

	EInputDevices getCurrentInputDeviceId( void ) const;
	void setCurrentInputDeviceId( EInputDevices deviceId );
	CInputDeviceProcessor &getCurrentInputDeviceProcessor( void ) const;

	void addWiiMoteUpdateListener( CInputDeviceEventsListener *listener );
	bool removeWiiMoteUpdateListener( CInputDeviceEventsListener *listener );

	void pauseWiiMoteUpdateTimer( bool bPause );
	bool isWiiMoteUpdateTimerRunning( void ) const;

	bool isKinectSimulationModeEnabled( void ) const;
	void setKinectSimulationMode( bool bEnable );

	bool isKinectSmoothMovementEnabled( void ) const;
	void setKinectSmoothMovement( bool bEnable );

	virtual bool OnInit();

protected:
	DECLARE_EVENT_TABLE()
};

DECLARE_APP( CApp );

#endif

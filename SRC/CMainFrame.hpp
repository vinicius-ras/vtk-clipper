#ifndef CMAINFRAME_HPP__
#define CMAINFRAME_HPP__

#include "CInputDeviceEventsListener.hpp"
#include <wx/wx.h>
#include <vtkSmartPointer.h>

class vtkRenderer;
class wxVTKRenderWindowInteractor;
class CVolumeClipperAppInteractorStyle;
class CKinectRenderer;

enum EClippingAlgorithm;

/** The application's main frame (window). */
class CMainFrame : public wxFrame, public CInputDeviceEventsListener
{
public:
	// METHODS
	CMainFrame();
	~CMainFrame();

	void updateSelectedClippingAlgorithm( EClippingAlgorithm algorithmId );

    vtkSmartPointer<vtkRenderer> getRenderer( void ) const;
	vtkSmartPointer<CVolumeClipperAppInteractorStyle> getInteractorStyle( void );
    bool isWiiMotionPlusChecked( void ) const;

	float getWiiMoteSensitivity( void ) const;

	void refreshKinectRenderPanel( int imgWidth, int imgHeight, const void * const depthData );
	CKinectRenderer &getKinectRenderer( void ) const;

private:
	// PROPERTIES
	wxVTKRenderWindowInteractor *m_pVTKWindow;
	vtkSmartPointer<CVolumeClipperAppInteractorStyle> m_interactorStyle;
	vtkSmartPointer<vtkRenderer> m_renderer;
	std::auto_ptr<CKinectRenderer> m_kinectRenderer;

	// METHODS
	bool loadDICOMSeriesFromDirectory( const std::string &dicomDir );
	void resetWiiMoteDisplayedData( void );
	void updateToolConfigurationPanel( void );

	void wiiMoteConnected( bool bConnected );
	void kinectConnected( bool bConnected );

	// EVENT HANDLING METHODS (wxWidgets)
	void OnClose( wxCloseEvent &evt );
	void OnMenuItemExit( wxCommandEvent &evt );
	void OnMenuItemAbout( wxCommandEvent &evt );
	void OnMenuItemOpenDICOMDirectory( wxCommandEvent &evt );
	void OnMenuItemKinectRenderingRadio( wxCommandEvent &evt );
	void OnMenuItemKinectSimulationMode( wxCommandEvent &evt );
	void OnMenuItemKinectSmoothing( wxCommandEvent &evt );
	void OnToolBarAlgorithmSelected( wxCommandEvent &evt );
	void OnToggleButtonEnable3DTools( wxCommandEvent &evt );
	void OnCheckboxEnableWiiMotionPlus( wxCommandEvent &evt );
	void OnButtonClearAppConsole( wxCommandEvent &evt );
	void OnButtonConnectWiiMote( wxCommandEvent &evt );
	void OnButtonConnectKinect( wxCommandEvent &evt );
	void OnSliderKinectElevationScrolled( wxCommandEvent & );


	// OVERRIDEN METHODS: CInputDeviceEventsListener
	virtual void OnInputDeviceConnectionChanged( bool bConnected );
	virtual void OnInputDeviceStateUpdated( void );

	// Declares wxWidgets' event table
	DECLARE_EVENT_TABLE()
};


#endif

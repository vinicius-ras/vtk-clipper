#ifndef CPANELERASERTOOLCONFIG_HPP__
#define CPANELERASERTOOLCONFIG_HPP__

#include <wx/panel.h>
#include <wx/event.h>
#include <wx/slider.h>
#include <wx/textctrl.h>

/** A panel used by the user to configure the 2D and 3D Eraser tools. */
class CPanelEraserToolConfig : public wxPanel
{
public:
	// METHODS
	explicit CPanelEraserToolConfig( wxWindow *parent );


private:
	// PROPERTIES
	wxSlider *m_sliderRadius;
	wxTextCtrl *m_txtCursorRadius;

	// METHODS
	void updateCursor( void );

	void OnRadiusSliderScrolled( wxCommandEvent & );
	void OnButtonSetCursorRadius( wxCommandEvent & );

	DECLARE_EVENT_TABLE()
};

#endif
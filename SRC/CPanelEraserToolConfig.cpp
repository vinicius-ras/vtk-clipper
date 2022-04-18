#include "CPanelEraserToolConfig.hpp"
#include "CApp.hpp"
#include "CMainFrame.hpp"
#include "CVolumeClipperAppInteractorStyle.hpp"
#include "clippingAlgorithms.hpp"
#include "CEraser2D.hpp"
#include "CEraser3D.hpp"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include <wx/sizer.h>
#include <wx/xrc/xmlres.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>
#include <memory>

// FILE CONSTANTS
static const wxString PANEL_ERASER_TOOL_CONFIG( _T( "PanelEraserToolConfig" ) );
static const wxString SLIDER_CURSOR_RADIUS( _T( "m_sliderCursorRadius" ) );
static const wxString TEXTCTRL_CURSOR_RADIUS( _T( "m_txtCursorRadius" ) );
static const wxString BUTTON_SET_CURSOR_RADIUS( _T( "m_btSetCursorRadius" ) );
static const wxString STR_ENTER_RADIUS_MSG( _T( "Enter the Radius value." ) );
static const wxString STR_ENTER_RADIUS_CAPTION( _T( "Radius value" ) );
static const wxString STR_ENTERED_INVALID_RADIUS_MSG( _T( "An invalid Radius value has been entered!" ) );
static const wxString STR_ENTERED_INVALID_RADIUS_CAPTION( _T( "Invalid value" ) );

// PANEL'S EVENT TABLE
BEGIN_EVENT_TABLE( CPanelEraserToolConfig, wxPanel )
	EVT_BUTTON( XRCID( BUTTON_SET_CURSOR_RADIUS ), OnButtonSetCursorRadius )
	EVT_SLIDER( XRCID(SLIDER_CURSOR_RADIUS), OnRadiusSliderScrolled )
END_EVENT_TABLE()


// METHODS
CPanelEraserToolConfig::CPanelEraserToolConfig( wxWindow *parent )
: wxPanel()
{
	// Load the panel
	wxXmlResource &resources = *wxXmlResource::Get();
	resources.LoadPanel( this, parent, PANEL_ERASER_TOOL_CONFIG );

	// Initialize control references
	m_sliderRadius = static_cast<wxSlider *>( wxWindow::FindWindowById( XRCID(SLIDER_CURSOR_RADIUS), this ) );
	m_txtCursorRadius = static_cast<wxTextCtrl *>( wxWindow::FindWindowById( XRCID(TEXTCTRL_CURSOR_RADIUS), this ) );

	// Initilize radius controls
	m_txtCursorRadius->SetValue( wxString::Format( _T( "%d" ), m_sliderRadius->GetValue() ) );
}


/** Updates the cursor being used by the user. */
void CPanelEraserToolConfig::updateCursor( void )
{
	CApp &app = wxGetApp();
	CMainFrame &mainFrame = app.getMainFrame();

	vtkSmartPointer<vtkRenderer> renderer = mainFrame.getRenderer();
	CVolumeClipperAppInteractorStyle *interactorStyle = static_cast<CVolumeClipperAppInteractorStyle *>( renderer->GetRenderWindow()->GetInteractor()->GetInteractorStyle() );

	// Get the entered scale value
	double enteredValue = -1;
	bool coversionResult = m_txtCursorRadius->GetValue().ToDouble( &enteredValue );
	assert( coversionResult == true );

	int curClippingAlgorithm = interactorStyle->getClippingAlgorithmId();
	assert( curClippingAlgorithm == evAlgorithmEraser2D || curClippingAlgorithm == evAlgorithmEraser3D );

	const bool isEraser2D = (curClippingAlgorithm == evAlgorithmEraser2D);
	const double sizeValue = ( enteredValue / 100.0 ) * ( ( isEraser2D ) ? CEraser2D::DEFAULT_ERASER2D_SIZE : CEraser3D::DEFAULT_CURSOR_RADIUS );

	// Verify the current clipping algorithm
	if ( curClippingAlgorithm == evAlgorithmEraser2D )
	{
		CEraser2D &eraser2D = static_cast<CEraser2D &>( interactorStyle->getClippingAlgorithm() );
		eraser2D.setCursorSize( sizeValue );

		interactorStyle->updateEraserCursor();
	}
	else if ( curClippingAlgorithm == evAlgorithmEraser3D )
	{
		CEraser3D &eraser3D = static_cast<CEraser3D &>( interactorStyle->getClippingAlgorithm() );
		eraser3D.setCylinderRadius( sizeValue );

		interactorStyle->updateEraserCursor();
	}
}


/** Called when the user changes the Eraser Radius Slider. */
void CPanelEraserToolConfig::OnRadiusSliderScrolled( wxCommandEvent & )
{
	m_txtCursorRadius->SetLabel( wxString::Format( _T("%d"), m_sliderRadius->GetValue() ) );
	updateCursor();
}


/** Called when the user clicks the "Set Cursor Radius" button. */
void CPanelEraserToolConfig::OnButtonSetCursorRadius( wxCommandEvent & )
{
	// Ask the user to type the value
	std::auto_ptr<wxTextEntryDialog> entryDlg( new wxTextEntryDialog( this, STR_ENTER_RADIUS_MSG, STR_ENTER_RADIUS_CAPTION, m_txtCursorRadius->GetValue() ) );
	long typedNum;
	bool firstTime = true;
	do
	{
		// Print error messages
		if ( firstTime == false )
		{
			wxMessageBox( STR_ENTERED_INVALID_RADIUS_MSG, STR_ENTERED_INVALID_RADIUS_CAPTION,
				wxOK | wxICON_EXCLAMATION, this );
		}

		// Show the dialog
		if ( entryDlg->ShowModal() != wxID_OK )
			return;
		firstTime = false;
	} while ( entryDlg->GetValue().ToLong( &typedNum ) == false );

	// Update cursor
	m_txtCursorRadius->SetValue( entryDlg->GetValue() );
	updateCursor();
}

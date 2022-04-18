#include "CAboutDialog.hpp"
#include <wx/xrc/xmlres.h>
#include <cassert>

// CONSTANTS
static const char *ABOUT_DIALOG_NAME = "AboutDialog";
static const char *BUTTON_OK = "m_btOk";


// EVENT TABLE (wxWidgets)
BEGIN_EVENT_TABLE( CAboutDialog, wxDialog )
END_EVENT_TABLE()


// METHODS
/** Contructor.
 * @param parent The window that will be the parent for the dialog.
 * @param modal A flag indicating if the dialog is modal or not.
 */
CAboutDialog::CAboutDialog( wxWindow *parent, bool modal )
	: wxDialog()
{
	// Load the dialog
	wxXmlResource &resources = *wxXmlResource::Get();
	bool result = resources.LoadDialog( this, parent, ABOUT_DIALOG_NAME );
	assert(result);

	// Set the default buttons
	SetAffirmativeId( XRCID(BUTTON_OK) );
}


/** Destructor. */
CAboutDialog::~CAboutDialog()
{
}


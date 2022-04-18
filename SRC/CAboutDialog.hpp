#ifndef CABOUTDIALOG_HPP__
#define CABOUTDIALOG_HPP__

#include <wx/dialog.h>

/** This class instantiates and handles the About dialog of the application. */
class CAboutDialog : public wxDialog
{
public:
	// METHODS
	CAboutDialog( wxWindow *parent, bool modal );
	~CAboutDialog();

private:
	DECLARE_EVENT_TABLE()
};

#endif

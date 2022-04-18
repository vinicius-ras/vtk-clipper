#ifndef CAPPCONSOLE_HPP__
#define CAPPCONSOLE_HPP__

#include <wx/textctrl.h>
#include <wx/string.h>

/** This class handles the console that is displayed to the user via the wxWidgets GUI. */
class CAppConsole
{
private:
	// PROPERTIES
	wxTextCtrl *m_txtConsole;   ///< The widget to use as a console which logs application's data.

	// METHODS
	explicit CAppConsole( wxTextCtrl *pTextCtrl );


public:
	// STATIC METHODS
	static void initialize( wxTextCtrl *pTextCtrl );
	static CAppConsole &getInstance( void );

	// METHODS
	void log( const wxString &message );
	void clearLog( void );
};

#endif

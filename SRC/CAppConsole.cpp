#include "CAppConsole.hpp"
#include <memory>
#include <cassert>

// FILE GLOBALS
/** The unique instance of the App Console. */
std::auto_ptr<CAppConsole> sg_consoleInstance;

// STATIC METHODS
/** Initializes the App Console, by creating its unique instance.
 * @param textCtrl A pointer to the widget that will be used as a console. */
void CAppConsole::initialize( wxTextCtrl *textCtrl )
{
	// Can only inigialize the App Console ONE TIME, and no more than that.
	assert( sg_consoleInstance.get() == NULL );

	// Instantiate App Console
	sg_consoleInstance.reset( new CAppConsole( textCtrl ) );
}


/** Retrieves the singleton instance of the App Console. Notice that by the time this method is
 * called, the App Console should have already been initialized by calling #CAppConsole::initialize(). */
CAppConsole &CAppConsole::getInstance( void )
{
	// Ensures the method CAppConsole::initialize() has been called already
	assert( sg_consoleInstance.get() != NULL );

	return *sg_consoleInstance;
}


// METHODS
/** Constructor.
 * @param textCtrl The widget to be used as a console. */
CAppConsole::CAppConsole( wxTextCtrl *textCtrl )
	: m_txtConsole( textCtrl )
{
	// The App Console MUST exist.
	assert( m_txtConsole );
}


/** Logs a message to the App Console.
 * @param message The message to be logged to the App Console. */
void CAppConsole::log( const wxString &message )
{
	m_txtConsole->AppendText( message );
	m_txtConsole->AppendText( _T("\n") );
}


/** Clears all the messages contained into the App Console. */
void CAppConsole::clearLog( void )
{
	m_txtConsole->Clear();
}

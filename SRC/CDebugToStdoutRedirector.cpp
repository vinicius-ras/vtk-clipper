#include "CDebugToStdoutRedirector.hpp"
#include <sstream>

// METHODS
CDebugToStdoutRedirector::CDebugToStdoutRedirector()
{
	std::cout << "CONSTRUTOR DE CDebugToStdoutRedirector" << std::endl;
}

CDebugToStdoutRedirector::~CDebugToStdoutRedirector()
{
	std::cout << "DESTRUTOR DE CDebugToStdoutRedirector" << std::endl;
}

/** Creates an instance of CDebugToStdoutRedirector. This is implemented so the class can be used with the
 * vtkSmartPointer template.
 * @return Returns the new instance of the class.
 */
CDebugToStdoutRedirector *CDebugToStdoutRedirector::New()
{
	return new CDebugToStdoutRedirector();
}


/** Utility method to print a text with a given prefix to stdout.
* @param prefix The prefix to be printed before the text.
* @param text The text to be printed.
*/
void CDebugToStdoutRedirector::displayTextWithPrefix( const char *prefix, const char *text )
{
	std::ostringstream stream(prefix);
	stream << ' ' << text;
	DisplayText( stream.str().c_str() );
}


/** Overrides vtkOutputWindow method to print the given text to stdout.
 *  @param txt The text to be printed.
 */
void CDebugToStdoutRedirector::DisplayText (const char *txt)
{
	std::cout << txt << std::endl;
}


/** Overrides vtkOutputWindow method to print the given text to stdout, with an error prefix.
 *  @param txt The text to be printed.
 */
void CDebugToStdoutRedirector::DisplayErrorText(const char *txt)
{
	displayTextWithPrefix( "[ERROR]", txt );
}


/** Overrides vtkOutputWindow method to print the given text to stdout, with a warning prefix.
 *  @param txt The text to be printed.
 */
void CDebugToStdoutRedirector::DisplayWarningText(const char *txt)
{
	displayTextWithPrefix( "[WARN]", txt );
}


/** Overrides vtkOutputWindow method to print the given text to stdout, with a generic warning prefix.
 *  @param txt The text to be printed.
 */
void CDebugToStdoutRedirector::DisplayGenericWarningText(const char *txt)
{
	displayTextWithPrefix( "[GENERIC WARN]", txt );
}

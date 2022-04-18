#ifndef DEBUGTOSTDOUTREDIRECTOR_HPP__
#define DEBUGTOSTDOUTREDIRECTOR_HPP__

#include <vtkOutputWindow.h>

/** This class overrides the methods of vtkOutputWindow class to redirect its output to STDOUT channel. */
class CDebugToStdoutRedirector : public vtkOutputWindow
{
private:
	// METHODS
	void displayTextWithPrefix( const char *prefix, const char *text );


protected:
	// METHODS
	CDebugToStdoutRedirector();
	~CDebugToStdoutRedirector();


public:
	// STATIC METHODS
	static CDebugToStdoutRedirector *New();

	// OVERRIDEN METHODS: vtkOutputWindow
	virtual void DisplayText (const char *txt);
	virtual void DisplayErrorText (const char *txt);
	virtual void DisplayWarningText (const char *txt);
	virtual void DisplayGenericWarningText (const char *txt);


};


#endif

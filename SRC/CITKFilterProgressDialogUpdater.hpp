#ifndef CITKFILTERPROGRESSDIALOGUPDATER_HPP__
#define CITKFILTERPROGRESSDIALOGUPDATER_HPP__

#include <itkCommand.h>
#include <itkProcessObject.h>

class wxProgressDialog;

/** This class links wxWidgets' progress dialogs to ITK filters, updating the progress dialog
 * with the progress of the filter.
 */
class CITKFilterProgressDialogUpdater : public itk::Command
{
private:
	// PROPERTIES
	wxProgressDialog *m_progressDlg;
	itk::ProcessObject *m_filter;


public:
	// METHODS
	CITKFilterProgressDialogUpdater();

    void Execute( itk::Object *caller, const itk::EventObject &evt );
    void Execute( const itk::Object *object, const itk::EventObject &evt );

	void setObserverConfiguration( wxProgressDialog &progressDlg, itk::ProcessObject &filter );

	itkNewMacro( CITKFilterProgressDialogUpdater );
};

#endif
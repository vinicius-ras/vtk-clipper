#include "CITKFilterProgressDialogUpdater.hpp"
#include <wx/progdlg.h>
#include <itkProcessObject.h>

// METHODS
/** Constructor. */
CITKFilterProgressDialogUpdater::CITKFilterProgressDialogUpdater()
	: m_progressDlg( NULL ),
	m_filter( NULL )
{
}


/** Executes during the ITK callback. */
void CITKFilterProgressDialogUpdater::Execute(itk::Object *caller, const itk::EventObject & event)
{
	Execute( const_cast<const itk::Object *>( caller ), event);
}


/** Executes during the ITK callback. */
void CITKFilterProgressDialogUpdater::Execute(const itk::Object * object, const itk::EventObject & event)
{
	m_progressDlg->Update( static_cast<int>( m_filter->GetProgress() * 100 ) );
}


/** Configures the object for execution.
 * @param progressDlg A reference to the progress dialog that will be controlled by the object.
 * @param filter The filter that will be observed, and whose progress will be monitored.
 */
void CITKFilterProgressDialogUpdater::setObserverConfiguration( wxProgressDialog &progressDlg, itk::ProcessObject &filter )
{
	m_progressDlg = &progressDlg;
	m_filter = &filter;
}

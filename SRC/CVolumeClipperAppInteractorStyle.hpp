#ifndef VOLUMECLIPPERAPPINTERACTORSTYLE_HPP__
#define VOLUMECLIPPERAPPINTERACTORSTYLE_HPP__

#include "CVector3.hpp"
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderer.h>
#include <vtkFollower.h>
#include <vtkVolume.h>
#include <vtkUnsignedShortArray.h>
#include <vtkSmartPointer.h>
#include <wx/wx.h>

#include <memory>

class CClippingAlgorithm;
class CInputDeviceProcessor;

/** A subclass of vtkInteractorStyle which controls the flow of execution and interaction of the application
 * with the user. */
class CVolumeClipperAppInteractorStyle : public wxEvtHandler, public vtkInteractorStyleTrackballCamera
{
	vtkTypeRevisionMacro(CVolumeClipperAppInteractorStyle,vtkInteractorStyleTrackballCamera);

private:
	// TYPEDEFS
	typedef vtkSmartPointer<vtkProp> TActorSmartPtr;
	typedef vtkSmartPointer<CInputDeviceProcessor> TInputDeviceProcessorPtr;

	// PROPERTIES
	vtkSmartPointer<CClippingAlgorithm> m_curClippingAlgorithm;   ///< A reference to the object that represents the currently selected Clipping Algorithm.
	TVector3Array m_algorithmPoints;                              ///< An array of points, defined by the user, that can be used for some Clipping Algorithms.
	int m_currentAlgorithmId;                                     ///< The identifier of the clipping algorithm that is currently selected by the user.
	std::vector<TActorSmartPtr> m_cursorActors;                   ///< A set of actors that can be used by the current clipping algorithm.

    std::auto_ptr<wxTimer> m_renderRateAdjustingTimer;            ///< A timer used to adjust the VTK's rendering rate when the mouse is moving.

	TInputDeviceProcessorPtr m_inputDeviceProcessor;              ///< An object whose behaviour is used to process the current Input Device state.

	// METHODS
	CVolumeClipperAppInteractorStyle();
	~CVolumeClipperAppInteractorStyle();

	const CVector3 screenToWorldCoords( int sx, int sy, double z );

	void clearCursorActors( void );

    void OnRenderRateAdjustingTimer( wxTimerEvent & );

public:
	// CONSTANTS
	/** The default time it takes for the Render Rate to restore to the default, after it has been
	 * set (usually for performance gaining purposes). */
	static const int DEFAULT_RENDER_RATE_RESTORE_TIME_MS = 500;
	// PROPERTIES
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkVolume> volume;
	int dimensoes[3];
	vtkSmartPointer<vtkUnsignedShortArray> imgDataArray;


	// STATIC METHODS
	static CVolumeClipperAppInteractorStyle* New();


	// METHODS
	void setClippingAlgorithm( int algorithmId );
	int getClippingAlgorithmId( void ) const;
	CClippingAlgorithm &getClippingAlgorithm( void ) const;
	void startCurrentClippingAlgorithm();

	void startRefreshRateChange( int timeToRestoreRate = CVolumeClipperAppInteractorStyle::DEFAULT_RENDER_RATE_RESTORE_TIME_MS );

	void setInputDeviceProcessor( TInputDeviceProcessorPtr inputDevProc );

	TActorSmartPtr getCursorActor( int index ) const;
	void updateEraserCursor( void );

	CInputDeviceProcessor &getInputDeviceProcessor( void ) const;

	// OVERRIDEN METHODS: vtkInteractorStyleTrackballCamera
	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();

protected:
    DECLARE_EVENT_TABLE()
};


#endif

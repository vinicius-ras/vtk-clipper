#ifndef KINECTRENDERER_HPP__
#define KINECTRENDERER_HPP__

#include <memory>
#include <wx/wx.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkActor2D.h>
#include <vtkInteractorStyleUser.h>
#include <vtkImageCanvasSource2D.h>
#include "wxVTKRenderWindowInteractor.h"

class CKinectMenu;


/** This is a specialized class for rendering the Kinect depth map and data into a wxWidgets panel. */
class CKinectRenderer : public wxEvtHandler, public vtkInteractorStyleUser
{
public:
	// TYPEDEFS
	/** A shared pointer to a Kinect menu. */
	typedef std::shared_ptr<CKinectMenu> TKinectMenuSharedPtr;

	// ENUMERATIONS
	/** Provides all of the possible rendering types for the Kinect Renderer, which affect the way the Kinect data is
	 * displayed to the user. */
	enum ERenderingType
	{
		evRenderingTypeFull,
		evRenderingTypeUserAndHands,
		evRenderingTypeHandsOnly,
	};

	// METHODS
	CKinectRenderer( wxPanel *parent );
	~CKinectRenderer();

	void copyDepthDataToBuffer( int imgWidth, int imgHeight, const void * const depthData );

	ERenderingType getRenderingType( void ) const;
	void setRenderingType( ERenderingType renderingType );

	void setKinectMenu( TKinectMenuSharedPtr menu );
	TKinectMenuSharedPtr getKinectMenu( void ) const;

	void OnRenderAndProcessingTimer( wxTimerEvent & );

	virtual void OnMouseMove( void );
	virtual void OnLeftButtonDown( void );
	virtual void OnRightButtonDown( void );
	virtual void OnLeftButtonUp( void );
	virtual void OnRightButtonUp( void );


private:
	// METHODS
	void onMouseEvent( void );
	vtkSmartPointer<vtkActor2D> createUserHandCursor( void );

	// PROPERTIES
	vtkSmartPointer<vtkRenderer> m_kinectVTKRenderer;   ///< The VTK renderer used for drawing into the Kinect Rendering Panel.
	wxVTKRenderWindowInteractor *m_pKinectVTKWindow;    ///< The wxWidgets window that will contain VTK's render window.
	std::auto_ptr<wxTimer> m_updateTimer;               ///< The timer used to update the renderer, by processing menus and rendering the panel.

	void *m_pDepthData;               ///< A buffer containing the Depth data sent by Kinect, to be drawn on the next Paint event.
	size_t m_pDepthDataLength;        ///< The allocated size of the m_pDepthData buffer.
	ERenderingType m_renderingType;   ///< The type of rendering the Kinect Render should do.

	TKinectMenuSharedPtr m_menu;      ///< The menu currently in use by the Kinect Panel.

	bool m_isMouseLeftDown;           ///< Flag which will keep track if the Left mouse button is currently down.
	bool m_isMouseRightDown;           ///< Flag which will keep track if the Right mouse button is currently down.

	vtkSmartPointer<vtkActor2D> m_actorLeftHand;    ///< An actor for the cursor representing user's Left Hand.
	vtkSmartPointer<vtkActor2D> m_actorRightHand;   ///< An actor for the cursor representing user's Right Hand.

	vtkSmartPointer<vtkImageCanvasSource2D> m_backgroundCanvas;   ///< A canvas used to draw the user's body into the background.

	DECLARE_EVENT_TABLE()
};

#endif

#include "CVolumeClipperAppInteractorStyle.hpp"
#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRegularPolygonSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkFollower.h>
#include <vtkActor2D.h>
#include <vtkLineSource.h>
#include <vtkSphereSource.h>
#include <vtkViewport.h>
#include <vtkCylinderSource.h>
#include <vtkCubeSource.h>
#include <vtkPlaneSource.h>
#include <vtkProperty.h>
#include <vtkMatrix4x4.h>
#include <vtkTransformFilter.h>
#include <vtkTransform.h>

#include <wx/wx.h>
#include <wx/progdlg.h>

#include <ctime>
#include <cassert>
#include <vector>
#include <set>

#include "testes.hpp"
#include "CApp.hpp"
#include "CMainFrame.hpp"
#include "CEraser2D.hpp"
#include "CEraser3D.hpp"
#include "CClipper2D.hpp"
#include "CClipper3D.hpp"
#include "CPolyClipper2D.hpp"
#include "CInputDeviceProcessor.hpp"
#include "clippingAlgorithms.hpp"

#include "CAppConsole.hpp"

vtkCxxRevisionMacro(CVolumeClipperAppInteractorStyle, "$Revision: 1.1 $");
vtkStandardNewMacro(CVolumeClipperAppInteractorStyle);



// GLOBAL FILE CONSTANTS
static const int ERASER3D_CYLINDER_HEIGHT = 353;
static const int RENDER_RATE_ADJUSTING_TIMER_ID = 123321;

// EVENT TABLE (wxWidgets)
BEGIN_EVENT_TABLE( CVolumeClipperAppInteractorStyle, wxEvtHandler )
    EVT_TIMER( RENDER_RATE_ADJUSTING_TIMER_ID, CVolumeClipperAppInteractorStyle::OnRenderRateAdjustingTimer )
END_EVENT_TABLE()


// METHODS
/** Constructor. */
CVolumeClipperAppInteractorStyle::CVolumeClipperAppInteractorStyle()
	: m_curClippingAlgorithm(),
	m_algorithmPoints(),
	m_currentAlgorithmId(evAlgorithmNone),
	m_cursorActors(),
    m_renderRateAdjustingTimer()
{
    dimensoes[0] = -1;
	dimensoes[1] = -2;
	dimensoes[2] = -3;

	setClippingAlgorithm( evAlgorithmNone );

    // Create the render rate adjusting timer
    m_renderRateAdjustingTimer.reset( new wxTimer( this, RENDER_RATE_ADJUSTING_TIMER_ID ) );
}


/** Destructor. */
CVolumeClipperAppInteractorStyle::~CVolumeClipperAppInteractorStyle()
{
}


/** Modifies the current clipping algorithm.
 * @param algorithmId The identifier of the algorithm that should be selected. */
void CVolumeClipperAppInteractorStyle::setClippingAlgorithm( int algorithmId )
{
	// Clear last clipping algorithm's state data.
	clearCursorActors();
	m_algorithmPoints.clear();

	// Update current algorithm
	m_currentAlgorithmId = algorithmId;
	switch ( algorithmId )
	{
	case evAlgorithmNone:
		m_curClippingAlgorithm = NULL;
		break;
	case evAlgorithmEraser2D:
		{
			m_curClippingAlgorithm = vtkSmartPointer<CEraser2D>::New();
			updateEraserCursor();
		}
		break;
	case evAlgorithmEraser3D:
		{
			m_curClippingAlgorithm = vtkSmartPointer<CEraser3D>::New();

			updateEraserCursor();
		}
		break;
	case evAlgorithmClipper2D:
		m_curClippingAlgorithm = vtkSmartPointer<CClipper2D>::New();
		break;
	case evAlgorithmClipper3D:
		{
			m_curClippingAlgorithm = vtkSmartPointer<CClipper3D>::New();

			// Retrieve a reference to the Clipper3D algorithm we've just created
			CClippingAlgorithm *curAlgorithm = m_curClippingAlgorithm.GetPointer();
			CClipper3D &clipper3D = static_cast<CClipper3D &>(*curAlgorithm);

			// Retrieve the bounds of the volume
			double volBounds[6] = { 0 };
			if ( volume.GetPointer() != NULL )
				volume->GetBounds(volBounds);

			// Estimate a good height for the cylinder
			double volWidth = volBounds[1] - volBounds[0];
			double volHeight = volBounds[3] - volBounds[2];
			double volDepth = volBounds[5] - volBounds[4];
			assert( volWidth >= 0 );
			assert( volHeight >= 0 );
			assert( volDepth >= 0 );
			double baseSize = sqrt(volWidth*volWidth + volHeight*volHeight + volDepth*volDepth);

			// Create the cylinder actor
			vtkSmartPointer<vtkRegularPolygonSource> cursorSrc = vtkSmartPointer<vtkRegularPolygonSource>::New();
			cursorSrc->SetNumberOfSides( 4 );
			cursorSrc->SetRadius( baseSize );
			cursorSrc->GeneratePolylineOff();
			cursorSrc->GeneratePolygonOn();

			vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
			mapper->SetInput( cursorSrc->GetOutput() );

			vtkSmartPointer<vtkActor> cursorActor = vtkSmartPointer<vtkActor>::New();
			cursorActor->SetMapper( mapper );
			cursorActor->SetPosition( (volBounds[0] + volBounds[1])/2, (volBounds[2] + volBounds[3])/2, (volBounds[4] + volBounds[5])/2 );
			renderer->AddActor( cursorActor );

			m_cursorActors.push_back( cursorActor );

            // Create a guider line, which tells the side of the plane that will be erased
            vtkSmartPointer<vtkLineSource> lineSrc = vtkSmartPointer<vtkLineSource>::New();
            lineSrc->SetPoint1( 0, 0, 0 );
            lineSrc->SetPoint2( 0, 0, baseSize );

			vtkSmartPointer<vtkPolyDataMapper> lineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
			lineMapper->SetInput( lineSrc->GetOutput() );

			vtkSmartPointer<vtkActor> lineActor = vtkSmartPointer<vtkActor>::New();
			lineActor->SetMapper( lineMapper );
            lineActor->GetProperty()->SetColor( 1, 0, 0 );
            lineActor->SetPosition( cursorActor->GetPosition() );
            lineActor->SetOrientation( cursorActor->GetOrientation() );

            renderer->AddActor( lineActor );
            m_cursorActors.push_back( lineActor );
		}
		break;
	case evAlgorithmPolyClipper2D:
		m_curClippingAlgorithm = vtkSmartPointer<CPolyClipper2D>::New();
		break;
	}

    // Render the scene
    if ( renderer.GetPointer() != NULL )
        renderer->GetRenderWindow()->Render();

	// Warn the Input Device Processor (if any) that the clipping algorithm has changed
	if ( m_inputDeviceProcessor.GetPointer() != NULL )
		m_inputDeviceProcessor->OnClippingAlgorithmChanged( m_currentAlgorithmId );
}


/** Retrieves the identifier of the current clipping algorithm selected by the user.
 * @return Returns the identifier of the current clipping algorithm, which can be compared
 *    against the enumerators of #EClippingAlgorithm.
 */
int CVolumeClipperAppInteractorStyle::getClippingAlgorithmId( void ) const
{
	return m_currentAlgorithmId;
}


/** Retrieves the currently set Clipping Algorithm.
 * @return Returns a reference to the current Clipping Algorithm.
 */
CClippingAlgorithm &CVolumeClipperAppInteractorStyle::getClippingAlgorithm( void ) const
{
	return *m_curClippingAlgorithm;
}


/** Updates the cursor for Eraser 2D and Eraser 3D algorithms. */
void CVolumeClipperAppInteractorStyle::updateEraserCursor( void )
{
	assert( m_currentAlgorithmId == evAlgorithmEraser2D || m_currentAlgorithmId == evAlgorithmEraser3D );

	// Lower the rendering resolution
	startRefreshRateChange();

	if ( m_currentAlgorithmId == evAlgorithmEraser2D )
	{
		// Retrieve the Eraser 2D algorithm's reference
		CClippingAlgorithm *curAlgorithm = m_curClippingAlgorithm.GetPointer();
		CEraser2D &eraser2D = static_cast<CEraser2D &>(*curAlgorithm);

		// Erases the previous cursor, keeping its position
		double lastCursorPos[4] = {0};
		if ( m_cursorActors.empty() == false )
		{
			vtkProp3D *cursor = static_cast<vtkProp3D *>(m_cursorActors[0].GetPointer());
			cursor->GetPosition( lastCursorPos );
		}

		clearCursorActors();

		// Creates a 2D "circunference"
		vtkSmartPointer<vtkRegularPolygonSource> polySrc = vtkSmartPointer<vtkRegularPolygonSource>::New();
		polySrc->SetRadius( eraser2D.getCursorSize() );
		polySrc->SetNumberOfSides(16);
		polySrc->GeneratePolygonOff();
		polySrc->GeneratePolylineOn();

		// Attaches the circunference to a Follower
		vtkSmartPointer<vtkPolyDataMapper> polyMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		polyMapper->SetInput( polySrc->GetOutput() );

		// This actor will be added to be used as a cursor for the user
		vtkSmartPointer<vtkFollower> followerActor = vtkSmartPointer<vtkFollower>::New();
		followerActor->SetMapper( polyMapper );
		followerActor->SetCamera( renderer->GetActiveCamera() );

		renderer->AddActor( followerActor );
		m_cursorActors.push_back( followerActor );

		// Set its position
		followerActor->SetPosition( lastCursorPos );
	}
	else if ( m_currentAlgorithmId == evAlgorithmEraser3D )
	{
		// Retrieve the Eraser 2D algorithm's reference
		CClippingAlgorithm *curAlgorithm = m_curClippingAlgorithm.GetPointer();
		CEraser3D &eraser3D = static_cast<CEraser3D &>(*curAlgorithm);

		// Retrieve the bounds of the volume
		double volBounds[6] = {0};
		if ( volume.GetPointer() != NULL )
			volume->GetBounds(volBounds);

		// Erases the previous cursor, keeping its position
		double lastCursorPos[4] = {0};
		double lastCursorOrientation[4] = {0};
		if ( m_cursorActors.empty() == false )
		{
			vtkProp3D *cursor = static_cast<vtkProp3D *>(m_cursorActors[0].GetPointer());
			cursor->GetPosition( lastCursorPos );
			cursor->GetOrientation( lastCursorOrientation );
		}
		else
		{
			lastCursorPos[0] = (volBounds[0] + volBounds[1])/2;
			lastCursorPos[1] = (volBounds[2] + volBounds[3])/2;
			lastCursorPos[2] = (volBounds[4] + volBounds[5])/2;
			lastCursorPos[3] = 1;
		}

		clearCursorActors();

		// Estimate a good height for the cylinder
		double volWidth = volBounds[1] - volBounds[0];
		double volHeight = volBounds[3] - volBounds[2];
		double volDepth = volBounds[5] - volBounds[4];
		assert( volWidth >= 0 );
		assert( volHeight >= 0 );
		assert( volDepth >= 0 );
		double cylHeight = sqrt(volWidth*volWidth + volHeight*volHeight + volDepth*volDepth);

		// Create the cylinder actor
		vtkSmartPointer<vtkCylinderSource> cylSrc = vtkSmartPointer<vtkCylinderSource>::New();
		cylSrc->SetHeight( ERASER3D_CYLINDER_HEIGHT );
		cylSrc->SetRadius( eraser3D.getCylinderRadius() );
		cylSrc->SetResolution( 30 );

		vtkSmartPointer<vtkPolyDataMapper> cylMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		cylMapper->SetInput( cylSrc->GetOutput() );

		vtkSmartPointer<vtkActor> cylActor = vtkSmartPointer<vtkActor>::New();
		cylActor->SetMapper( cylMapper );
		renderer->AddActor( cylActor );

		// Keep track of the created actor
		m_cursorActors.push_back(cylActor);

		cylActor->SetPosition( lastCursorPos );
		cylActor->SetOrientation( lastCursorOrientation );
	}
}


/** Retrieves the current Input Device Processor which is being used to interact with user's input, through special
 * devices (WiiMote, Kinect).
 * @return Returns a reference to the current Input Device Processor. */
CInputDeviceProcessor &CVolumeClipperAppInteractorStyle::getInputDeviceProcessor( void ) const
{
	return *m_inputDeviceProcessor;
}


/** Called by VTK when the mouse is moved into the VTK window's client area. */
void CVolumeClipperAppInteractorStyle::OnMouseMove()
{
	// Retrieves the Device Coordinates of the mouse
	int mx, my;
	GetInteractor()->GetEventPosition( mx, my );

	// Process the cursor changes of the current clipping algorithm
	switch ( m_currentAlgorithmId )
	{
	case evAlgorithmEraser2D:
		{
			// Retrieves a coordinate close to the near clipping plane to position the cursor
			double mouseCoords[4];
			renderer->SetDisplayPoint( mx, my, 0.05 );
			renderer->DisplayToWorld();
			renderer->GetWorldPoint( mouseCoords );

#ifdef DEBUG
			vtkActor *cursorActor = dynamic_cast<vtkActor *>( m_cursorActors[0].GetPointer() );
#else
			vtkActor *cursorActor = static_cast<vtkActor *>( m_cursorActors[0].GetPointer() );
#endif
			cursorActor->SetPosition( mouseCoords[0], mouseCoords[1], mouseCoords[2] );

            // Change the refresh rate for quick rendering
			startRefreshRateChange();
        }
		break;
	}

	// Calls the superclass' equivalent method
	vtkInteractorStyleTrackballCamera::OnMouseMove();
}


/** Called by VTK when the left mouse button is clicked into the VTK window's client area. */
void CVolumeClipperAppInteractorStyle::OnLeftButtonDown()
{
	int mx, my;
	GetInteractor()->GetEventPosition( mx, my );

	std::cout << "Pressed left mouse button @ " << mx << ", " << my << endl;
	if ( m_currentAlgorithmId != evAlgorithmNone )
	{
		// Some clipping algorithms should be initialized here, according to their specific type
		CClippingAlgorithm *curAlgorithm = m_curClippingAlgorithm.GetPointer();

		bool startAlgorithm = false;
		switch ( m_currentAlgorithmId )
		{
		case evAlgorithmEraser2D:
			{
				CEraser2D &eraser2D = static_cast<CEraser2D &>(*curAlgorithm);
				eraser2D.setScreenReferencePoint( renderer, mx, my );
				startAlgorithm = true;
			}
			break;
		case evAlgorithmClipper2D:
			{
				// Verify if 3 points are defined: 2 points which define the plane, and 1 point which defines which
				// part of the plane will be removed.
				CVector3 clickPointNear = screenToWorldCoords( mx, my, 0 );
				m_algorithmPoints.push_back( clickPointNear );
				std::cout << "Adicionou um ponto NEAR ao vetor..." << std::endl;

				const size_t totalPointsAdded = m_algorithmPoints.size();
                bool updateScreen = false;
				if ( totalPointsAdded <= 2 )
				{
					// Creates a 2D "circunference"
					vtkSmartPointer<vtkRegularPolygonSource> polySrc = vtkSmartPointer<vtkRegularPolygonSource>::New();
					polySrc->SetRadius( 3 );
					polySrc->SetNumberOfSides(20);

					// Attaches the circunference to a Follower
					vtkSmartPointer<vtkPolyDataMapper2D> polyMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
					polyMapper->SetInput( polySrc->GetOutput() );

					// This actor will be added to be used as a cursor for the user
					vtkSmartPointer<vtkActor2D> cursorActor = vtkSmartPointer<vtkActor2D>::New();
					cursorActor->SetMapper( polyMapper );
					cursorActor->SetPosition( mx, my );
					renderer->AddActor2D( cursorActor );

					m_cursorActors.push_back( cursorActor );

                    updateScreen = true;
				}

				if ( totalPointsAdded == 2 )
				{
					// If 2 points are already added, then add the third point: a point on the Far Clipping Plane
					// which defines the CClipper2D's Clipping Plane.
					CVector3 clickPointFar = screenToWorldCoords( mx, my, 1 );
					m_algorithmPoints.push_back( clickPointFar );

					// Also, draw a special cursor: a line representing the clipping plane
					double *point1Pos = static_cast<vtkActor2D *>( m_cursorActors[0].GetPointer() )->GetPosition();
					double *point2Pos = static_cast<vtkActor2D *>( m_cursorActors[1].GetPointer() )->GetPosition();

					CVector3 desiredPoint1( point1Pos[0], point1Pos[1], point1Pos[2] );
					CVector3 desiredPoint2( point2Pos[0], point2Pos[1], point2Pos[2] );
					CVector3 planeLineDirection = desiredPoint2 - desiredPoint1;
					planeLineDirection.normalize();

					double lineScale = 1; // this block uses some heuristics to scale the line in a way it will always have its endpoints outside the viewport
					int *viewportSize = renderer->GetSize();
					lineScale += viewportSize[0] * viewportSize[1];

					desiredPoint1 = desiredPoint1 - lineScale * planeLineDirection;
					desiredPoint2 = desiredPoint2 + lineScale * planeLineDirection;

					vtkSmartPointer<vtkLineSource> lineSrc = vtkSmartPointer<vtkLineSource>::New();
					lineSrc->SetPoint1( desiredPoint1.x, desiredPoint1.y, desiredPoint1.z );
					lineSrc->SetPoint2( desiredPoint2.x, desiredPoint2.y, desiredPoint2.z );

					vtkSmartPointer<vtkPolyDataMapper2D> polyMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
					polyMapper->SetInput( lineSrc->GetOutput() );

					vtkSmartPointer<vtkActor2D> cursorActor = vtkSmartPointer<vtkActor2D>::New();
					cursorActor->SetMapper( polyMapper );
					renderer->AddActor2D( cursorActor );

					m_cursorActors.push_back( cursorActor );

                    updateScreen = true;
					std::cout << "Adicionou um ponto FAR ao vetor!" << std::endl;
				}
				else if ( totalPointsAdded > 2 )
				{
					// The user should have only 4 points defined: the first 3 points define the plane's normal, and
					// the 4th point is used to check the side of the plane which will have its voxels hidden.
					assert( totalPointsAdded == 4 );

					std::cout << "Adicionou o ponto de teste ao vetor. O algoritmo vai iniciar!" << std::endl;

					// Calculate the normal of the user-defined Clipping Plane.
					// The first 3 auxiliary points are used.
					CVector3 v1 = m_algorithmPoints[1] - m_algorithmPoints[0];
					CVector3 v2 = m_algorithmPoints[2] - m_algorithmPoints[0];
					CVector3 normal = CVector3::cross( v1, v2 );
					normal.normalize();

					// Use the 4th point as the reference point to tell the clipping algorithm which side of the
					// plane should be hidden.
					CClipper2D &clipper2D = static_cast<CClipper2D &>(*curAlgorithm);
					clipper2D.setConfiguration( normal, m_algorithmPoints[0], m_algorithmPoints.back() );

					// Clear the auxiliary points vector, and start!
					m_algorithmPoints.clear();
					clearCursorActors();
					startAlgorithm = true;
				}


                if ( updateScreen )
                    renderer->GetRenderWindow()->Render();
			}
			break;
		case evAlgorithmPolyClipper2D:
			{
				if ( GetInteractor()->GetControlKey() == 0 )
				{
					// Add the pair of points that will compose the algorithm points
					CVector3 clickPointNear = screenToWorldCoords( mx, my, 0 );
					CVector3 clickPointFar = screenToWorldCoords( mx, my, 1 );
					m_algorithmPoints.push_back( clickPointNear );
					m_algorithmPoints.push_back( clickPointFar );

					// Creates a 2D "circunference"
					vtkSmartPointer<vtkRegularPolygonSource> polySrc = vtkSmartPointer<vtkRegularPolygonSource>::New();
					polySrc->SetRadius( 3 );
					polySrc->SetNumberOfSides(20);

					// Attaches the circunference to a Follower
					vtkSmartPointer<vtkPolyDataMapper2D> polyMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
					polyMapper->SetInput( polySrc->GetOutput() );

					// This actor will be added to be used as a cursor for the user
					vtkSmartPointer<vtkActor2D> cursorActor = vtkSmartPointer<vtkActor2D>::New();
					cursorActor->SetMapper( polyMapper );
					cursorActor->SetPosition( mx, my );
					renderer->AddActor2D( cursorActor );

					if ( m_cursorActors.size() > 3 )   // remove the line that links the last point to the first one
					{
						renderer->RemoveActor2D( m_cursorActors.back() );
						m_cursorActors.pop_back();
					}
					m_cursorActors.push_back( cursorActor );

					// Add a line connecting two points
					vtkSmartPointer<vtkActor2D> lastActor;
					size_t totalAddedActors = m_cursorActors.size();
					if ( totalAddedActors == 2 )
						lastActor = static_cast<vtkActor2D *>( m_cursorActors[0].GetPointer() );
					else if ( totalAddedActors > 2 )
						lastActor = static_cast<vtkActor2D *>( m_cursorActors[totalAddedActors - 3].GetPointer() );

					if ( lastActor.GetPointer() != NULL )
					{
						// Create a line connecting the two points
						vtkSmartPointer<vtkLineSource> lineSrc = vtkSmartPointer<vtkLineSource>::New();
						double *lastPointPos = lastActor->GetPosition();
						lineSrc->SetPoint1( lastPointPos[0], lastPointPos[1], lastPointPos[2] );
						lineSrc->SetPoint2( mx, my, 0 );

						// Create and add the actor for the line
						vtkSmartPointer<vtkPolyDataMapper2D> lineMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
						lineMapper->SetInput( lineSrc->GetOutput() );

						vtkSmartPointer<vtkActor2D> lineActor = vtkSmartPointer<vtkActor2D>::New();
						lineActor->SetMapper( lineMapper );
						renderer->AddActor2D( lineActor );
						m_cursorActors.push_back( lineActor );

					}

					// Add a line that connect the last point to the first one
					if ( m_cursorActors.size() > 3 )
					{
						// Create a line connecting the two points
						vtkSmartPointer<vtkLineSource> lineSrc = vtkSmartPointer<vtkLineSource>::New();
						double *firstPointPos = static_cast<vtkActor2D *>( m_cursorActors[0].GetPointer() )->GetPosition();
						lineSrc->SetPoint1( mx, my, 0 );
						lineSrc->SetPoint2( firstPointPos[0], firstPointPos[1], 0 );

						// Create and add the actor for the line
						vtkSmartPointer<vtkPolyDataMapper2D> lineMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
						lineMapper->SetInput( lineSrc->GetOutput() );

						vtkSmartPointer<vtkActor2D> lineActor = vtkSmartPointer<vtkActor2D>::New();
						lineActor->SetMapper( lineMapper );
						renderer->AddActor2D( lineActor );
						m_cursorActors.push_back( lineActor );
					}

                    // Update screen
                    renderer->GetRenderWindow()->Render();
				}
				else
				{
					CVector3 clickPointNear = screenToWorldCoords( mx, my, 0 );

					CPolyClipper2D &polyClipper = static_cast<CPolyClipper2D &>( *m_curClippingAlgorithm );
					polyClipper.setConfiguration( m_algorithmPoints, clickPointNear );
					startAlgorithm = true;

					m_algorithmPoints.clear();
					clearCursorActors();
				}
			}
			break;
		}
		// Start the clipping algorithm
		if ( startAlgorithm )
			startCurrentClippingAlgorithm();
	}

	// Forward events to superclass
	if ( m_currentAlgorithmId != evAlgorithmEraser2D )
		vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}


/** Starts the currently selected clipping algorithm execution. */
void CVolumeClipperAppInteractorStyle::startCurrentClippingAlgorithm()
{
	// Always pause the WiiMote updating timer, for safety
	CApp &app = wxGetApp();
	bool originalWiiMoteUpdateTimerState = app.isWiiMoteUpdateTimerRunning();
	app.pauseWiiMoteUpdateTimer( true );

	// Calculate the data we need to estimate the positions of each voxel in the 3D world
	double volBounds[6] = {0};
	if ( volume.GetPointer() != NULL )
		volume->GetBounds( volBounds );

	double cellSize[3];      // the size of the cells, in each one of the 3 axes
	double startCoords[3];   // the starting/minimum coordinates of the centers of the first voxels in each axis
	for ( int axis = 0; axis < 3; axis++ )
	{
		const int minBoundIndex = axis * 2;
		const int maxBoundIndex = minBoundIndex + 1;
		cellSize[axis] = (volBounds[maxBoundIndex] - volBounds[minBoundIndex]) / dimensoes[axis];
		startCoords[axis] = volBounds[minBoundIndex] + cellSize[axis] / 2;
	}

	// Open a Progress Dialog to the user
	wxProgressDialog progressDlg( _T("Clipping algorithm"), _T("The volume is being clipped, please wait."),
		dimensoes[2], &app.getMainFrame(), wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME );
	progressDlg.Centre();
	progressDlg.Fit();

	// Calculate the positions of each voxel, and activate the current Clipping Algorithm in each one of them.
	std::cout << "Recortando..." << std::endl;
	CAppConsole &appConsole = CAppConsole::getInstance();
	appConsole.log( _T( "Clipping initialized..." ) );
	int cellIndex = 0;
	clock_t marker = clock();

	double cellZ = startCoords[2];

	for ( int zIndex = 0; zIndex < dimensoes[2]; zIndex++ )
	{
		cellZ += cellSize[2];
		double cellY = startCoords[1];

		std::cout << "   Processando fatia " << (zIndex+1) << " de " << dimensoes[2] << std::endl;
		for ( int yIndex = 0; yIndex < dimensoes[1]; yIndex++ )
		{
			cellY += cellSize[1];
			double cellX = startCoords[0];
			for ( int xIndex = 0; xIndex < dimensoes[0]; xIndex++, cellIndex++ )
			{
				cellX += cellSize[0];

				if ( m_curClippingAlgorithm->hideVoxel( cellX, cellY, cellZ ) )
					imgDataArray->SetValue( cellIndex, 0 );

			}
		}

		// Update the progress dialog
		progressDlg.Update( zIndex );
	}

	double elapsedTime = ((clock() - marker) / static_cast<double>(CLOCKS_PER_SEC));
	appConsole.log( wxString::Format( _T( "Clipping finished. Elapsed time: %.2lf seconds." ), elapsedTime ) );

	std::cout << "Tempo do recorte: " << elapsedTime << " segs" << std::endl;
	if ( volume.GetPointer() != NULL )
		volume->Update();

	// Return the WiiMote updating timer to its original running state
	app.pauseWiiMoteUpdateTimer( !originalWiiMoteUpdateTimerState );

    // Render the scene
    renderer->GetRenderWindow()->Render();
}


/** Utility method which converts a point from screen coordinates to world coordinates.
 * @param sx The X coordinate, on screen coordinates, from the point.
 * @param sy The Y coordinate, on screen coordinates, from the point.
 * @param z Represents the normalized depth of the point, from 0 (near clipping plane) to 1 (far clipping plane).
 * @return Returns a CVector3 object representing the point, given in world coordinates.
 */
const CVector3 CVolumeClipperAppInteractorStyle::screenToWorldCoords( int sx, int sy, double z )
{
	double worldCoords[4];
	renderer->SetDisplayPoint( sx, sy, z );
	renderer->DisplayToWorld();
	renderer->GetWorldPoint( worldCoords );

	return CVector3(worldCoords[0], worldCoords[1], worldCoords[2]);
}


/** Retrieves a cursor Actor object, given its index;
 * @param index The index of the cursor Actor into the Actors vector.
 * @return Returns a pointer to the given cursor Actor. */
CVolumeClipperAppInteractorStyle::TActorSmartPtr CVolumeClipperAppInteractorStyle::getCursorActor( int index ) const
{
	return m_cursorActors[index];
}


/** Utility method for clearing the list of cursor actors. */
void CVolumeClipperAppInteractorStyle::clearCursorActors( void )
{
	const int totalCursorActors = static_cast<int>(m_cursorActors.size());
	for ( int a = 0; a < totalCursorActors; a++ )
	{
		if ( dynamic_cast<vtkActor2D *>(m_cursorActors[a].GetPointer()) )
			renderer->RemoveActor2D( m_cursorActors[a] );
		else
			renderer->RemoveActor( m_cursorActors[a] );
	}
	m_cursorActors.clear();
}


/** Called when the timer for the Render Rate adjusting is set off.
 * This method then adjusts (restores) the rendering rate of the VTK Render Window and asks it to render the
 * scene in a precise way. */
void CVolumeClipperAppInteractorStyle::OnRenderRateAdjustingTimer( wxTimerEvent & )
{
    if ( renderer.GetPointer() == NULL )
        return;

    vtkRenderWindow *renderWnd = renderer->GetRenderWindow();
    if ( renderWnd != NULL )
    {
        renderWnd->SetDesiredUpdateRate( 0 );
        renderWnd->Render();
    }
}


/** Changes the refreshing rate for the VTK rendering to a mode where it can render the volume quickly,
 * by diminishing its level of detail, and start a timer which will be responsible for restoring the
 * level of details of the volume. This method should be called constantly during operations that require
 * intensive rendering, such as repositioning a clipping algorithm's cursor. */
void CVolumeClipperAppInteractorStyle::startRefreshRateChange( int timeToRestoreRate )
{
	// Set the update rate
	vtkRenderWindow *renderWnd = renderer->GetRenderWindow();
	renderWnd->SetDesiredUpdateRate( APPLICATION_DESIRED_FPS );
	renderWnd->Render();

	// Start a timer which will restore the refreshing rate
	m_renderRateAdjustingTimer->Start( timeToRestoreRate, true );
}


/** Sets the object which is responsible for processing an Input Device that is connected to the Application.
 * @param inputDevProc The object which will process an Input Device for the application. */
void CVolumeClipperAppInteractorStyle::setInputDeviceProcessor( TInputDeviceProcessorPtr inputDevProc )
{
	CApp &app = wxGetApp();

	// Removes any listener that was previously installed
	if ( m_inputDeviceProcessor.GetPointer() != NULL )
		app.removeWiiMoteUpdateListener( m_inputDeviceProcessor.GetPointer() );

	// Updates input processor reference and registers it for listening Input Device events
	m_inputDeviceProcessor = inputDevProc;
	if ( m_inputDeviceProcessor.GetPointer() != NULL )
		app.addWiiMoteUpdateListener( m_inputDeviceProcessor.GetPointer() );
}

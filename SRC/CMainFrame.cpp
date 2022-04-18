
#include <windows.h>
#include <NuiApi.h>

// Default includes
#include "CMainFrame.hpp"

#include "CAppConsole.hpp"
#include "CDebugToStdoutRedirector.hpp"
#include "CVolumeClipperAppInteractorStyle.hpp"
#include "CAboutDialog.hpp"
#include "CITKFilterProgressDialogUpdater.hpp"
#include "CPanelEraserToolConfig.hpp"
#include "CWiiMoteInputProcessor.hpp"
#include "CKinectInputProcessor.hpp"

#include "wxVTKRenderWindowInteractor.h"

#include "clippingAlgorithms.hpp"
#include "inputDevices.hpp"

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkImageShiftScale.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkPointData.h>
#include <vtkCamera.h>
#include <vtkRegularPolygonSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRendererCollection.h>
#include <vtkCallbackCommand.h>

#include <itkDICOMSeriesFileNames.h>
#include <itkImageSeriesReader.h>
#include <itkImage.h>
#include <itkGDCMImageIO.h>
#include <itkImageToVTKImageFilter.h>

#include "CKinectRenderer.hpp"

#include <wx/wx.h>
#include <wx/tglbtn.h>
#include <wx/progdlg.h>
#include <wx/msgdlg.h>
#include <wx/xrc/xmlres.h>

#include "definicoesLocais.hpp"
#include "CApp.hpp"

#include <vector>


// CONSTANTS
static const char *MAIN_FRAME_NAME = "MainFrame";
static const char *MENU_ITEM_OPEN_DICOM_DIR = "menuItemOpenDicomDirectory";
static const char *MENU_ITEM_EXIT = "menuItemExit";
static const char *MENU_ITEM_ABOUT = "menuItemAbout";
static const char *MENU_ITEM_KINECT_RENDERING_FULL = "menuItemKinectRenderingFull";
static const char *MENU_ITEM_KINECT_RENDERING_USER_AND_HANDS = "menuItemKinectRenderingUserAndHands";
static const char *MENU_ITEM_KINECT_RENDERING_HANDS_ONLY = "menuItemKinectRenderingHandsOnly";
static const char *MENU_ITEM_KINECT_SIMULATION_MODE = "menuItemKinectSimulationMode";
static const char *MENU_ITEM_KINECT_SMOOTHING = "menuItemKinectSmoothMovement";
static const char *TOOLBAR_ITEM_VISUALIZE = "m_toolVisualize";
static const char *TOOLBAR_ITEM_ERASER = "m_toolEraser2D";
static const char *TOOLBAR_ITEM_CLIPPER = "m_toolClipper2D";
static const char *TOOLBAR_ITEM_POLYCLIPPER2D = "m_toolPolyClipper2D";
static const char *WIDGET_VTK_PANEL = "m_panelVTKWidget";
static const char *TOGGLEBUTTON_ENABLE_WIIMOTE = "m_tglbtEnableWiiMote";
static const char *LABEL_WIIMOTE_STATUS = "m_lblWiiMoteStatus";
static const char *LABEL_WIIMOTE_ID = "m_lblId";
static const char *LABEL_WIIMOTE_BATTERY = "m_lblBattery";
static const char *LABEL_WIIMOTE_LEDS = "m_lblLEDs";
static const char *LABEL_WIIMOTE_BUTTONS = "m_lblButtons";
static const char *LABEL_WIIMOTE_ACCELX = "m_lblAccelX";
static const char *LABEL_WIIMOTE_ACCELY = "m_lblAccelY";
static const char *LABEL_WIIMOTE_ACCELZ = "m_lblAccelZ";
static const char *LABEL_WIIMOTE_ORIENTX = "m_lblOrientX";
static const char *LABEL_WIIMOTE_ORIENTY = "m_lblOrientY";
static const char *LABEL_WIIMOTE_ORIENTZ = "m_lblOrientZ";
static const char *LABEL_WIIMOTE_PITCH = "m_lblPitch";
static const char *LABEL_WIIMOTE_ROLL = "m_lblRoll";
static const char *LABEL_WII_MOTION_PLUS_STATE = "m_lblMotionPlusState";
static const char *LABEL_WII_MOTION_PLUS_YAW = "m_lblMotionPlusYaw";
static const char *LABEL_WII_MOTION_PLUS_PITCH = "m_lblMotionPlusPitch";
static const char *LABEL_WII_MOTION_PLUS_ROLL = "m_lblMotionPlusRoll";
static const char *LABEL_SELECTED_TOOL_NAME = "m_lblToolName";
static const char *LABEL_KINECT_STATUS = "m_lblKinectStatus";
static const char *LABEL_KINECT_ELEVATION_ANGLE = "m_lblElevationAngleValue";
static const char *BUTTON_CLEAR_APP_CONSOLE = "m_btConsoleClear";
static const char *BUTTON_CONNECT_WIIMOTE = "m_bpbtConnectWiiMote";
static const char *BUTTON_CONNECT_KINECT = "m_bpbtConnectKinect";
static const char *PANEL_WIIMOTE_CONNECTION_STATUS = "m_panelWiiMoteConnectionStatus";
static const char *PANEL_TOOL_CONFIG = "m_panelToolConfig";
static const char *PANEL_KINECT_RENDERER = "m_kinectDrawPanel";
static const char *SCROLLEDPANEL_SIDE_PANEL = "m_scrolledSidePanel";
static const char *CHECKBOX_ENABLE_MOTION_PLUS = "m_chkUseMotionPlus";
static const char *SLIDER_WIIMOTE_SENSIBILITY = "m_sliderWiimoteSensibility";
static const char *SLIDER_KINECT_ELEVATION_ANGLE = "m_sliderKinectElevationAngle";
static const char *TEXT_CTRL_APP_CONSOLE = "m_txtConsole";

static const wxString STR_INPUT_DEVICE_CONNECTED(_T("[CONNECTED]"));
static const wxString STR_INPUT_DEVICE_DISCONNECTED(_T("[DISCONNECTED]"));
static const wxString STR_INPUT_DEVICE_SIMULATING(_T("[SIMULATING]"));
static const wxString STR_UNKNOWN(_T("?"));
static const wxString STR_NONE(_T("None"));
static const wxString IMGPATH_WIIMOTE_CONNECTED(_T("MEDIA/wiimoteConnected.png"));
static const wxString IMGPATH_WIIMOTE_DISCONNECTED(_T("MEDIA/wiimoteDisconnected.png"));
static const wxString IMGPATH_KINECT_CONNECTED(_T("MEDIA/kinectConnected.png"));
static const wxString IMGPATH_KINECT_DISCONNECTED(_T("MEDIA/kinectDisconnected.png"));
static const wxString PANEL_NO_TOOL_CONFIGURATIONS_NAME(_T("PanelToolNoConfig"));

static const wxString TOOL_NAME_CLIPPER2D( _T("Clipper 2D") );
static const wxString TOOL_NAME_CLIPPER3D( _T("Clipper 3D") );
static const wxString TOOL_NAME_POLYCLIPPER2D( _T("Poly Clipper 2D") );
static const wxString TOOL_NAME_ERASER2D( _T("Eraser 2D") );
static const wxString TOOL_NAME_ERASER3D( _T("Eraser 3D") );
static const wxString TOOL_NAME_NONE( _T("None") );


// wxWidgets event table
BEGIN_EVENT_TABLE( CMainFrame, wxFrame )
	EVT_CLOSE( OnClose )
	EVT_BUTTON( XRCID(BUTTON_CLEAR_APP_CONSOLE), OnButtonClearAppConsole )
	EVT_BUTTON( XRCID(BUTTON_CONNECT_WIIMOTE), OnButtonConnectWiiMote )
	EVT_BUTTON( XRCID(BUTTON_CONNECT_KINECT), OnButtonConnectKinect )
	EVT_MENU( XRCID(MENU_ITEM_OPEN_DICOM_DIR), OnMenuItemOpenDICOMDirectory )
	EVT_MENU( XRCID(MENU_ITEM_EXIT), OnMenuItemExit )
	EVT_MENU( XRCID(MENU_ITEM_ABOUT), OnMenuItemAbout )
	EVT_MENU( XRCID(MENU_ITEM_KINECT_RENDERING_FULL), OnMenuItemKinectRenderingRadio )
	EVT_MENU( XRCID(MENU_ITEM_KINECT_RENDERING_USER_AND_HANDS), OnMenuItemKinectRenderingRadio )
	EVT_MENU( XRCID(MENU_ITEM_KINECT_RENDERING_HANDS_ONLY), OnMenuItemKinectRenderingRadio )
	EVT_MENU( XRCID(MENU_ITEM_KINECT_SIMULATION_MODE), OnMenuItemKinectSimulationMode )
	EVT_MENU( XRCID(MENU_ITEM_KINECT_SMOOTHING), OnMenuItemKinectSmoothing )
	EVT_TOOL( XRCID(TOOLBAR_ITEM_VISUALIZE), OnToolBarAlgorithmSelected )
	EVT_TOOL( XRCID(TOOLBAR_ITEM_ERASER), OnToolBarAlgorithmSelected )
	EVT_TOOL( XRCID(TOOLBAR_ITEM_CLIPPER), OnToolBarAlgorithmSelected )
	EVT_TOOL( XRCID(TOOLBAR_ITEM_POLYCLIPPER2D), OnToolBarAlgorithmSelected )
	EVT_TOGGLEBUTTON( XRCID(TOGGLEBUTTON_ENABLE_WIIMOTE), OnToggleButtonEnable3DTools )
	EVT_CHECKBOX( XRCID(CHECKBOX_ENABLE_MOTION_PLUS), OnCheckboxEnableWiiMotionPlus )
	EVT_SLIDER( XRCID(SLIDER_KINECT_ELEVATION_ANGLE), OnSliderKinectElevationScrolled )
END_EVENT_TABLE()

// METHODS
/** Constructor. */
CMainFrame::CMainFrame()
	: wxFrame(),
	m_pVTKWindow(NULL),
	m_interactorStyle(),
	m_renderer()
{
	// Loads the main frame GUI from a XRC file
	wxXmlResource &resources = *wxXmlResource::Get();
	bool loadFrameResult = resources.LoadFrame( this, NULL, MAIN_FRAME_NAME );
	assert( loadFrameResult );

	// Initializes the App Console
	wxTextCtrl *txtAppLog = static_cast<wxTextCtrl *>( wxWindow::FindWindowById( XRCID(TEXT_CTRL_APP_CONSOLE), this ) );
	CAppConsole::initialize( txtAppLog );

	CAppConsole &console = CAppConsole::getInstance();
	console.log( _T( "Console has been initialized." ) );
	console.log( _T( "Application's ready!" ) );

	// Configures the scrolled side panel of the GUI
	resetWiiMoteDisplayedData();

	wxScrolledWindow *scrolledSidePanel = static_cast<wxScrolledWindow *>( wxWindow::FindWindowById( XRCID(SCROLLEDPANEL_SIDE_PANEL), this ) );
	scrolledSidePanel->SetScrollRate( 10, 10 );

	// Creates VTK widget
	wxWindow *panelForVTKWidget = wxWindow::FindWindowById( XRCID(WIDGET_VTK_PANEL), this );

	m_pVTKWindow = new wxVTKRenderWindowInteractor( panelForVTKWidget, wxID_ANY );
	panelForVTKWidget->GetSizer()->Add( m_pVTKWindow, 1, wxEXPAND );

	m_pVTKWindow->wxWindowBase::SetSize( panelForVTKWidget->GetSize() );

	// Configures the VTK widget
	m_renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWnd = m_pVTKWindow->GetRenderWindow();

	renderWnd->AddRenderer( m_renderer );

	// Initialize the class that receives debug messages
	vtkSmartPointer<CDebugToStdoutRedirector> dbgObj = vtkSmartPointer<CDebugToStdoutRedirector>::New();
	vtkOutputWindow::SetInstance( dbgObj );

	// Creates our interactor style
	m_interactorStyle = vtkSmartPointer<CVolumeClipperAppInteractorStyle>::New();
	m_pVTKWindow->SetInteractorStyle( m_interactorStyle );
	m_interactorStyle->AutoAdjustCameraClippingRangeOff();
	m_interactorStyle->renderer = m_renderer;

	if ( DICOM_PATH_STR.empty() == false )
		loadDICOMSeriesFromDirectory( DICOM_PATH_STR );

	// Add self as a listener for WiiMote updating events
	CApp &app = wxGetApp();
	app.addWiiMoteUpdateListener( this );

	// Update the tool's configuration panel
	updateToolConfigurationPanel();

	// Enables Kinect rendering panel
	wxPanel *kinectDrawPanel = static_cast<wxScrolledWindow *>( wxWindow::FindWindowById( XRCID(PANEL_KINECT_RENDERER), this ) );
	m_kinectRenderer.reset( new CKinectRenderer( kinectDrawPanel ) );

	this->SetSize( 892, 652 );
}


/** Destructor. */
CMainFrame::~CMainFrame()
{
	// Remove self from the list of listeners of WiiMote updating events
	CApp &app = wxGetApp();
	bool removeResult = app.removeWiiMoteUpdateListener( this );
	assert(removeResult);

	// Delete the VTK window
	m_pVTKWindow->Delete();
}


/** Loads a series of DICOM files from the given directory.
 * @param dicomDir The directory from where the DICOM files will be loaded.
 * @return Returns true in case of success, false otherwise.
 */
bool CMainFrame::loadDICOMSeriesFromDirectory( const std::string &dicomDir )
{
	// Clear any previous volume data
	if ( m_interactorStyle->volume.GetPointer() != NULL )
		m_renderer->RemoveVolume( m_interactorStyle->volume );

	// Keep current time, for measuring total ellapsed time
	std::cout << "Carregando serie DICOM: " << dicomDir << std::endl;
	clock_t appStartTime = clock();

	// Look into the given directory for DICOM images, and sort them by their image (slice) numbers.
	typedef itk::DICOMSeriesFileNames DICOMSeriesFileNames;
	DICOMSeriesFileNames::Pointer dicomSeriesFileNames = DICOMSeriesFileNames::New();
	dicomSeriesFileNames->SetDirectory(dicomDir);
	dicomSeriesFileNames->SetFileNameSortingOrderToSortByImageNumber();

	// Opens a progress dialog to tell the user that the DICOM files are being read
	wxString progressMsg = _T("Please wait while the DICOM files are being read.\nLoading: ");
	progressMsg.append( dicomDir );
	wxProgressDialog progressDlg( _T("Reading DICOM files..."),
		progressMsg, 100, this,
		wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_AUTO_HIDE );
	progressDlg.Centre();
	progressDlg.Fit();

	// Reads the sorted DICOM image series
	typedef itk::Image<short, 3> ITKImage3D;
	typedef itk::ImageSeriesReader<ITKImage3D> SeriesReaderType;
	SeriesReaderType::Pointer itkReader = SeriesReaderType::New();
	itkReader->SetFileNames(dicomSeriesFileNames->GetFileNames());

	typedef itk::GDCMImageIO ImageIOType; // trocar
	ImageIOType::Pointer itkDicomIO = ImageIOType::New();
	itkReader->SetImageIO(itkDicomIO);

	CITKFilterProgressDialogUpdater::Pointer updaterCmd = CITKFilterProgressDialogUpdater::New();
	static_cast<CITKFilterProgressDialogUpdater *>(updaterCmd.GetPointer())->setObserverConfiguration( progressDlg, *itkReader );
	itkReader->AddObserver(itk::ProgressEvent(), updaterCmd);

	try 
	{
		std::cout << "Iniciando leitura..." << std::endl;
		itkReader->Update();
		std::cout << "Leitura concluida!" << std::endl;
	}
	catch (itk::ExceptionObject &ex) 
	{
		std::cerr << "EXCEPTION: " << ex.GetDescription() << std::endl;
		return false;
	}

	progressDlg.Close();

	// Get loaded image and set its origin to the world's origin
	ITKImage3D::Pointer itkImage = itkReader->GetOutput();

	double imgOrigin[4] = { 0, 0, 0, 1 };
	itkImage->SetOrigin( imgOrigin );

	// Create a connector that can convert the DICOM series that was read by ITK to VTK format
	typedef itk::ImageToVTKImageFilter<ITKImage3D> ConnectorType;
	ConnectorType::Pointer connector = ConnectorType::New();
	connector->SetInput(itkImage);
	connector->Update();

	// Retrieve the scalar range (minimum and maximum values) from the image data
	vtkSmartPointer<vtkImageData> loadedImgData = connector->GetOutput();

	double originalImgDataRange[2];
	loadedImgData->GetScalarRange( originalImgDataRange );

	// Calculate the shift to be aplied to the image values.
	// The following calculation makes the image values start at 0.
	const double rangeShift = 0 - originalImgDataRange[0];
	const double shiftedImgDataRange[2] = { originalImgDataRange[0] + rangeShift, originalImgDataRange[1] + rangeShift };
	assert( shiftedImgDataRange[0] >= 0 );        // ensures the shifted image values start at 0 (using
	assert( shiftedImgDataRange[0] <= 0.0001 );   // simple double comparation by proximity)


	// Create the color transfer function to be used by the volume (based on ESTIMATE VALUES)
	vtkSmartPointer<vtkColorTransferFunction> cTFun = vtkSmartPointer<vtkColorTransferFunction>::New();

	double table[256][3];
	for(int i = 0; i < 256; i++)
	{
		if ( i > 200 )   /* estimate for RED TISSUE. */
		{
			table[i][0] = 1;
			table[i][1] = 0;
			table[i][2] = 0;
		}
		else if ( i > 100 )   /* estimate for BONES. */
		{
			table[i][0] = 1;
			table[i][1] = 1;
			table[i][2] = 1;
		}
		else if ( i > 25 )   /* estimate for SKIN. */
		{
			table[i][0] = 1;
			table[i][1] = 0.84;
			table[i][2] = 0.45;
		}
		else   /* any other stuff. */
		{
			table[i][0] = 0;
			table[i][1] = 0;
			table[i][2] = 1;
		}
	}
	table[0][0] = table[0][1] = table[0][2] = 0;

	cTFun->BuildFunctionFromTable( shiftedImgDataRange[0], shiftedImgDataRange[1], 255, &table[0][0] );

	vtkSmartPointer<vtkPiecewiseFunction> goTFun = vtkSmartPointer<vtkPiecewiseFunction>::New();
	goTFun->AddPoint( 0, 0.0 );
	goTFun->AddPoint( 255, 0.7 );

	vtkSmartPointer<vtkPiecewiseFunction> oTFun = vtkSmartPointer<vtkPiecewiseFunction>::New();
	oTFun->AddPoint(shiftedImgDataRange[0], 0);
	oTFun->AddPoint(shiftedImgDataRange[1], 0.7);


	// Configure the properties of the volume we are going to render
	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->SetColor(cTFun);
	volumeProperty->SetGradientOpacity(goTFun);
	volumeProperty->SetScalarOpacity(oTFun);
	volumeProperty->ShadeOn();
	volumeProperty->SetInterpolationTypeToLinear();

	// We must to shift the intensity values forward, because we cannot render negative values
	vtkSmartPointer<vtkImageShiftScale> shiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
	shiftScale->SetInput( loadedImgData );
	shiftScale->SetScale(1);
	shiftScale->SetShift(0 - originalImgDataRange[0]);
	shiftScale->ClampOverflowOn();
	shiftScale->SetOutputScalarTypeToUnsignedShort();
	shiftScale->Update();


	// Creates the volume mapper
	vtkSmartPointer<vtkVolumeRayCastMapper> volMapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
	vtkSmartPointer<vtkVolumeRayCastCompositeFunction> compositeFun = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();
	volMapper->SetVolumeRayCastFunction( compositeFun );
	volMapper->SetInputConnection(shiftScale->GetOutputPort());

	// Creates the volume to be rendered
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetProperty(volumeProperty);
	volume->SetMapper(volMapper);

	// Adds our volume to the renderer
	m_renderer->AddVolume(volume);

	// Retrieve the data we will need from the volume, update data references
	vtkSmartPointer<vtkImageData> imgData = shiftScale->GetOutput();
	vtkSmartPointer<vtkPointData> pointData = imgData->GetPointData();
	vtkSmartPointer<vtkUnsignedShortArray> shortArray = dynamic_cast<vtkUnsignedShortArray *>( pointData->GetScalars() );

	imgData->GetDimensions(m_interactorStyle->dimensoes);
	m_interactorStyle->volume = volume;
	m_interactorStyle->imgDataArray = shortArray;

	// Configures and renderizes an initial frame for the scene.
	// Apparently, VTK initializes some things during this first renderization.
	m_renderer->ResetCamera();
	m_renderer->Render();

	// Setup the camera
	vtkSmartPointer<vtkCamera> cam = m_renderer->GetActiveCamera();
	cam->SetClippingRange( 1, 10000 );

	std::cout << "Tempo total para inicializar: " << ((clock()-appStartTime) / static_cast<float>(CLOCKS_PER_SEC)) << " segundos" << std::endl;

	return true;
}


/** Sets all of the data displayed by the WiiMote to an unknown/disconnected state. */
void CMainFrame::resetWiiMoteDisplayedData( void )
{
	// Update other labels, upon disconnection
	wxStaticText *lblId = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ID), this ) );
	wxStaticText *lblBattery = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_BATTERY), this ) );
	wxStaticText *lblLeds = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_LEDS), this ) );
	wxStaticText *lblButtons = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_BUTTONS), this ) );
	wxStaticText *lblAccelX = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ACCELX), this ) );
	wxStaticText *lblAccelY = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ACCELY), this ) );
	wxStaticText *lblAccelZ = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ACCELZ), this ) );
	wxStaticText *lblOrientX = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ORIENTX), this ) );
	wxStaticText *lblOrientY = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ORIENTY), this ) );
	wxStaticText *lblOrientZ = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ORIENTZ), this ) );
	wxStaticText *lblPitch = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_PITCH), this ) );
	wxStaticText *lblRoll = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ROLL), this ) );
	wxStaticText *lblMotionPlusState = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WII_MOTION_PLUS_STATE), this ) );
	wxStaticText *lblMotionPlusYaw = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WII_MOTION_PLUS_YAW), this ) );
	wxStaticText *lblMotionPlusPitch = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WII_MOTION_PLUS_PITCH), this ) );
	wxStaticText *lblMotionPlusRoll = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WII_MOTION_PLUS_ROLL), this ) );

	lblId->SetLabel( STR_UNKNOWN );
	lblBattery->SetLabel( STR_UNKNOWN );
	lblLeds->SetLabel( STR_UNKNOWN );
	lblButtons->SetLabel( STR_UNKNOWN );
	lblAccelX->SetLabel( STR_UNKNOWN );
	lblAccelY->SetLabel( STR_UNKNOWN );
	lblAccelZ->SetLabel( STR_UNKNOWN );
	lblOrientX->SetLabel( STR_UNKNOWN );
	lblOrientY->SetLabel( STR_UNKNOWN );
	lblOrientZ->SetLabel( STR_UNKNOWN );
	lblPitch->SetLabel( STR_UNKNOWN );
	lblRoll->SetLabel( STR_UNKNOWN );
	lblMotionPlusState->SetLabel( STR_INPUT_DEVICE_DISCONNECTED );
	lblMotionPlusYaw->SetLabel( STR_UNKNOWN );
	lblMotionPlusPitch->SetLabel( STR_UNKNOWN );
	lblMotionPlusRoll->SetLabel( STR_UNKNOWN );
}


/** Called when the user clicks the Exit menu item.
 * @param evt An object containing data about the event.
 */
void CMainFrame::OnMenuItemExit( wxCommandEvent &evt )
{
	this->Close();
}


/** Called when the user clicks the About menu item.
 * @param evt An object containing data about the event.
 */
void CMainFrame::OnMenuItemAbout( wxCommandEvent &evt )
{
	std::auto_ptr<CAboutDialog> aboutDlg( new CAboutDialog( this, true ) );
	aboutDlg->ShowModal();
}


/** Called when the user clicks the "Open DICOM Directory" menu item.
 * @param evt An object containing data about the event.
 */
void CMainFrame::OnMenuItemOpenDICOMDirectory( wxCommandEvent &evt )
{
	// Ask the user to select a directory for the DICOM files
	std::auto_ptr<wxDirDialog> dirSelector( new wxDirDialog( this,
		_T("Select the directory containing the DICOM files"), wxEmptyString,
		wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST ) );
	if ( dirSelector->ShowModal() != wxID_OK )
		return;

	// Try to read the DICOM series from the selected directory
	std::string dicomDir = dirSelector->GetPath();
	if ( loadDICOMSeriesFromDirectory( dicomDir ) == false )
	{
		wxMessageBox( _T("Could not load DICOM series from the specified directory!"), _T("Loading error"),
			wxOK | wxICON_ERROR, this );
		return;
	}
    else
        m_renderer->GetRenderWindow()->Render();
}


/** Called when the user clicks the "Kinect Rendering" menu items.
 * @param evt An object containing data about the event. */
void CMainFrame::OnMenuItemKinectRenderingRadio( wxCommandEvent &evt )
{
	// Verify which rendering style is selected
	wxMenuBar *menuBar = this->GetMenuBar();
	assert( menuBar != NULL );

	if ( menuBar->IsChecked( XRCID( MENU_ITEM_KINECT_RENDERING_FULL ) ) )
		m_kinectRenderer->setRenderingType( CKinectRenderer::evRenderingTypeFull );
	else if ( menuBar->IsChecked( XRCID( MENU_ITEM_KINECT_RENDERING_HANDS_ONLY ) ) )
		m_kinectRenderer->setRenderingType( CKinectRenderer::evRenderingTypeHandsOnly );
	else if ( menuBar->IsChecked( XRCID( MENU_ITEM_KINECT_RENDERING_USER_AND_HANDS ) ) )
		m_kinectRenderer->setRenderingType( CKinectRenderer::evRenderingTypeUserAndHands );
}


/** Called when the user clicks the "Kinect Smulation" menu item.
 * @param evt An object containing data about the event. */
void CMainFrame::OnMenuItemKinectSimulationMode( wxCommandEvent &evt )
{
	// Verify if Kinect simulation has been enabled
	wxMenuBar *menuBar = this->GetMenuBar();
	assert( menuBar != NULL );

	bool bEnableKinectSimulation = menuBar->IsChecked( XRCID( MENU_ITEM_KINECT_SIMULATION_MODE ) );
	
	CApp &app = wxGetApp();
	app.setKinectSimulationMode( bEnableKinectSimulation );
}


/** Called when the user clicks the "Kinect Smoothing" menu item.
 * @param evt An object containing data about the event. */
void CMainFrame::OnMenuItemKinectSmoothing( wxCommandEvent &evt )
{
	// Verify if Kinect Smoothing has been enabled
	wxMenuBar *menuBar = this->GetMenuBar();
	assert( menuBar != NULL );

	bool bEnableKinectSmoothing = menuBar->IsChecked( XRCID( MENU_ITEM_KINECT_SMOOTHING ) );
	
	CApp &app = wxGetApp();
	app.setKinectSmoothMovement( bEnableKinectSmoothing );
}


/** Called when the user selects a Clipping Algorithm (or the Visualization Mode Tool) from the toolbar.
 * @param evt An object containing data about the event.
 */
void CMainFrame::OnToolBarAlgorithmSelected( wxCommandEvent &evt )
{
	int evtId = evt.GetId();

	// This will be used to verify if the selected algorithm is 2D or 3D
	wxToggleButton *toggleButtonEnableWiiMote = static_cast<wxToggleButton *>( wxWindow::FindWindowById( XRCID(TOGGLEBUTTON_ENABLE_WIIMOTE), this ) );
	bool bEnableWiiMote = toggleButtonEnableWiiMote->GetValue();

	// Verify the selected algorithm
	vtkSmartPointer<CVolumeClipperAppInteractorStyle> interactorStyle = static_cast<CVolumeClipperAppInteractorStyle *>( m_pVTKWindow->GetInteractorStyle() );

	EClippingAlgorithm clippingAlgorithm = evAlgorithmNone;
	if ( evtId == XRCID(TOOLBAR_ITEM_VISUALIZE) )
		clippingAlgorithm = evAlgorithmNone;
	else if ( evtId == XRCID(TOOLBAR_ITEM_ERASER) )
		clippingAlgorithm = bEnableWiiMote ? evAlgorithmEraser3D : evAlgorithmEraser2D;
	else if ( evtId == XRCID(TOOLBAR_ITEM_CLIPPER) )
		clippingAlgorithm = bEnableWiiMote ? evAlgorithmClipper3D : evAlgorithmClipper2D;
	else if ( evtId == XRCID(TOOLBAR_ITEM_POLYCLIPPER2D) )
		clippingAlgorithm = evAlgorithmPolyClipper2D;
	else
		assert( false );   /* we're trying to process a tool that has not been implemented yet! */

	updateSelectedClippingAlgorithm( clippingAlgorithm );
}


void CMainFrame::updateSelectedClippingAlgorithm( EClippingAlgorithm algorithmId )
{
	// Set the current clipping algorithm
	vtkSmartPointer<CVolumeClipperAppInteractorStyle> interactorStyle = static_cast<CVolumeClipperAppInteractorStyle *>( m_pVTKWindow->GetInteractorStyle() );
	interactorStyle->setClippingAlgorithm( algorithmId );

	// Guarantees that the tool is selected on the toolbar
	int toolWidgetId = wxID_ANY;
	switch( algorithmId )
	{
	case evAlgorithmNone:
		toolWidgetId = XRCID( TOOLBAR_ITEM_VISUALIZE );
		break;
	case evAlgorithmEraser2D:
	case evAlgorithmEraser3D:
		toolWidgetId = XRCID( TOOLBAR_ITEM_ERASER );
		break;
	case evAlgorithmClipper2D:
	case evAlgorithmClipper3D:
		toolWidgetId = XRCID( TOOLBAR_ITEM_CLIPPER );
		break;
	case evAlgorithmPolyClipper2D:
		toolWidgetId = XRCID( TOOLBAR_ITEM_POLYCLIPPER2D );
		break;
	default:
		// we're trying to process a tool that has not been implemented yet!
		assert( false );
	}

	this->GetToolBar()->ToggleTool( toolWidgetId, true );

	// Update Tool config panel
	updateToolConfigurationPanel();
}

void CMainFrame::updateToolConfigurationPanel( void )
{
	// Destroy any previous Tool Configuration Panel
	wxPanel *toolConfigPanelPlace = static_cast<wxPanel *>( wxWindow::FindWindowById( XRCID( PANEL_TOOL_CONFIG ), this ) );
	toolConfigPanelPlace->DestroyChildren();

	// Verify which configuration panel to load
	wxPanel *cfgPanel = NULL;
	vtkSmartPointer<CVolumeClipperAppInteractorStyle> interactorStyle = static_cast<CVolumeClipperAppInteractorStyle *>( m_pVTKWindow->GetInteractorStyle() );

	wxString toolName( _T(TOOL_NAME_NONE) );
	const int curClippingAlgorithmId = interactorStyle->getClippingAlgorithmId();
	switch ( curClippingAlgorithmId )
	{
	case evAlgorithmClipper2D:
		toolName = TOOL_NAME_CLIPPER2D;
		break;
	case evAlgorithmClipper3D:
		toolName = TOOL_NAME_CLIPPER3D;
		break;
	case evAlgorithmPolyClipper2D:
		toolName = TOOL_NAME_POLYCLIPPER2D;
		break;
	case evAlgorithmEraser2D:
	case evAlgorithmEraser3D:
		toolName = ( curClippingAlgorithmId == evAlgorithmEraser2D ? TOOL_NAME_ERASER2D : TOOL_NAME_ERASER3D );
		cfgPanel = new CPanelEraserToolConfig( toolConfigPanelPlace );
		break;
	}

	if ( cfgPanel == NULL )
	{
		wxXmlResource &resources = *wxXmlResource::Get();
		cfgPanel = resources.LoadPanel( toolConfigPanelPlace, PANEL_NO_TOOL_CONFIGURATIONS_NAME );
	}

	// Update loaded panel's GUI
	toolConfigPanelPlace->GetSizer()->Add( cfgPanel, 1, wxEXPAND );
	cfgPanel->Fit();

	// Update tool's name
	wxStaticText *lblToolName = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID( LABEL_SELECTED_TOOL_NAME ), this ) );
	lblToolName->SetLabel( toolName );

	// Load the selected tool's configuration panel
	//toolConfigPanelPlace->GetSizer()->Add( cfgPanel, 1, wxEXPAND );
	cfgPanel->Fit();
}


/** Called when a WiiMote device is connected to the application. */
void CMainFrame::wiiMoteConnected( bool bConnected )
{
	CApp &app = wxGetApp();

	// Sets "connected"/"disconnected" labels
	wxStaticText *lblStatus = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_STATUS), this ) );
	lblStatus->SetLabel( bConnected ? STR_INPUT_DEVICE_CONNECTED : STR_INPUT_DEVICE_DISCONNECTED );

	// Update the WiiMote connection button image
	wxBitmapButton *bmpbtConnectWiiMote = static_cast<wxBitmapButton *>( wxWindow::FindWindowById( XRCID(BUTTON_CONNECT_WIIMOTE), this ) );
	wxImage imgButton( bConnected ? IMGPATH_WIIMOTE_CONNECTED : IMGPATH_WIIMOTE_DISCONNECTED );
	wxBitmap bmpButton( imgButton );
	bmpbtConnectWiiMote->SetBitmapLabel( bmpButton );

	// Update the displayed identifier of the WiiMote
	wxStaticText *lblId = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ID), this ) );
	if ( bConnected )
	{
		wiimote &remote = app.getWiiMote();

		lblId->SetLabel( wxString::Format( _T("%I64u"), remote.UniqueID ) );
	}
	else
		resetWiiMoteDisplayedData();

	// Update the sizes of some panels
	wxPanel *panelWiiMoteConnectionStatus = static_cast<wxPanel *>( wxWindow::FindWindowById( XRCID(PANEL_WIIMOTE_CONNECTION_STATUS), this ) );
	panelWiiMoteConnectionStatus->Fit();

	wxScrolledWindow *scrolledSidePanel = static_cast<wxScrolledWindow *>( wxWindow::FindWindowById( XRCID(SCROLLEDPANEL_SIDE_PANEL), this ) );
	scrolledSidePanel->FitInside();
}


/** Called when a Kinect sensor is connected to the application. */
void CMainFrame::kinectConnected( bool bConnected )
{
	CApp &app = wxGetApp();

	// Sets "connected"/"disconnected" labels
	wxStaticText *lblStatus = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_KINECT_STATUS), this ) );

	wxString connectionStatusLabelStr;
	if ( bConnected )
	{
		if ( app.isKinectSimulationModeEnabled() )
			connectionStatusLabelStr = STR_INPUT_DEVICE_SIMULATING;
		else
			connectionStatusLabelStr = STR_INPUT_DEVICE_CONNECTED;
	}
	else
		connectionStatusLabelStr = STR_INPUT_DEVICE_DISCONNECTED;
	lblStatus->SetLabel( connectionStatusLabelStr );

	// Update the Kinect connection button image
	wxBitmapButton *bmpbtConnectKinect = static_cast<wxBitmapButton *>( wxWindow::FindWindowById( XRCID(BUTTON_CONNECT_KINECT), this ) );
	wxImage imgButton( bConnected ? IMGPATH_KINECT_CONNECTED : IMGPATH_KINECT_DISCONNECTED );
	wxBitmap bmpButton( imgButton );
	bmpbtConnectKinect->SetBitmapLabel( bmpButton );

	// Update Kinect data
	if ( bConnected )
	{

		// Update elevation angle slider
		wxSlider *sldElevationAngle = static_cast<wxSlider *>( wxWindow::FindWindowById( XRCID(SLIDER_KINECT_ELEVATION_ANGLE), this ) );
		wxStaticText *lblElevationAngle = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_KINECT_ELEVATION_ANGLE), this ) );

		sldElevationAngle->SetRange( static_cast<int>( NUI_CAMERA_ELEVATION_MINIMUM ), static_cast<int>( NUI_CAMERA_ELEVATION_MAXIMUM ) );

		if ( app.isKinectSimulationModeEnabled() == false )
		{
			LONG camElevation;
			INuiSensor &kinect = app.getKinect();
			if ( kinect.NuiCameraElevationGetAngle( &camElevation ) != S_OK )
			{
				sldElevationAngle->SetValue( 0 );
				lblElevationAngle->SetLabel( _T( "??" ) );
			}
			else
			{
				sldElevationAngle->SetValue( static_cast<int>( camElevation ) );
				lblElevationAngle->SetLabel( wxString::Format( _T( "%d" ), static_cast<int>( camElevation ) ) );
			}
		}
		else
		{
			sldElevationAngle->SetValue( 0 );
			lblElevationAngle->SetLabel( _T( "0" ) );
		}
	}
}


/** Asks for the Kinect render panel to be refreshed.
 * The Kinect render panel is where the Kinect Depth data is displayed to the user. */
void CMainFrame::refreshKinectRenderPanel( int imgWidth, int imgHeight, const void * const depthData )
{
	m_kinectRenderer->copyDepthDataToBuffer( imgWidth, imgHeight, depthData );
}


/** Retrieves the Kinect Renderer, which handles the panel where the Kinect Sensor data is rendered to the user.
 * @return Returns a reference to the Kinect Renderer. */
CKinectRenderer &CMainFrame::getKinectRenderer( void ) const
{
	return *m_kinectRenderer;
}


/* Called when the user clicks the "Enable WiiMote" Toggle Button. */
void CMainFrame::OnToggleButtonEnable3DTools( wxCommandEvent &evt )
{
	vtkSmartPointer<CVolumeClipperAppInteractorStyle> interactorStyle = static_cast<CVolumeClipperAppInteractorStyle *>( m_pVTKWindow->GetInteractorStyle() );

	switch ( interactorStyle->getClippingAlgorithmId() )
	{
	case evAlgorithmEraser2D:
		interactorStyle->setClippingAlgorithm( evAlgorithmEraser3D );
		break;
	case evAlgorithmEraser3D:
		interactorStyle->setClippingAlgorithm( evAlgorithmEraser2D );
		break;
	case evAlgorithmClipper2D:
		interactorStyle->setClippingAlgorithm( evAlgorithmClipper3D );
		break;
	case evAlgorithmClipper3D:
		interactorStyle->setClippingAlgorithm( evAlgorithmClipper2D );
		break;
	}
}


/** Called when the user clicks the "Enable Wii Motion Plus" Checkbox. */
void CMainFrame::OnCheckboxEnableWiiMotionPlus( wxCommandEvent &evt )
{
	// Reset the current clipping algorithm, for safety reasons
	vtkSmartPointer<CVolumeClipperAppInteractorStyle> interactorStyle = static_cast<CVolumeClipperAppInteractorStyle *>( m_pVTKWindow->GetInteractorStyle() );

	const int curAlgorithmId = interactorStyle->getClippingAlgorithmId();
	interactorStyle->setClippingAlgorithm( evAlgorithmNone );
	interactorStyle->setClippingAlgorithm( curAlgorithmId );
}


/** Called when the state of the connection to the WiiMote changes.
 * @param bConnected A flag indicating if the WiiMote has stablished a connection (true) or lost a connection (false).
 */
void CMainFrame::OnInputDeviceConnectionChanged( bool bConnected )
{
	// Upon input device's disconnection, update CApp data
	CApp &app = wxGetApp();
	const EInputDevices changedInputDevice = app.getCurrentInputDeviceId();

	if ( bConnected == false )
		app.setCurrentInputDeviceId( evInputDeviceNone );

	// Redirect to proper methods
	switch ( changedInputDevice )
	{
	case evInputDeviceWiiMote:
		wiiMoteConnected( bConnected );
		break;
	case evInputDeviceKinect:
		kinectConnected( bConnected );
		break;
	}
}


/** Called when the WiiMote state is refreshed. */
void CMainFrame::OnInputDeviceStateUpdated( void )
{
	CApp &app = wxGetApp();

	// Get a pointer to each widget we'll be updating
	wxStaticText *lblBattery = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_BATTERY), this ) );
	wxStaticText *lblLeds = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_LEDS), this ) );
	wxStaticText *lblButtons = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_BUTTONS), this ) );
	wxStaticText *lblAccelX = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ACCELX), this ) );
	wxStaticText *lblAccelY = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ACCELY), this ) );
	wxStaticText *lblAccelZ = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ACCELZ), this ) );
	wxStaticText *lblOrientX = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ORIENTX), this ) );
	wxStaticText *lblOrientY = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ORIENTY), this ) );
	wxStaticText *lblOrientZ = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ORIENTZ), this ) );
	wxStaticText *lblPitch = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_PITCH), this ) );
	wxStaticText *lblRoll = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WIIMOTE_ROLL), this ) );
	wxStaticText *lblMotionPlusState = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WII_MOTION_PLUS_STATE), this ) );
	wxStaticText *lblMotionPlusYaw = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WII_MOTION_PLUS_YAW), this ) );
	wxStaticText *lblMotionPlusPitch = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WII_MOTION_PLUS_PITCH), this ) );
	wxStaticText *lblMotionPlusRoll = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_WII_MOTION_PLUS_ROLL), this ) );

	// Get a reference to the WiiMote
	wiimote &remote = app.getWiiMote();

	// Create a string representing the LEDs of the WiiMote
	wxString ledsStr(_T("----"));
	for ( int lightIndex = 0; lightIndex < 4; lightIndex++ )
	{
		if ( remote.LED.Lit( lightIndex ) )
			ledsStr[lightIndex] = _T('*');
	}
	lblLeds->SetLabel( ledsStr );

	// Create a string containing the pressed buttons
	wxString buttonsStr;
	for(int bit=0; bit<16; bit++)
	{
		int mask = (1 << bit);
		if((wiimote_state::buttons::ALL & mask) == 0)
			continue;

		const TCHAR* buttonName = wiimote::ButtonNameFromBit[bit];
		bool		 buttonPressed	 = ((remote.Button.Bits & mask) != 0);

		if ( buttonPressed )
		{
			if ( buttonsStr.IsEmpty() == false )
				buttonsStr.append( _T(", ") );
			buttonsStr.append(buttonName);
		}
	}

	if ( buttonsStr.IsEmpty() )
		buttonsStr = STR_NONE;

	lblButtons->SetLabel( buttonsStr );

	// Update Motion Plus data
	if ( remote.MotionPlusConnected() )
	{
		lblMotionPlusState->SetLabel( STR_INPUT_DEVICE_CONNECTED );
		lblMotionPlusYaw->SetLabel( wxString::Format( _T("%f"), remote.MotionPlus.Speed.Yaw ) );
		lblMotionPlusPitch->SetLabel( wxString::Format( _T("%f"), remote.MotionPlus.Speed.Pitch ) );
		lblMotionPlusRoll->SetLabel( wxString::Format( _T("%f"), remote.MotionPlus.Speed.Roll ) );
	}
	else
		lblMotionPlusState->SetLabel( STR_INPUT_DEVICE_DISCONNECTED );

	// Update other displayed data
	lblBattery->SetLabel( wxString::Format( _T("%d%%"), remote.BatteryPercent ) );
	lblAccelX->SetLabel( wxString::Format( _T("%+2.3f"), remote.Acceleration.X ) );
	lblAccelY->SetLabel( wxString::Format( _T("%+2.3f"), remote.Acceleration.Y ) );
	lblAccelZ->SetLabel( wxString::Format( _T("%+2.3f"), remote.Acceleration.Z ) );
	lblOrientX->SetLabel( wxString::Format( _T("%+2.3f"), remote.Acceleration.Orientation.X ) );
	lblOrientY->SetLabel( wxString::Format( _T("%+2.3f"), remote.Acceleration.Orientation.Y ) );
	lblOrientZ->SetLabel( wxString::Format( _T("%+2.3f"), remote.Acceleration.Orientation.Z ) );
	lblPitch->SetLabel( wxString::Format( _T("%d"), static_cast<int>( remote.Acceleration.Orientation.Pitch ) ) );
	lblRoll->SetLabel( wxString::Format( _T("%d"), static_cast<int>( remote.Acceleration.Orientation.Roll ) ) );
}


/** Called when the user clicks the "Clear App Console" button.
 * @param evt An object containing data about the event. */
void CMainFrame::OnButtonClearAppConsole( wxCommandEvent &evt )
{
	CAppConsole &console = CAppConsole::getInstance();
	console.clearLog();
}


/** Called when the user clicks the button to connect the application with the WiiMote.
 * @param evt An object containing data about the event. */
void CMainFrame::OnButtonConnectWiiMote( wxCommandEvent &evt )
{
	CApp &app = wxGetApp();
	wiimote &remote = app.getWiiMote();

	// Verify if the WiiMote should be connected or disconnected
	if ( remote.IsConnected() == false )
	{
		// Open a progress dialog
		wxProgressDialog progressDlg( _T("Connecting to WiiMote..."),
			_T("Trying to connect to WiiMote.\nPlease, hold 1 and 2 simultaneously while the application tries to connect to the WiiMote."),
			1, &app.getMainFrame(), wxPD_SMOOTH | wxPD_CAN_ABORT );
		progressDlg.Centre();
		progressDlg.Fit();

		// Wait for the connection to be stablished
		bool userCancelled = false;
		while ( userCancelled == false && remote.Connect( wiimote::FIRST_AVAILABLE ) == false )
		{
			wxMilliSleep(200);
			if ( progressDlg.Pulse() == false )
				userCancelled = true;
		}

		if ( remote.IsConnected() )
		{
			// Configure WiiMote
			remote.SetReportType( wiimote::IN_BUTTONS_ACCEL_IR_EXT );
			remote.SetLEDs( (1<<0) | (1<<2) );

			// Set the WiiMote's input Processor
			vtkSmartPointer<CInputDeviceProcessor> devProcessor( new CWiiMoteInputProcessor() );
			m_interactorStyle->setInputDeviceProcessor( devProcessor );
		}

	}
	else
	{
		// Disconnect WiiMote and renew the wiimote object instance
		remote.Disconnect();
		app.destroyCurrentWiiMoteInstance();

		// Destroy the WiiMote's input processor
		m_interactorStyle->setInputDeviceProcessor( NULL );
	}
}




/** Called when the user clicks the button to connect the application with a Kinect device.
 * @param evt An object containing data about the event.
 */
void CMainFrame::OnButtonConnectKinect( wxCommandEvent &evt )
{
	// Is the application set to "Kinect Simulation Mode"?
	CApp &app = wxGetApp();
	const bool isKinectInSimulationMode = app.isKinectSimulationModeEnabled();

	/* Are there any Kinect devices to connect to? (Kinect Simulation Mode does not need that test: there's always a
	 * virtual Kinect ready for us!) */
	if ( isKinectInSimulationMode == false )
	{
		int totalKinectDevices = -1;
		HRESULT opResult = NuiGetSensorCount( &totalKinectDevices );
		if ( opResult != S_OK )
		{
			wxMessageBox( wxString::Format( _T( "Error [0x%08X] has occurred while trying to detect Kinect devices." ), opResult ),
				_T( "Error retrieving sensors" ), wxOK | wxICON_ERROR );
			return;
		}

		if ( totalKinectDevices <= 0 )
		{
			wxMessageBox( _T( "There are no Kinect devices currently available!" ), _T( "No devices available" ),
				wxOK | wxICON_INFORMATION );
			return;
		}
	}

	// Verify if the Kinect sensor should be connected or disconnected
	wxMenuItem *kinectSimModeMenuItem = this->GetMenuBar()->FindItem( XRCID( MENU_ITEM_KINECT_SIMULATION_MODE ) );
	if ( app.isKinectConnected() == false )
	{
		// Create the Kinect sensor
		if ( app.createKinect() == false )
		{
			wxMessageBox( _T( "Generic Error while creating a Kinect Sensor Reference!" ), _T( "Kinect Sensor: Failure" ), wxICON_ERROR );
			return;
		}

		// If everything went well, set the Kinect's input Processor
		vtkSmartPointer<CInputDeviceProcessor> devProcessor( new CKinectInputProcessor() );
		m_interactorStyle->setInputDeviceProcessor( devProcessor );

		// Also, disable the "Kinect Simulation Mode" menu item
		kinectSimModeMenuItem->Enable( false );
	}
	else
	{
		// Disconnect Kinect
		app.destroyCurrentKinectInstance();

		// Destroy the Kinect's input processor
		m_interactorStyle->setInputDeviceProcessor( NULL );

		// Enable the "Kinect Simulation Mode" menu item
		kinectSimModeMenuItem->Enable( true );
	}
}


/** Called when the user changes the Eraser Radius Slider. */
void CMainFrame::OnSliderKinectElevationScrolled( wxCommandEvent & )
{
	// Retrieve widget references
	wxSlider *sldElevationAngle = static_cast<wxSlider *>( wxWindow::FindWindowById( XRCID(SLIDER_KINECT_ELEVATION_ANGLE), this ) );
	wxStaticText *lblElevationAngle = static_cast<wxStaticText *>( wxWindow::FindWindowById( XRCID(LABEL_KINECT_ELEVATION_ANGLE), this ) );

	// Update label with angle
	const int newAngle = sldElevationAngle->GetValue();
	lblElevationAngle->SetLabel( wxString::Format( _T( "%d" ), newAngle ) );

	// Send command to Kinect sensor
	CApp &app = wxGetApp();
	INuiSensor &kinect = app.getKinect();

	kinect.NuiCameraElevationSetAngle( newAngle );
}



/** Called when the user closes the window.
 * @param evt An object containing data about the event.
 */
void CMainFrame::OnClose( wxCloseEvent &evt )
{
	// Destroy application's resources
	CApp &app = wxGetApp();
	app.destroyUpdateTimer();

	app.destroyCurrentWiiMoteInstance();
	app.destroyCurrentKinectInstance();

	// Destroy main frame
	this->Destroy();
}


/** Retrieves the renderer of the VTK rendering window, which is contained inside the Main Frame. */
vtkSmartPointer<vtkRenderer> CMainFrame::getRenderer( void ) const
{
    return m_renderer;
}


/** Retrieves the Interactor Style used by the main frame.
 * @return Returns a smart pointer which points to the Main Frame's interactor style.
 */
vtkSmartPointer<CVolumeClipperAppInteractorStyle> CMainFrame::getInteractorStyle( void )
{
#ifdef DEBUG
	vtkSmartPointer<CVolumeClipperAppInteractorStyle> interactorStyle = dynamic_cast<CVolumeClipperAppInteractorStyle *>( m_pVTKWindow->GetInteractorStyle() );
#else
	vtkSmartPointer<CVolumeClipperAppInteractorStyle> interactorStyle = static_cast<CVolumeClipperAppInteractorStyle *>( m_pVTKWindow->GetInteractorStyle() );
#endif
	return interactorStyle;
}


/** Verifies if the user has checked the "Enable" checkbox associated to the Wii Motion Plus accessory.
 * @return Returns true if the checkbox is checked, false otherwise. */
bool CMainFrame::isWiiMotionPlusChecked( void ) const
{
    wxCheckBox *chkEnableMotionPlus = static_cast<wxCheckBox *>( wxWindow::FindWindowById( XRCID(CHECKBOX_ENABLE_MOTION_PLUS), this ) );
    return chkEnableMotionPlus->IsChecked();
}


/** Retrieves the currently defined WiiMote Sensitivity value.
 * @return Returns a factor representing the sensitivity of the WiiMote. Default returned value is 1.0f, which is
 *    exactly in the middle of the WiiMote Sensitivity Slider. */
float CMainFrame::getWiiMoteSensitivity( void ) const
{
	wxSlider *slider = static_cast<wxSlider *>( wxWindow::FindWindowById( XRCID( SLIDER_WIIMOTE_SENSIBILITY ), this ) );

	float midValue = (slider->GetMin() + slider->GetMax()) / 2.0f;
	return (slider->GetValue() / midValue);
}

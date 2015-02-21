#include "StdAfx.h"
#include <wx/dnd.h>
#include "MainWindow.h"

#if !defined(__WXMSW__) && !defined(__WXPM__)
#include "DKMediaPlayer.xpm"
#endif

#define FRAME_CLIENT_MIN_WIDTH		300
#define FRAME_CLIENT_MIN_HEIGHT		200

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(UICommandFileOpenLocal,		MainWindow::OnFileOpenLocal)
	EVT_MENU(UICommandFileOpenURL,			MainWindow::OnFileOpenURL)
	EVT_MENU(UICommandFileExit,				MainWindow::OnFileExit)

	EVT_MENU(UICommandViewResize50,			MainWindow::OnViewResize)
	EVT_MENU(UICommandViewResize100,		MainWindow::OnViewResize)
	EVT_MENU(UICommandViewResize150,		MainWindow::OnViewResize)
	EVT_MENU(UICommandViewResize200,		MainWindow::OnViewResize)
	EVT_MENU(UICommandViewAutoResize,		MainWindow::OnViewAutoResize)
	EVT_MENU(UICommandViewFullScreen,		MainWindow::OnViewFullScreen)

	EVT_MENU(UICommandHelpAbout,			MainWindow::OnHelpAbout)

	EVT_DROP_FILES(MainWindow::OnDropFiles)
	EVT_SIZE(MainWindow::OnSize)
	EVT_CLOSE(MainWindow::OnClose)
	EVT_MOUSE_EVENTS(MainWindow::OnMouseEvent)
wxEND_EVENT_TABLE();

MainWindow::MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxFrame(NULL, wxID_ANY, title, pos, size, style)
	, autoResize(true)
{
	// set the frame icon
	SetIcon(wxICON(DKMediaPlayer));

	// create a menu
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(UICommandFileOpenLocal, "&Open...\tCtrl-O", "Open exist");
	fileMenu->Append(UICommandFileOpenURL, "&Open URL...\tCtrl-U", "Open Url");
	fileMenu->AppendSeparator();
	fileMenu->Append(UICommandFileExit, "E&xit\tAlt-X", "Quit this program");

	wxMenu* viewMenu = new wxMenu;
	wxMenu* viewScaleMenu = new wxMenu;
	viewScaleMenu->Append(UICommandViewResize50, "Resize 50%\t1");
	viewScaleMenu->Append(UICommandViewResize100, "Resize 100%\t2");
	viewScaleMenu->Append(UICommandViewResize150, "Resize 150%\t3");
	viewScaleMenu->Append(UICommandViewResize200, "Resize 200%\t4");
	viewMenu->AppendSubMenu(viewScaleMenu, "Scale");
	viewMenu->AppendCheckItem(UICommandViewAutoResize, "Auto Resize\tR");
	viewMenu->AppendCheckItem(UICommandViewFullScreen, "Full Screen\tEnter");

	wxMenu* helpMenu = new wxMenu;
	helpMenu->Append(UICommandHelpAbout, "About...", "About Program");
	
	// create a menu bar
	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "&File");
	menuBar->Append(viewMenu, "&View");
	menuBar->Append(helpMenu, "&Help");
	SetMenuBar(menuBar);

	//SetDoubleBuffered(true);
	SetBackgroundColour(wxColour(0,0,0));
	SetMinClientSize(wxSize(FRAME_CLIENT_MIN_WIDTH, FRAME_CLIENT_MIN_HEIGHT));
	//SetMinSize(wxSize(FRAME_CLIENT_MIN_WIDTH, FRAME_CLIENT_MIN_HEIGHT) + (GetSize() - GetClientSize()));

	//DragAcceptFiles(true);
	class MediaFileDropTarget : public wxFileDropTarget
	{
	public:
		MediaFileDropTarget(MainWindow* w) : wnd(w) {}
	protected:
		bool OnDropFiles(wxCoord, wxCoord, const wxArrayString& files)
		{
			//DKLog("%s\n", DKLIB_FUNCTION_NAME);
			wnd->OnDropFiles(files);
			return true;
		}
	private:
		MainWindow* wnd;
	};
	this->SetDropTarget(new MediaFileDropTarget(this));

	DKApplication* app = DKApplication::Instance();
	// 설정 파일 로딩
	DKString configFile = DKDirectory::OpenDir(app->EnvironmentPath(DKApplication::SystemPathAppResource))->AbsolutePath() + "/" + (const wchar_t*)wxTheApp->GetAppName() + ".properties";
	configFile = configFile.FilePathString();
	int imported = DKPropertySet::DefaultSet().Import(configFile, true);
	DKPropertySet::DefaultSet().SetValue("ConfigFile", configFile);
	DKLog("Config file: %ls (%d imported)\n", (const wchar_t*)configFile, imported);

	if (DKPropertySet::DefaultSet().Value("FrameRect").ValueType() == DKVariant::TypeVector4)
	{
		DKVector4 frameRectVec = DKPropertySet::DefaultSet().Value("FrameRect").Vector4();
		wxPoint framePos = wxPoint(frameRectVec.x, frameRectVec.y);
		wxSize frameSize = wxSize(Max<int>(frameRectVec.z, FRAME_CLIENT_MIN_WIDTH), Max<int>(frameRectVec.w, FRAME_CLIENT_MIN_HEIGHT));

		this->SetPosition(framePos);
		this->SetClientSize(frameSize);
	}
	if (DKPropertySet::DefaultSet().Value("AutoResize").ValueType() == DKVariant::TypeInteger)
	{
		this->autoResize = DKPropertySet::DefaultSet().Value("AutoResize").Integer() != 0;
	}
	GetMenuBar()->FindItem(UICommandViewAutoResize)->Check(this->autoResize);

	this->window = DKWindow::CreateProxy(GetHandle());
	if (this->window)
	{
		DKLog("Proxy Window created.\n");
		
		this->mediaFrame = DKObject<MediaFrame>::New();
		// 렌더러 생성 및 연계
		this->screen = DKObject<DKScreen>::New();
		if (this->screen->Run(this->window, this->mediaFrame))
		{
			//	screen->SetMinimumActiveFrameTime(1.0f/60.0f);		// 활성시 60 fps 로 한다
			//	screen->SetMinimumInactiveFrameTime(1.0f/60.0f);		// 비활성시 60 fps 로 한다
			// 딜레이를 적용시키기 위해 액티베이트 시킨다.
			this->window->PostWindowEvent(DKWindow::EventWindowActivated, this->window->ContentSize(), this->window->Origin(), false);
			DKOpenGLContext::SharedInstance()->Bind();
		}
		else
		{
			this->mediaFrame = NULL;
			this->window = NULL;
			this->screen = NULL;
			DKERROR_THROW("Error: Failed to create renderer.\n");
		}
	}
	else
	{
		this->mediaFrame = NULL;
		this->window = NULL;
		this->screen = NULL;
		DKERROR_THROW("Error: cannot create Proxy Window.\n");
	}
}

MainWindow::~MainWindow(void)
{
	mediaFrame = NULL;
	screen = NULL;
	window = NULL;

	DKString configFile = DKPropertySet::DefaultSet().Value("ConfigFile").String();
	DKPropertySet::DefaultSet().Export(configFile, true);
}

void MainWindow::OpenFiles(const wxArrayString& files)
{
	this->OnDropFiles(files);
}

void MainWindow::OnFileOpenLocal(wxCommandEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
	wxFileDialog file(this, "Select Media File", wxEmptyString, wxEmptyString, 
		"Movie files |*.avi;*.wmv;*.mov;*.asf;*.mp4;*.mkv|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (file.ShowModal() == wxID_OK)
	{
		wxString filepath = file.GetPath();
		DKLog("Openning File:%ls...\n", (const wchar_t*)filepath);
		if (!OpenMediaURL(filepath))
			DKLog("Cannot open File:%ls.\n", (const wchar_t*)filepath);
	}
}

void MainWindow::OnFileOpenURL(wxCommandEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
	wxTextEntryDialog url(this, "OpenURL", "OpenURL");
	if (url.ShowModal() == wxID_OK)
	{
		wxString filepath = url.GetValue();
		DKLog("Openning URL:%ls...\n", (const wchar_t*)filepath);
		if (!OpenMediaURL(filepath))
			DKLog("Cannot open URL:%ls.\n", (const wchar_t*)filepath);
	}
}

void MainWindow::OnFileExit(wxCommandEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
	e.Skip();
	Close(true);
}

void MainWindow::OnViewResize(wxCommandEvent& e)
{
	if (this->IsFullScreen())
	{
		DKLog("MainWindow is fullscreen!\n");
		return;
	}

	double scale = 1.0;
	switch (e.GetId())
	{
	case UICommandViewResize50:		scale = 0.5; break;
	case UICommandViewResize100:	scale = 1.0; break;
	case UICommandViewResize150:	scale = 1.5; break;
	case UICommandViewResize200:	scale = 2.0; break;
	default:
		return;
	}

	int width = 0;
	int height = 0;
	if (mediaFrame->GetMediaSize(width, height) && width > 0 && height > 0)
	{
		SetClientSize(width * scale, height * scale);
	}
}

void MainWindow::OnViewAutoResize(wxCommandEvent& e)
{
	this->autoResize = this->GetMenuBar()->FindItem(UICommandViewAutoResize)->IsChecked();
	DKLog("Auto Resize:%d\n", (int)this->autoResize);
}

void MainWindow::OnViewFullScreen(wxCommandEvent& e)
{
	bool fullScreen = this->GetMenuBar()->FindItem(UICommandViewFullScreen)->IsChecked();
	DKLog("FullScreen:%d\n", (int)fullScreen);

	this->ShowFullScreen(fullScreen, wxFULLSCREEN_ALL);
}

void MainWindow::OnHelpAbout(wxCommandEvent& e)
{
	wxMessageBox(wxString::Format(
		"DK MediaPlayer\n\n"
		"Using %ls.\n"
		"running under %ls.",
		(const wchar_t*)wxVERSION_STRING,
		(const wchar_t*)wxGetOsDescription()),
		wxString::Format("About %ls", (const wchar_t*)wxTheApp->GetAppName()),
		wxOK | wxICON_INFORMATION,
		this);
}

void MainWindow::OnSize(wxSizeEvent& e)
{
	if (window)
		window->UpdateProxy();
}

void MainWindow::OnClose(wxCloseEvent& e)
{
	mediaFrame = NULL;
	screen->Terminate(true);
	screen = NULL;
	window = NULL;
	e.Skip();

	wxPoint framePos = GetPosition();
	wxSize frameSize = GetClientSize();
	DKPropertySet::DefaultSet().SetValue("FrameRect", DKVariant::VVector4(framePos.x, framePos.y, frameSize.x, frameSize.y));
	DKPropertySet::DefaultSet().SetValue("AutoResize", DKVariant::VInteger(this->autoResize));
}

void MainWindow::OnDropFiles(const wxArrayString& files)
{
	for (size_t i = 0; i < files.GetCount(); ++i)
	{
		const wxString& filepath = files.Item(i);
		DKLog("Openning file:%ls...\n", (const wchar_t*)filepath);
		if (OpenMediaURL(filepath))
			break;
		else
			DKLog("Cannot open File:%ls.\n", (const wchar_t*)filepath);
	}
}

void MainWindow::OnDropFiles(wxDropFilesEvent& e)
{
	wxArrayString files;
	for (int i = 0; i < e.GetNumberOfFiles(); i++)
		files.Add(e.GetFiles()[i]);

	return OnDropFiles(files);
}

void MainWindow::OnMouseEvent(wxMouseEvent& e)
{
	e.Skip();

	if (window && screen)
	{
		DKPoint	pos(e.m_x, GetClientSize().GetHeight() - e.m_y);
		DKVector2 delta = pos.Vector() - mousePosition.Vector();
		mousePosition = pos;

		int id = e.GetEventType();
		
		if		(id == wxEVT_LEFT_DOWN)			window->PostMouseEvent(DKWindow::EventMouseDown,	0, 0, mousePosition, delta, false);
		else if (id == wxEVT_LEFT_DCLICK)		window->PostMouseEvent(DKWindow::EventMouseDown,	0, 0, mousePosition, delta, false);
		else if (id == wxEVT_LEFT_UP)			window->PostMouseEvent(DKWindow::EventMouseUp,		0, 0, mousePosition, delta, false);
		else if (id == wxEVT_RIGHT_DOWN)		window->PostMouseEvent(DKWindow::EventMouseDown,	0, 1, mousePosition, delta, false);
		else if (id == wxEVT_RIGHT_DCLICK)		window->PostMouseEvent(DKWindow::EventMouseDown,	0, 1, mousePosition, delta, false);
		else if (id == wxEVT_RIGHT_UP)			window->PostMouseEvent(DKWindow::EventMouseUp,		0, 1, mousePosition, delta, false);
		else if (id == wxEVT_MIDDLE_DOWN)		window->PostMouseEvent(DKWindow::EventMouseDown,	0, 2, mousePosition, delta, false);
		else if (id == wxEVT_MIDDLE_DCLICK)		window->PostMouseEvent(DKWindow::EventMouseDown,	0, 2, mousePosition, delta, false);
		else if (id == wxEVT_MIDDLE_UP)			window->PostMouseEvent(DKWindow::EventMouseUp,		0, 2, mousePosition, delta, false);
		else if (id == wxEVT_MOTION)			window->PostMouseEvent(DKWindow::EventMouseMove,	0, 0, mousePosition, delta, false);
		else if (id == wxEVT_ENTER_WINDOW)		window->PostMouseEvent(DKWindow::EventMouseMove,	0, 0, mousePosition, delta, false);
		else if (id == wxEVT_LEAVE_WINDOW)		window->PostMouseEvent(DKWindow::EventMouseMove,	0, 0, mousePosition, delta, false);
		else if (id == wxEVT_MOUSEWHEEL)
		{
			window->PostMouseEvent(DKWindow::EventMouseWheel,	0, 0, mousePosition, DKVector2(0, e.m_wheelRotation > 0 ? 1 : (e.m_wheelRotation < 0 ? -1 : 0)), false);
		}
		else
		{
			//wxEVT_ENTER_WINDOW:
			//wxEVT_LEAVE_WINDOW:
		}
	}
}

bool MainWindow::OpenMediaURL(const wxString& url)
{
	if (mediaFrame->OpenMedia((const wchar_t*)url))
	{
		if (this->autoResize && !this->IsFullScreen())
		{
			int width = 0;
			int height = 0;
			if (mediaFrame->GetMediaSize(width, height) && width > 0 && height > 0)
			{
				SetClientSize(width, height);
			}
		}
		wxString title = wxString::Format("%ls - %ls", (const wchar_t*)url, (const wchar_t*)wxTheApp->GetAppName());
		this->SetTitle(title);
		return true;
	}

	this->SetTitle(wxTheApp->GetAppName());
	return false;
}

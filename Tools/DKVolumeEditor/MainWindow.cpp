#include "StdAfx.h"

#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

#include "MainWindow.h"
#
#if !defined(__WXMSW__) && !defined(__WXPM__)
#include "DKVolumeEditor.xpm"
#endif

#include <wx/imaglist.h>
#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/image.h>
#include <wx/propgrid/propgrid.h>
#include <wx/wrapsizer.h>
#include <wx/spinctrl.h>
#include <wx/xrc/xmlres.h>
#include <wx/fs_mem.h>

#define MINIMUM_VOLUME_SIZE	8
#define MAXIMUM_VOLUME_SIZE	4096

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(wxID_NEW,						MainWindow::OnFileNew)
	EVT_MENU(wxID_OPEN,						MainWindow::OnFileOpen)
	EVT_MENU(wxID_SAVE,						MainWindow::OnFileSave)
	EVT_MENU(wxID_SAVEAS,					MainWindow::OnFileSaveAs)
	EVT_MENU(wxID_EXIT,						MainWindow::OnFileQuit)

	EVT_MENU(wxID_CUT,						MainWindow::OnEditCut)
	EVT_MENU(wxID_COPY,						MainWindow::OnEditCopy)
	EVT_MENU(wxID_PASTE,					MainWindow::OnEditPaste)
	EVT_MENU(wxID_DELETE,					MainWindow::OnEditDelete)

	EVT_MENU(XRCID("MenuItemViewLog"),		MainWindow::OnViewLog)

	EVT_MENU(wxID_PREFERENCES,				MainWindow::OnToolsOptions)

	EVT_MENU(wxID_ABOUT,					MainWindow::OnHelpAbout)

	EVT_SIZE(								MainWindow::OnSize)
	EVT_CLOSE(								MainWindow::OnClose)

	EVT_CHECKBOX(wxID_ANY,					MainWindow::OnCheckBoxClicked)
	EVT_SLIDER(wxID_ANY,					MainWindow::OnSliderUpdated)
	EVT_COLOURPICKER_CHANGED(wxID_ANY,		MainWindow::OnColourPickerChanged)
	EVT_BUTTON(XRCID("ResizeVolume"),		MainWindow::OnResizeVolumeClicked)
wxEND_EVENT_TABLE();

MainWindow::MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxFrame(NULL, wxID_ANY, title, pos, size, style)
	, volumeWidth(MINIMUM_VOLUME_SIZE)
	, volumeHeight(MINIMUM_VOLUME_SIZE)
	, volumeDepth(MINIMUM_VOLUME_SIZE)
{
	// set the frame icon
	SetIcon(wxICON(DKVolumeEditor));

	// auiManager 초기화
	auiManager.SetManagedWindow(this);

	wxFileSystem::AddHandler(new wxMemoryFSHandler());
	wxXmlResource::Get()->InitAllHandlers();
	if (true)
	{
		DKObject<DKData> d = DKApplication::Instance()->LoadStaticResource("ui.xrc");
		DKDataReader reader(d);
		wxMemoryFSHandler::AddFile("ui.xrc", (const void*)reader, reader.Length());
	}
	wxXmlResource::Get()->Load("memory:ui.xrc");

	editorWindow = wxXmlResource::Get()->LoadPanel(this, "EditorPanel");
	sliderX = XRCCTRL(*editorWindow, "SliderX", wxSlider);
	sliderY = XRCCTRL(*editorWindow, "SliderY", wxSlider);
	sliderZ = XRCCTRL(*editorWindow, "SliderZ", wxSlider);
	checkboxX = XRCCTRL(*editorWindow, "CheckBoxX", wxCheckBox);
	checkboxY = XRCCTRL(*editorWindow, "CheckBoxY", wxCheckBox);
	checkboxZ = XRCCTRL(*editorWindow, "CheckBoxZ", wxCheckBox);
	voxelColorPicker = XRCCTRL(*editorWindow, "VoxelColorPicker", wxColourPickerCtrl);
	wireColorPicker = XRCCTRL(*editorWindow, "WireColorPicker", wxColourPickerCtrl);
	bgColorPicker = XRCCTRL(*editorWindow, "BGColorPicker", wxColourPickerCtrl);

	// 로그 윈도우
	logWindow = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_RICH | wxTE_MULTILINE | wxTE_AUTO_URL);
	logWindow->AppendText("this is log window\nhttp://icondb.com\n");

	// 메인 스크린
	renderWindow = new RenderWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, "Main Screen");


	// 윈도우들 붙이기
	auiManager.AddPane(editorWindow, wxAuiPaneInfo().Name("editor").Caption("Editor")
		.Right().Layer(0).Position(2).CloseButton(false).MaximizeButton(false).MinSize(200,200).BestSize(350,300));

	auiManager.AddPane(logWindow, wxAuiPaneInfo().Name("logWindow").Caption("Log")
		.Bottom().Layer(1).Position(3).CloseButton(true).MaximizeButton(false).BestSize(250,300));

	auiManager.AddPane(renderWindow, wxAuiPaneInfo().Name("screenWindow").CenterPane().PaneBorder(false));

	auiManager.GetPane(renderWindow).Show(true);

	wxMenuBar* menuBar = wxXmlResource::Get()->LoadMenuBar("MyMenuBar1");
	SetMenuBar(menuBar);

	// create a status bar just for fun (by default with 1 pane only)
	CreateStatusBar(2);
	SetStatusText("Welcome to wxWidgets!");

	SetMinSize(wxSize(400,400));

	DKApplication* app = DKApplication::Instance();
	DKString configFile = DKDirectory::OpenDir(app->EnvironmentPath(DKApplication::SystemPathAppResource))->AbsolutePath() + "/" + (const wchar_t*)wxTheApp->GetAppName() + ".properties";
	int imported = DKPropertySet::DefaultSet().Import(configFile, true);
	DKLog("Config file: %ls (%d imported)\n", (const wchar_t*)configFile, imported);

	if (DKPropertySet::DefaultSet().Value("FrameRect").ValueType() == DKVariant::TypeVector4)
	{
		wxRect screen = wxGetClientDisplayRect();

		DKVector4 frameRectVec = DKPropertySet::DefaultSet().Value("FrameRect").Vector4();
		wxPoint framePos = wxPoint(frameRectVec.x, frameRectVec.y);
		wxSize frameSize = wxSize(Max<int>(frameRectVec.z, 400), Max<int>(frameRectVec.w, 400));

		if (framePos.x < screen.x)				framePos.x = screen.x;
		if (framePos.y < screen.y)				framePos.y = screen.y;
		if (frameSize.x > screen.width)			frameSize.x = screen.width;
		if (frameSize.y > screen.height)		frameSize.y = screen.height;
		
		this->SetPosition(framePos);
		this->SetSize(frameSize);
	}
	if (DKPropertySet::DefaultSet().Value("AUILayout").ValueType() == DKVariant::TypeString)
	{
		auiManager.LoadPerspective((const wchar_t*)DKPropertySet::DefaultSet().Value("AUILayout").String()); 
	}
	if (DKPropertySet::DefaultSet().Value("VoxelColor").ValueType() == DKVariant::TypeString)
	{
		wxColour color = (const wchar_t*)DKPropertySet::DefaultSet().Value("VoxelColor").String();
		this->voxelColorPicker->SetColour(color);
	}
	if (DKPropertySet::DefaultSet().Value("WireColor").ValueType() == DKVariant::TypeString)
	{
		wxColour color = (const wchar_t*)DKPropertySet::DefaultSet().Value("WireColor").String();
		this->wireColorPicker->SetColour(color);
	}
	if (DKPropertySet::DefaultSet().Value("BGColor").ValueType() == DKVariant::TypeString)
	{
		wxColour color = (const wchar_t*)DKPropertySet::DefaultSet().Value("BGColor").String();
		this->bgColorPicker->SetColour(color);
	}


	// 다른곳에서 사용하기 위해 DKPropertySet::DefaultSet 에 저장함
	DKPropertySet::DefaultSet().SetValue("ConfigFile", configFile);

	// 프록시 윈도우 및 렌더러 생성은 auiManager.Update() 후에 한다!
	renderWindow->InitScreen();

	ProcessCommand(wxID_NEW);
}

MainWindow::~MainWindow(void)
{
	auiManager.UnInit();

	DKString configFile = DKPropertySet::DefaultSet().Value("ConfigFile").String();
	DKPropertySet::DefaultSet().Export(configFile, true);

	wxXmlResource::Get()->Unload("memory:ui.xrc");
	wxMemoryFSHandler::RemoveFile("ui.xrc");
}

void MainWindow::OnFileNew(wxCommandEvent& e)
{
	PrintLog("OnFileNew\n", wxColour(0,0,255));

	volumeWidth = 16;
	volumeHeight = 16;
	volumeDepth = 16;

	sliderX->SetValue(0);
	sliderY->SetValue(0);
	sliderZ->SetValue(0);

	sliderX->SetMin(0);
	sliderY->SetMin(0);
	sliderZ->SetMin(0);
	sliderX->SetMax(volumeWidth-1);
	sliderY->SetMax(volumeHeight-1);
	sliderZ->SetMax(volumeDepth-1);

	renderWindow->frame.ResetVoxelData();
	renderWindow->frame.SetVolumeSize(volumeWidth, volumeHeight, volumeDepth);

	int posX = sliderX->GetValue();
	int posY = sliderY->GetValue();
	int posZ = sliderZ->GetValue();
	renderWindow->frame.SetSliderPos( posX, posY, posZ );

	bool x = checkboxX->IsChecked();
	bool y = checkboxY->IsChecked();
	bool z = checkboxZ->IsChecked();
	renderWindow->frame.SetSliderPlaneVisible(x,y,z);

	{
		wxColour c = this->voxelColorPicker->GetColour();
		DKColor::RGBA32 rgba = {c.Red(), c.Green(), c.Blue(), 0xff};
		renderWindow->frame.SetVoxelColor(rgba);
	}
	{
		wxColour c = this->wireColorPicker->GetColour();
		DKColor::RGBA32 rgba = {c.Red(), c.Green(), c.Blue(), 0xff};
		renderWindow->frame.SetWireframeColor(rgba);
	}
	{
		wxColour c = this->bgColorPicker->GetColour();
		DKColor::RGBA32 rgba = {c.Red(), c.Green(), c.Blue(), 0xff};
		renderWindow->frame.SetBGColor(rgba);
	}
}

void MainWindow::OnFileOpen(wxCommandEvent& e)
{
	PrintLog("OnFileOpen\n", wxColour(0,0,255));
}

void MainWindow::OnFileSave(wxCommandEvent& e)
{
	PrintLog("OnFileSave\n", wxColour(0,0,255));
}

void MainWindow::OnFileSaveAs(wxCommandEvent& e)
{
	PrintLog("OnFileSaveAs\n", wxColour(0,0,255));
}

void MainWindow::OnFileQuit(wxCommandEvent& e)
{
	e.Skip();
	Close(true);
}

void MainWindow::OnEditCut(wxCommandEvent& e)
{
	PrintLog("OnEditCut\n", wxColour(0,0,255));
}

void MainWindow::OnEditCopy(wxCommandEvent& e)
{
	PrintLog("OnEditCopy\n", wxColour(0,0,255));
}

void MainWindow::OnEditPaste(wxCommandEvent& e)
{
	PrintLog("OnEditPaste\n", wxColour(0,0,255));
}

void MainWindow::OnEditDelete(wxCommandEvent& e)
{
	PrintLog("OnEditDelete\n", wxColour(0,0,255));
}

void MainWindow::OnViewLog(wxCommandEvent& e)
{
	wxAuiPaneInfo& paneInfo = auiManager.GetPane(logWindow);
	paneInfo.Show(true);
	auiManager.Update();
}

void MainWindow::OnToolsOptions(wxCommandEvent& e)
{
	PrintLog("OnToolsOptions\n", wxColour(0,0,255));
}

void MainWindow::OnHelpAbout(wxCommandEvent& e)
{
	wxMessageBox(wxString::Format(
		"DK VolumeEditor\n\n"
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
	auiManager.Update();
}

void MainWindow::OnClose(wxCloseEvent& e)
{
	e.Skip();

	wxPoint framePos = GetPosition();
	wxSize frameSize = GetSize();

	DKPropertySet::DefaultSet().SetValue("FrameRect", DKVariant::VVector4(framePos.x, framePos.y, frameSize.x, frameSize.y));
	DKPropertySet::DefaultSet().SetValue("AUILayout", (const wchar_t*)auiManager.SavePerspective());
	DKPropertySet::DefaultSet().SetValue("VoxelColor", (const wchar_t*)voxelColorPicker->GetColour().GetAsString());
	DKPropertySet::DefaultSet().SetValue("WireColor", (const wchar_t*)wireColorPicker->GetColour().GetAsString());
	DKPropertySet::DefaultSet().SetValue("BGColor", (const wchar_t*)bgColorPicker->GetColour().GetAsString());

	renderWindow->Close(true);
}

void MainWindow::PrintLog(const wxString& msg, const wxColour& c)
{
	logWindow->SetDefaultStyle(wxTextAttr(c));
	logWindow->AppendText(msg);
}

void MainWindow::OnSliderUpdated(wxCommandEvent& e)
{
	wxObject* obj = e.GetEventObject();

	if (obj == sliderX || obj == sliderY || obj == sliderZ)
	{
		int posX = sliderX->GetValue();
		int posY = sliderY->GetValue();
		int posZ = sliderZ->GetValue();
		renderWindow->frame.SetSliderPos( posX, posY, posZ );
		e.Skip();
	}
}

void MainWindow::OnCheckBoxClicked(wxCommandEvent& e)
{
	wxObject* obj = e.GetEventObject();
	if (obj == checkboxX) {}
	else if (obj == checkboxY) {}
	else if (obj == checkboxZ) {}
	else
	{
		return;
	}
	e.Skip();

	bool x = checkboxX->IsChecked();
	bool y = checkboxY->IsChecked();
	bool z = checkboxZ->IsChecked();
	renderWindow->frame.SetSliderPlaneVisible(x,y,z);
}

void MainWindow::OnColourPickerChanged(wxColourPickerEvent& e)
{
	wxObject* obj = e.GetEventObject();
	if (obj == this->voxelColorPicker)
	{
		wxColour c = this->voxelColorPicker->GetColour();
		DKColor::RGBA32 rgba = {c.Red(), c.Green(), c.Blue(), 0xff};
		renderWindow->frame.SetVoxelColor(rgba);
	}
	else if (obj == this->wireColorPicker)
	{
		wxColour c = this->wireColorPicker->GetColour();
		DKColor::RGBA32 rgba = {c.Red(), c.Green(), c.Blue(), 0xff};
		renderWindow->frame.SetWireframeColor(rgba);
	}
	else if (obj == this->bgColorPicker)
	{
		wxColour c = this->bgColorPicker->GetColour();
		DKColor::RGBA32 rgba = {c.Red(), c.Green(), c.Blue(), 0xff};
		renderWindow->frame.SetBGColor(rgba);
	}
}

void MainWindow::OnResizeVolumeClicked(wxCommandEvent& e)
{
	wxDialog dlg;
	if (wxXmlResource::Get()->LoadDialog(&dlg, this, "ResizeVolumeDialog"))
	{
		wxSpinCtrl* width = XRCCTRL(dlg, "ResizeVolumeWidth", wxSpinCtrl);
		wxSpinCtrl* height = XRCCTRL(dlg, "ResizeVolumeHeight", wxSpinCtrl);
		wxSpinCtrl* depth = XRCCTRL(dlg, "ResizeVolumeDepth", wxSpinCtrl);

		width->SetRange(MINIMUM_VOLUME_SIZE, MAXIMUM_VOLUME_SIZE);
		width->SetValue(this->volumeWidth);

		height->SetRange(MINIMUM_VOLUME_SIZE, MAXIMUM_VOLUME_SIZE);
		height->SetValue(this->volumeHeight);

		depth->SetRange(MINIMUM_VOLUME_SIZE, MAXIMUM_VOLUME_SIZE);
		depth->SetValue(this->volumeDepth);

		if (dlg.ShowModal() == wxID_OK)
		{
			int w = width->GetValue();
			int h = height->GetValue();
			int d = depth->GetValue();

			if (w != volumeWidth || h != volumeHeight || d != volumeDepth)
			{
				wxASSERT(w >= MINIMUM_VOLUME_SIZE && w <= MAXIMUM_VOLUME_SIZE);
				wxASSERT(h >= MINIMUM_VOLUME_SIZE && h <= MAXIMUM_VOLUME_SIZE);
				wxASSERT(d >= MINIMUM_VOLUME_SIZE && d <= MAXIMUM_VOLUME_SIZE);

				volumeWidth = w;
				volumeHeight = h;
				volumeDepth = d;

				sliderX->SetValue(0);
				sliderY->SetValue(0);
				sliderZ->SetValue(0);

				sliderX->SetMin(0);
				sliderY->SetMin(0);
				sliderZ->SetMin(0);
				sliderX->SetMax(volumeWidth-1);
				sliderY->SetMax(volumeHeight-1);
				sliderZ->SetMax(volumeDepth-1);

				renderWindow->frame.SetVolumeSize(volumeWidth, volumeHeight, volumeDepth);
				renderWindow->frame.SetSliderPos( 0, 0, 0);
			}
		}
	}
}

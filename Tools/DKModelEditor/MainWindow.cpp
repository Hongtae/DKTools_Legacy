#include "StdAfx.h"
#include "MainWindow.h"
#
#if !defined(__WXMSW__) && !defined(__WXPM__)
#include "DKModelEditor.xpm"
#endif

#include <wx/imaglist.h>
#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/image.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/clrpicker.h>
#include <wx/propgrid/propgrid.h>

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(wxID_NEW,						MainWindow::OnFileNew)
	EVT_MENU(wxID_OPEN,						MainWindow::OnFileOpen)
	EVT_MENU(wxID_SAVE,						MainWindow::OnFileSave)
	EVT_MENU(wxID_SAVEAS,					MainWindow::OnFileSaveAs)
	EVT_MENU(UICommandFileFormatXML,		MainWindow::OnFileFormat)
	EVT_MENU(UICommandFileFormatXMLb,		MainWindow::OnFileFormat)
	EVT_MENU(UICommandFileFormatBin,		MainWindow::OnFileFormat)
	EVT_MENU(UICommandFileFormatCBin,		MainWindow::OnFileFormat)
	EVT_MENU(wxID_EXIT,						MainWindow::OnFileQuit)

	EVT_MENU(wxID_CUT,						MainWindow::OnEditCut)
	EVT_MENU(wxID_COPY,						MainWindow::OnEditCopy)
	EVT_MENU(wxID_PASTE,					MainWindow::OnEditPaste)
	EVT_MENU(wxID_DELETE,					MainWindow::OnEditDelete)

	EVT_MENU(UICommandViewNodeTree,			MainWindow::OnViewNodeTree)
	EVT_MENU(UICommandViewResources,		MainWindow::OnViewResources)
	EVT_MENU(UICommandViewScreenController,	MainWindow::OnViewScreenController)
	EVT_MENU(UICommandViewAnimation,		MainWindow::OnViewAnimation)
	EVT_MENU(UICommandViewLog,				MainWindow::OnViewLog)

	EVT_MENU(wxID_PREFERENCES,				MainWindow::OnToolsOptions)
	EVT_MENU(wxID_ABOUT,					MainWindow::OnHelpAbout)

	EVT_SIZE(MainWindow::OnSize)
	EVT_CLOSE(MainWindow::OnClose)
wxEND_EVENT_TABLE();

static const char* SerializeFormatString(DKSerializer::SerializeForm sf)
{
	switch (sf)
	{
	case DKSerializer::SerializeFormXML:				return "SerializeFormXML";break;
	case DKSerializer::SerializeFormBinXML:				return "SerializeFormBinXML";break;
	case DKSerializer::SerializeFormBinary:				return "SerializeFormBinary";break;
	case DKSerializer::SerializeFormCompressedBinary:	return "SerializeFormCompressedBinary";break;
	}
	return "Unknown";
};

MainWindow::MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxFrame(NULL, wxID_ANY, title, pos, size, style)
	, model(NULL)
	, modified(false)
	, modelPath("")
	, fileFormat(DKSerializer::SerializeFormXML)
{
	// set the frame icon
	SetIcon(wxICON(DKModelEditor));

	// 모델 노드 트리
	nodeTree = new ModelTree(this, wxID_ANY);
	nodeProperties = new ModelProperties(this, wxID_ANY);

	// 스크린 제어
	sceneController = new wxWindow(this, wxID_ANY);		// 화면 제어
	if (true)
	{
		wxStaticBoxSizer* cameraSizer = new wxStaticBoxSizer( wxVERTICAL, sceneController, "Camera" );
		{
			wxStaticBox* staticBox = cameraSizer->GetStaticBox();
			wxTextCtrl* posTxt = new wxTextCtrl(staticBox, wxID_ANY, "pos");
			wxTextCtrl* targetTxt = new wxTextCtrl(staticBox, wxID_ANY, "target");
			wxButton* resetBtn = new wxButton(staticBox, wxID_ANY, "Reset");

			wxWrapSizer* wrapSizer = new wxWrapSizer(wxHORIZONTAL);
			wrapSizer->Add(posTxt, wxSizerFlags().Centre().Border());
			wrapSizer->Add(targetTxt, wxSizerFlags().Centre().Border());
			wrapSizer->Add(resetBtn, wxSizerFlags().Centre().Border());
			cameraSizer->Add(wrapSizer, wxSizerFlags(1).Expand());
		}
		wxStaticBoxSizer* drawingSizer = new wxStaticBoxSizer( wxVERTICAL, sceneController, "Drawing" );
		{
			wxStaticBox* staticBox = drawingSizer->GetStaticBox();
			wxCheckBox* check1 = new wxCheckBox(staticBox, wxID_ANY, "check1");
			wxCheckBox* check2 = new wxCheckBox(staticBox, wxID_ANY, "check1");
			wxCheckBox* check3 = new wxCheckBox(staticBox, wxID_ANY, "check1");
			wxCheckBox* check4 = new wxCheckBox(staticBox, wxID_ANY, "check1");

			wxWrapSizer* wrapSizer = new wxWrapSizer(wxHORIZONTAL);
			wrapSizer->Add(check1, wxSizerFlags().Centre().Border());
			wrapSizer->Add(check2, wxSizerFlags().Centre().Border());
			wrapSizer->Add(check3, wxSizerFlags().Centre().Border());
			wrapSizer->Add(check4, wxSizerFlags().Centre().Border());
			drawingSizer->Add(wrapSizer, wxSizerFlags(1).Expand());
		}
		wxStaticBoxSizer* colorsSizer = new wxStaticBoxSizer( wxVERTICAL, sceneController, "Colors" );
		{
			wxStaticBox* staticBox = colorsSizer->GetStaticBox();
			wxColourPickerCtrl* color1 = new wxColourPickerCtrl(staticBox, wxID_ANY, *wxBLACK,
				wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL);
			wxColourPickerCtrl* color2 = new wxColourPickerCtrl(staticBox, wxID_ANY, *wxBLUE);
			wxColourPickerCtrl* color3 = new wxColourPickerCtrl(staticBox, wxID_ANY, *wxYELLOW);

			wxWrapSizer* wrapSizer = new wxWrapSizer(wxHORIZONTAL);
			wrapSizer->Add(color1, wxSizerFlags().Centre().Border());
			wrapSizer->Add(color2, wxSizerFlags().Centre().Border());
			wrapSizer->Add(color3, wxSizerFlags().Centre().Border());
			colorsSizer->Add(wrapSizer, wxSizerFlags(1).Expand());
		}

		wxBoxSizer* rootSizer = new wxBoxSizer( wxVERTICAL );
		rootSizer->Add(cameraSizer, wxSizerFlags().Expand().Border(wxALL, 2));
		rootSizer->Add(drawingSizer, wxSizerFlags().Expand().Border(wxALL, 2));
		rootSizer->Add(colorsSizer, wxSizerFlags().Expand().Border(wxALL, 2));

		sceneController->SetSizerAndFit(rootSizer);
	}

	lightingController = new wxWindow(this, wxID_ANY);	// 조명 제어
	animationController = new wxWindow(this, wxID_ANY);	// 애니메이션 제어

	screenControllerGroup = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	screenControllerGroup->AddPage(sceneController, "Scene");
	screenControllerGroup->AddPage(lightingController, "Lighting");
	screenControllerGroup->AddPage(animationController, "Animation");

	// 로그 윈도우
	logWindow = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_RICH | wxTE_MULTILINE | wxTE_AUTO_URL);
	logWindow->AppendText("this is log window\nhttp://icondb.com\n");

	// 메인 스크린
	renderWindow = new RenderWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, "Main Screen");

	// auiManager 초기화
	auiManager.SetManagedWindow(this);
	// 윈도우들 붙이기
	// Left (모델 관련 데이터들)
	auiManager.AddPane(nodeTree, wxAuiPaneInfo().Name("NodeTree").Caption("Node Tree")
		.Left().Layer(2).Position(1).CloseButton(false).MaximizeButton(false).BestSize(350,300));

	// Right (선택된 노드 정보)
	auiManager.AddPane(nodeProperties, wxAuiPaneInfo().Name("NodeProperties").Caption("Node Properties")
		.Right().Layer(0).Position(2).CloseButton(false).MaximizeButton(false).BestSize(350,300));

	// Bottom (스크린 제어)
	auiManager.AddPane(screenControllerGroup, wxAuiPaneInfo().Name("screenControllerGroup").Caption("Screen Controller")
		.Bottom().Layer(1).Position(1).CloseButton(true).MaximizeButton(false).BestSize(250,300));
	auiManager.AddPane(logWindow, wxAuiPaneInfo().Name("logWindow").Caption("Log")
		.Bottom().Layer(1).Position(3).CloseButton(true).MaximizeButton(false).BestSize(250,300));

	auiManager.AddPane(renderWindow, wxAuiPaneInfo().Name("screenWindow").CenterPane().PaneBorder(false));

	auiManager.GetPane(renderWindow).Show(true);
	auiManager.GetPane(nodeTree).Show(true);
	auiManager.GetPane(nodeProperties).Show(true);

	// create a menu
	wxMenu *fileMenu = new wxMenu;
	wxMenu *editMenu = new wxMenu;
	wxMenu *viewMenu = new wxMenu;
	wxMenu *toolsMenu = new wxMenu;
	wxMenu *helpMenu = new wxMenu;

	// 서브메뉴
	wxMenu* fileFormatMenu = new wxMenu;
	fileFormatMenu->AppendRadioItem(UICommandFileFormatXML, "XML with plain-text entity");
	fileFormatMenu->AppendRadioItem(UICommandFileFormatXMLb, "XML with encoded cdata entity");
	fileFormatMenu->AppendRadioItem(UICommandFileFormatBin, "Binary");
	fileFormatMenu->AppendRadioItem(UICommandFileFormatCBin, "Compressed Binary");

	fileMenu->Append(wxID_NEW, "&New", "Create New Model");
	fileMenu->Append(wxID_OPEN, "&Open...", "Open exist");
	fileMenu->AppendSeparator();
	fileMenu->AppendSubMenu(fileFormatMenu, "File Format");
	fileMenu->Append(wxID_SAVE, "&Save", "Save Model");
	fileMenu->Append(wxID_SAVEAS, "Save As...", "Save Model as new file");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, "E&xit\tAlt-X", "Quit this program");

	editMenu->Append(wxID_CUT, "Cu&t\tCtrl-X", "Cut selected model");
	editMenu->Append(wxID_COPY, "&Copy\tCtrl-C", "Copy selected model");
	editMenu->Append(wxID_PASTE, "&Paste\tCtrl-V", "Paste model into current selection");
	editMenu->Append(wxID_DELETE, "&Delete\tDel", "Delete selection");

	viewMenu->Append(UICommandViewNodeTree, "Node Tree", "View Node Tree");
	viewMenu->Append(UICommandViewResources, "Resources", "View External Resources");	
	viewMenu->Append(UICommandViewScreenController, "Screen Controller", "View Screen Controller");
	viewMenu->Append(UICommandViewAnimation, "Animation", "View Animation Controller");		
	viewMenu->Append(UICommandViewLog, "Log", "View Tree");			

	toolsMenu->Append(wxID_PREFERENCES, "&Options...", "User preferences");

	helpMenu->Append(wxID_ABOUT, "&About...\tF1", "Show about dialog");

	// create a menu bar
	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "&File");
	menuBar->Append(editMenu, "&Edit");
	menuBar->Append(viewMenu, "&View");
	menuBar->Append(toolsMenu, "&Tools");
	menuBar->Append(helpMenu, "&Help");

	SetMenuBar(menuBar);

	// create a status bar just for fun (by default with 1 pane only)
	CreateStatusBar(2);
	SetStatusText("Welcome to wxWidgets!");

	SetMinSize(wxSize(400,400));

	DKApplication* app = DKApplication::Instance();
	DKString configFile = DKDirectory::OpenDir(app->EnvironmentPath(DKApplication::SystemPathAppResource))->AbsolutePath() + "/" + (const wchar_t*)wxTheApp->GetAppName() + ".properties";
	configFile = configFile.FilePathString();
	int imported = DKPropertySet::DefaultSet().Import(configFile, true);
	DKLog("Config file: %ls (%d imported)\n", (const wchar_t*)configFile, imported);

	if (DKPropertySet::DefaultSet().Value("FrameRect").ValueType() == DKVariant::TypeVector4)
	{
		DKVector4 frameRectVec = DKPropertySet::DefaultSet().Value("FrameRect").Vector4();
		wxPoint framePos = wxPoint(frameRectVec.x, frameRectVec.y);
		wxSize frameSize = wxSize(Max<int>(frameRectVec.z, 400), Max<int>(frameRectVec.w, 400));

		this->SetPosition(framePos);
		this->SetSize(frameSize);
	}
	if (DKPropertySet::DefaultSet().Value("AUILayout").ValueType() == DKVariant::TypeString)
	{
		auiManager.LoadPerspective((const wchar_t*)DKPropertySet::DefaultSet().Value("AUILayout").String()); 
	}
	if (DKPropertySet::DefaultSet().Value("FileFormat").ValueType() == DKVariant::TypeInteger)
	{
		int fmt = DKPropertySet::DefaultSet().Value("FileFormat").Integer();
		switch (fmt)
		{
		case DKSerializer::SerializeFormXML:
		case DKSerializer::SerializeFormBinXML:
		case DKSerializer::SerializeFormBinary:
		case DKSerializer::SerializeFormCompressedBinary:
			this->fileFormat = (DKSerializer::SerializeForm)fmt;
			break;
		}
	}
	// 메뉴 갱신
	switch (this->fileFormat)
	{
	case DKSerializer::SerializeFormXML:				fileFormatMenu->Check(UICommandFileFormatXML, true); break;	
	case DKSerializer::SerializeFormBinXML:				fileFormatMenu->Check(UICommandFileFormatXMLb, true); break;	
	case DKSerializer::SerializeFormBinary:				fileFormatMenu->Check(UICommandFileFormatBin, true); break;	
	case DKSerializer::SerializeFormCompressedBinary:	fileFormatMenu->Check(UICommandFileFormatCBin, true); break;	
	default:
		wxFAIL_MSG("Wrong format!");
		break;
	}

	DKLog("File Format: %s\n", SerializeFormatString(this->fileFormat));

	// 다른곳에서 사용하기 위해 DKPropertySet::DefaultSet 에 저장함
	DKPropertySet::DefaultSet().SetValue("ConfigFile", configFile);
	DKPropertySet::DefaultSet().SetValue("FileFormat", (DKVariant::VInteger)fileFormat);

	// 프록시 윈도우 및 렌더러 생성은 auiManager.Update() 후에 한다!
	renderWindow->InitScreen();

	ProcessCommand(wxID_NEW);
}

MainWindow::~MainWindow(void)
{
	auiManager.UnInit();

	DKString configFile = DKPropertySet::DefaultSet().Value("ConfigFile").String();
	DKPropertySet::DefaultSet().Export(configFile, true);
}

void MainWindow::OnFileNew(wxCommandEvent& e)
{
	PrintLog("OnFileNew\n", wxColour(0,0,255));
	if (model && modified)
	{
		wxMessageDialog dlg(this, "Save?", "save file", wxYES_NO | wxCANCEL | wxYES_DEFAULT | wxICON_QUESTION);
		int ret = dlg.ShowModal();
		if (ret == wxID_YES)
		{
			// save
			ProcessCommand(wxID_SAVE);
		}
		else if (ret == wxID_CANCEL)
			return;
	}
	modified = false;
	modelPath = "";
	model = DKObject<DKModel>::New();

	renderWindow->frame.SetModel(model);

	nodeTree->RebuildTree();
	UpdateTitle();
}

void MainWindow::OnFileOpen(wxCommandEvent& e)
{
	bool mustSave = false;
	if (model && modified)
	{
		wxMessageDialog dlg(this, "Save?", "save file", wxYES_NO | wxCANCEL | wxYES_DEFAULT | wxICON_QUESTION);
		int ret = dlg.ShowModal();
		if (ret == wxID_YES)
		{
			// save
			mustSave = true;
			ProcessCommand(wxID_SAVE);
		}
		else if (ret == wxID_CANCEL)
			return;
	}
	if (mustSave && modified)		// 세이브가 제대로 되지 않았기 때문에 열지 않음.
		return;

	PrintLog("OnFileOpen\n", wxColour(0,0,255));
	wxFileDialog file(this, "Select Model", wxEmptyString, wxEmptyString, 
		"DKModel files (*.dkmodel)|*.dkmodel|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (file.ShowModal() == wxID_OK)
	{
		wxString filepath = file.GetPath();
		wxString filedir = file.GetDirectory();
		wxString filename = file.GetFilename();

		PrintLog(wxString::Format("open file: %ls\n", (const wchar_t*)filepath));
		PrintLog(wxString::Format("Set Temporary Resource Directory: %ls\n", (const wchar_t*)filedir));

		// 파일 로딩
		DKResourcePool pool;
		pool.AddSearchPath((const wchar_t*)filedir);

		DKObject<DKModel> m = pool.LoadResource((const wchar_t*)filename).SafeCast<DKModel>();
		if (m)
		{
			modelPath = (const wchar_t*)filepath;
			model = m;
			model->SetName((const wchar_t*)file.GetFilename());
			modified = false;

			nodeTree->RebuildTree();

			renderWindow->frame.SetModel(model);

			UpdateTitle();
		}
		else
		{
			PrintLog(wxString::Format("failed to load file: %ls\n", (const wchar_t*)filepath));
			wxMessageDialog dlg(this, wxString::Format("Failed to load file: \"%ls\"", (const wchar_t*)filename), 
				"Failed to load file", wxOK | wxICON_ERROR);
			dlg.ShowModal();
		}
	}
}

void MainWindow::OnFileSave(wxCommandEvent& e)
{
	PrintLog("OnFileSave\n", wxColour(0,0,255));
	if (modelPath.Length() == 0)
	{
		ProcessCommand(wxID_SAVEAS);
	}
	else
	{
		DKObject<DKData> data = model->Serialize(this->fileFormat);
		if (data)
		{
			DKObject<DKFile> file = DKFile::Create(modelPath, DKFile::ModeOpenNew, DKFile::ModeShareExclusive);
			if (file)
			{
				file->Write(data);
				file = NULL;
				modified = false;
				PrintLog(wxString::Format("%ls saved.(%llu bytes)\n", (const wchar_t*)modelPath, data->Length()), wxColour(0,0,255));
				UpdateTitle();
			}
			else
			{
				PrintLog(wxString::Format("Cannot open file:%ls.\n", (const wchar_t*)modelPath), wxColour(0,0,255));
			}
		}
		else
		{
			PrintLog(wxString::Format("Saving file:%ls failed.\n", (const wchar_t*)modelPath), wxColour(0,0,255));
		}
	}
}

void MainWindow::OnFileSaveAs(wxCommandEvent& e)
{
	wxFileDialog file(this, "Select File To Save Model", wxEmptyString, wxEmptyString, 
		"DKModel files (*.dkmodel)|*.dkmodel|All files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (file.ShowModal() == wxID_OK)
	{
		modelPath = (const wchar_t*)file.GetPath();
		ProcessCommand(wxID_SAVE);
	}
}

void MainWindow::OnFileFormat(wxCommandEvent& e)
{
	switch (e.GetId())
	{
	case UICommandFileFormatXML:
		this->fileFormat = DKSerializer::SerializeFormXML;
		break;
	case UICommandFileFormatXMLb:
		this->fileFormat = DKSerializer::SerializeFormBinXML;
		break;
	case UICommandFileFormatBin:
		this->fileFormat = DKSerializer::SerializeFormBinary;
		break;
	case UICommandFileFormatCBin:
		this->fileFormat = DKSerializer::SerializeFormCompressedBinary;
		break;
	}

	DKLog("File Format: %s\n", SerializeFormatString(this->fileFormat));
	DKPropertySet::DefaultSet().SetValue("FileFormat", (DKVariant::VInteger)fileFormat);
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

void MainWindow::OnViewNodeTree(wxCommandEvent& e)
{
	PrintLog("OnViewNodeTree\n", wxColour(0,0,255));
}

void MainWindow::OnViewResources(wxCommandEvent& e)
{
	PrintLog("OnViewResources\n", wxColour(0,0,255));
}

void MainWindow::OnViewScreenController(wxCommandEvent& e)
{
	PrintLog("OnViewScreenController\n", wxColour(0,0,255));
}

void MainWindow::OnViewAnimation(wxCommandEvent& e)
{
	PrintLog("OnViewAnimation\n", wxColour(0,0,255));
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
		"DK ModelEditor\n\n"
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
	sceneController->Layout();
}

void MainWindow::OnClose(wxCloseEvent& e)
{
	if (model && modified)
	{
		wxMessageDialog dlg(this, "Save?", "save file", wxYES_NO | wxCANCEL | wxYES_DEFAULT | wxICON_QUESTION);
		int ret = dlg.ShowModal();
		if (ret == wxID_YES)
		{
			// save
			ProcessCommand(wxID_SAVE);
		}
		else if (ret == wxID_CANCEL)
		{
			e.Veto();
			return;
		}
	}
	model = NULL;

	e.Skip();

	wxPoint framePos = GetPosition();
	wxSize frameSize = GetSize();

	DKPropertySet::DefaultSet().SetValue("FrameRect", DKVariant::VVector4(framePos.x, framePos.y, frameSize.x, frameSize.y));
	DKPropertySet::DefaultSet().SetValue("AUILayout", (const wchar_t*)auiManager.SavePerspective());
	DKPropertySet::DefaultSet().SetValue("FileFormat", (DKVariant::VInteger)fileFormat);
	
	nodeTree->Close(true);
	nodeProperties->Close(true);
	renderWindow->Close(true);
}

void MainWindow::PrintLog(const wxString& msg, const wxColour& c)
{
	logWindow->SetDefaultStyle(wxTextAttr(c));
	logWindow->AppendText(msg);
}

DKModel* MainWindow::GetModel(void)
{
	return model;
}

void MainWindow::SetModelModified(void)
{
	if (modified == false)
	{
		modified = true;
		UpdateTitle();

		renderWindow->frame.SetModel(displayModel);
	}
}

void MainWindow::UpdateTitle(void)
{
	DKString name = modelPath;
	if (name.Length() == 0)
		name = model->Name();
	if (name.Length() == 0)
		name = "Untitled Model";

	wxString title = wxString::Format("%ls%s - %ls",
		(const wchar_t*)name,
		modified ? "*" : "",
		(const wchar_t*)wxTheApp->GetAppName());

	SetTitle(title);
}

DKObject<DKMaterial> MainWindow::OpenMaterial(void)
{
	DKObject<DKMaterial> mat = NULL;

	wxFileDialog file(this, "Select Material", wxEmptyString, wxEmptyString, 
		"DKMaterial files (*.dkmaterial)|*.dkmaterial|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (file.ShowModal() == wxID_OK)
	{
		wxString filepath = file.GetPath();
		wxString filedir = file.GetDirectory();
		wxString filename = file.GetFilename();

		PrintLog(wxString::Format("open file: %ls\n", (const wchar_t*)filepath));
		PrintLog(wxString::Format("Set Temporary Resource Directory: %ls\n", (const wchar_t*)filedir));

		// 리소스 로딩
		DKResourcePool pool;
		pool.AddSearchPath((const wchar_t*)filedir);
		mat = pool.LoadResource((const wchar_t*)filename).SafeCast<DKMaterial>();

		if (mat)
		{
			mat->SetName((const wchar_t*)filename);
			DKMaterial::BuildLog log;
			if (mat->Build(&log))
			{
				PrintLog(wxString::Format("Material (%ls) Build Succeed.\n", (const wchar_t*)filename));
			}
			else
			{
				PrintLog(wxString::Format("Material Build Error: %ls\n", (const wchar_t*)log.errorLog));
				PrintLog(wxString::Format("Error on Shader: %ls\n", (const wchar_t*)log.failedShader));
			}
		}
		else
		{
			PrintLog(wxString::Format("failed to load file: %ls\n", (const wchar_t*)filepath));
			wxMessageDialog dlg(this, wxString::Format("Failed to load file:%ls", (const wchar_t*)filename), 
				"Failed to load file", wxOK | wxICON_ERROR);
			dlg.ShowModal();
		}
	}
	return mat;
}

DKObject<DKTexture> MainWindow::OpenTexture(void)
{
	DKObject<DKTexture> tex = NULL;

	wxFileDialog file(this, "Select Texture", wxEmptyString, wxEmptyString,
		"All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (file.ShowModal() == wxID_OK)
	{
		wxString filepath = file.GetPath();
		wxString filedir = file.GetDirectory();
		wxString filename = file.GetFilename();

		PrintLog(wxString::Format("open file: %ls\n", (const wchar_t*)filepath));
		PrintLog(wxString::Format("Set Temporary Resource Directory: %ls\n", (const wchar_t*)filedir));

		// 리소스 로딩
		DKResourcePool pool;
		pool.AddSearchPath((const wchar_t*)filedir);
		tex = pool.LoadResource((const wchar_t*)filename).SafeCast<DKTexture>();

		if (tex == NULL)
		{
			PrintLog(wxString::Format("failed to load file: %ls\n", (const wchar_t*)filepath));
			wxMessageDialog dlg(this, wxString::Format("Failed to load file:%ls", (const wchar_t*)filename),
				"Failed to load file", wxOK | wxICON_ERROR);
			dlg.ShowModal();
		}
	}
	return tex;
}

void MainWindow::LockRenderer(void) const
{
	renderWindow->frame.Lock();
}

void MainWindow::UnlockRenderer(void) const
{
	renderWindow->frame.Unlock();
}

void MainWindow::SetActiveItem(ModelNodeItemData* item)
{
	nodeProperties->SetActiveNode(item);

	displayModel = NULL;
	if (item)
	{
		switch (item->type)
		{
		case ModelNodeItemData::TypeModel:
		case ModelNodeItemData::TypeRigidBody:
		case ModelNodeItemData::TypeSoftBody:
		case ModelNodeItemData::TypeMesh:
			if (true)
			{
				DKModel* m = item->GetModel();
				displayModel = m->Clone();
				displayModel->SetWorldTransform(m->WorldTransform());
			}
			break;
		case ModelNodeItemData::TypeShape:
			if (true)
			{
				// 선택된 물리 객체는 rigid-body 로 보여준다.
				DKModel* m = item->parent->GetModel();
				DKCollisionShape* shape = static_cast<ModelNodeShapeData*>(item)->shape;
				DKObject<DKCompoundShape> compound = DKObject<DKCompoundShape>::New();
				compound->AddShape(shape, item->Transform());

				displayModel = DKOBJECT_NEW DKRigidBody(compound);
				displayModel->SetWorldTransform(m->WorldTransform());
			}
			break;
		case ModelNodeItemData::TypeConstraint:
			if (true)
			{
				// 선택된 constraint 는 월드에 정적으로 연결하여 보여준다.
			}
			break;
		default:
			wxFAIL_MSG("Unknown object!");
			break;
		}
	}

	//if (displayModel)
	//	displayModel->ResetWorldTransform();
	renderWindow->frame.SetModel(displayModel);
}

void MainWindow::UpdateTreeItem(wxTreeItemId item)
{
	nodeTree->UpdateTreeItem(item, false);
	nodeTree->SortEnclosingItem(item);
}

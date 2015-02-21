#include "StdAfx.h"
#include "MainWindow.h"
#
#if !defined(__WXMSW__) && !defined(__WXPM__)
#include "DKMaterialEditor.xpm"
#endif

#include <wx/imaglist.h>
#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/image.h>

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(wxID_NEW,							MainWindow::OnFileNew)
	EVT_MENU(wxID_OPEN,							MainWindow::OnFileOpen)
	EVT_MENU(wxID_SAVE,							MainWindow::OnFileSave)
	EVT_MENU(wxID_SAVEAS,						MainWindow::OnFileSaveAs)
	EVT_MENU(wxID_EXIT,							MainWindow::OnFileQuit)

	EVT_MENU(wxID_CUT,							MainWindow::OnEditCut)
	EVT_MENU(wxID_COPY,							MainWindow::OnEditCopy)
	EVT_MENU(wxID_PASTE,						MainWindow::OnEditPaste)
	EVT_MENU(wxID_DELETE,						MainWindow::OnEditDelete)

	EVT_MENU(UICommandViewRenderProperties,		MainWindow::OnViewRenderProperties)
	EVT_MENU(UICommandViewMaterialProperties,	MainWindow::OnViewMaterialProperties)
	EVT_MENU(UICommandViewLightingProperties,	MainWindow::OnViewLightingProperties)
	EVT_MENU(UICommandViewSceneProperties,		MainWindow::OnViewSceneProperties)
	EVT_MENU(UICommandViewLog,					MainWindow::OnViewLog)

	EVT_MENU(wxID_PREFERENCES,					MainWindow::OnToolsOptions)

    EVT_MENU(wxID_ABOUT,						MainWindow::OnHelpAbout)

	EVT_SIZE(									MainWindow::OnSize)
	EVT_CLOSE(									MainWindow::OnClose)
wxEND_EVENT_TABLE();

MainWindow::MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxFrame(NULL, wxID_ANY, title, pos, size, style)
	, material(NULL)
	, modified(false)
	, materialPath("")
{
	// set the frame icon
	SetIcon(wxICON(DKMaterialEditor));

	// 메인 스크린
	renderWindow = new RenderWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, "Main Screen");

	// 프로퍼티 제어 윈도우
	materialTree = new MaterialTree(this, wxID_ANY);
	materialProperties = new MaterialProperties(this, wxID_ANY);
	sceneProperties = new wxWindow(this, wxID_ANY);
	// 에디터
	shaderEditor = new ShaderEditor(this, wxID_ANY);

	// 로그 윈도우
	logWindow = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(400, 260), wxTE_RICH | wxTE_MULTILINE | wxTE_AUTO_URL);
	logWindow->AppendText("this is log window\nhttp://icondb.com\n");

	// auiManager 초기화
	auiManager.SetManagedWindow(this);
	// 윈도우들 붙이기
	// Left
	auiManager.AddPane(materialTree, wxAuiPaneInfo().Name("materialTree").Caption("Material")
		.Left().Layer(2).Position(0).CloseButton(false).MaximizeButton(false).BestSize(200,400));
	auiManager.AddPane(materialProperties, wxAuiPaneInfo().Name("materialProperties").Caption("Properties")
		.Left().Layer(2).Position(1).CloseButton(false).MaximizeButton(false).BestSize(200,400));
	// Right
	auiManager.AddPane(renderWindow, wxAuiPaneInfo().Name("screenWindow").Caption("Render")
		.Right().Layer(0).Position(0).CloseButton(true).MaximizeButton(false).BestSize(400,400));
	// Bottom
	auiManager.AddPane(sceneProperties, wxAuiPaneInfo().Name("sceneProperties").Caption("Scene")
		.Bottom().Layer(1).Position(0).CloseButton(true).MaximizeButton(false).BestSize(400,200));
	auiManager.AddPane(logWindow, wxAuiPaneInfo().Name("logWindow").Caption("Log")
		.Bottom().Layer(1).Position(1).CloseButton(true).MaximizeButton(false).BestSize(200,200));

	auiManager.AddPane(shaderEditor, wxAuiPaneInfo().Name("shaderEditor").CenterPane().PaneBorder(false));

	// 프록시 윈도우 및 렌더러 생성은 auiManager.Update() 후에 한다!
	renderWindow->InitScreen();

	// create a menu
	wxMenu *fileMenu = new wxMenu;
	wxMenu *editMenu = new wxMenu;
	wxMenu *viewMenu = new wxMenu;
	wxMenu *toolsMenu = new wxMenu;
	wxMenu *helpMenu = new wxMenu;

	fileMenu->Append(wxID_NEW, "&New", "Create New Material");
	fileMenu->Append(wxID_OPEN, "&Open...", "Open exist");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_SAVE, "&Save", "Save Material");
	fileMenu->Append(wxID_SAVEAS, "Save As...", "Save Material As...");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, "E&xit\tAlt-X", "Quit Program");

	editMenu->Append(wxID_CUT, "Cu&t\tCtrl-X", "Cut selected model");
	editMenu->Append(wxID_COPY, "&Copy\tCtrl-C", "Copy selected model");
	editMenu->Append(wxID_PASTE, "&Paste\tCtrl-V", "Paste model into current selection");
	editMenu->Append(wxID_DELETE, "&Delete\tDel", "Delete selection");

	viewMenu->Append(UICommandViewRenderProperties, "Render Properties", "View Rener Window");
	viewMenu->Append(UICommandViewMaterialProperties, "Material Properties", "View Material Window");	
	viewMenu->Append(UICommandViewLightingProperties, "Lighting Properties", "View Lighting Window");
	viewMenu->Append(UICommandViewSceneProperties, "Scene Properties", "View Scene Window");		
	viewMenu->Append(UICommandViewScreenSelector, "Screen Selector", "View Screen Selector");
	viewMenu->Append(UICommandViewLog, "Log", "View Log Window");			

	toolsMenu->Append(wxID_PREFERENCES, "&Options...", "User preferences");

    helpMenu->Append(wxID_ABOUT, "&About...\tF1", "Show about dialog");

	// create a menu bar
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
	//menuBar->Append(editMenu, "&Edit");		// 에디터(wxScintilla)에서 단축키가 작동 안해서 제외함
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
	DKPropertySet::DefaultSet().SetValue("ConfigFile", configFile);

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
	if (material && modified)
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

	materialProperties->SetActiveNode(NULL);
	shaderEditor->CloseAll();
	modified = false;
	materialPath = "";
	material = DKObject<DKMaterial>::New();

	materialTree->RebuildTree();

	UpdateTitle();
}

void MainWindow::OnFileOpen(wxCommandEvent& e)
{
	bool mustSave = false;
	if (material && modified)
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

		DKObject<DKMaterial> m = pool.LoadResource((const wchar_t*)filename).SafeCast<DKMaterial>();
		if (m)
		{
			materialProperties->SetActiveNode(NULL);
			shaderEditor->CloseAll();
			materialPath = (const wchar_t*)filepath;
			material = m;
			material->SetName((const wchar_t*)file.GetFilename());
			materialTree->RebuildTree();
			modified = false;
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
	if (materialPath.Length() == 0)
	{
		ProcessCommand(wxID_SAVEAS);
	}
	else
	{
		DKObject<DKData> data = material->Serialize(DKSerializer::SerializeFormXML);
		if (data)
		{
			DKObject<DKFile> file = DKFile::Create(materialPath, DKFile::ModeOpenNew, DKFile::ModeShareExclusive);
			if (file)
			{
				file->Write(data);
				file = NULL;
				modified = false;
				PrintLog(wxString::Format("%ls saved.(%d bytes)\n", (const wchar_t*)materialPath, data->Length()), wxColour(0,0,255));
				UpdateTitle();
			}
			else
			{
				PrintLog(wxString::Format("Cannot open file:%ls.\n", (const wchar_t*)materialPath), wxColour(0,0,255));
			}
		}
		else
		{
			PrintLog(wxString::Format("Saving file:%ls failed.\n", (const wchar_t*)materialPath), wxColour(0,0,255));
		}
	}
}

void MainWindow::OnFileSaveAs(wxCommandEvent& e)
{
	wxFileDialog file(this, "Select File To Save Material", wxEmptyString, wxEmptyString, 
		"DKMaterial files (*.dkmaterial)|*.dkmaterial|All files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (file.ShowModal() == wxID_OK)
	{
		materialPath = (const wchar_t*)file.GetPath();
		ProcessCommand(wxID_SAVE);
	}
}

void MainWindow::OnFileQuit(wxCommandEvent& e)
{
	e.Skip();
	Close(true);
}

void MainWindow::OnEditCut(wxCommandEvent& e)
{
	PrintLog(wxString::Format("%s\n", DKLIB_FUNCTION_NAME), wxColour(0,0,255));
}

void MainWindow::OnEditCopy(wxCommandEvent& e)
{
	PrintLog(wxString::Format("%s\n", DKLIB_FUNCTION_NAME), wxColour(0,0,255));
}

void MainWindow::OnEditPaste(wxCommandEvent& e)
{
	PrintLog(wxString::Format("%s\n", DKLIB_FUNCTION_NAME), wxColour(0,0,255));
}

void MainWindow::OnEditDelete(wxCommandEvent& e)
{
	PrintLog(wxString::Format("%s\n", DKLIB_FUNCTION_NAME), wxColour(0,0,255));
}

void MainWindow::OnViewRenderProperties(wxCommandEvent& e)
{
	PrintLog(wxString::Format("%s\n", DKLIB_FUNCTION_NAME), wxColour(0,0,255));
}

void MainWindow::OnViewMaterialProperties(wxCommandEvent& e)
{
	PrintLog(wxString::Format("%s\n", DKLIB_FUNCTION_NAME), wxColour(0,0,255));
}

void MainWindow::OnViewLightingProperties(wxCommandEvent& e)
{
	PrintLog(wxString::Format("%s\n", DKLIB_FUNCTION_NAME), wxColour(0,0,255));
}

void MainWindow::OnViewSceneProperties(wxCommandEvent& e)
{
	PrintLog(wxString::Format("%s\n", DKLIB_FUNCTION_NAME), wxColour(0,0,255));
}

void MainWindow::OnViewLog(wxCommandEvent& e)
{
	PrintLog(wxString::Format("%s\n", DKLIB_FUNCTION_NAME), wxColour(0,0,255));
}

void MainWindow::OnToolsOptions(wxCommandEvent& e)
{
	PrintLog(wxString::Format("%s\n", DKLIB_FUNCTION_NAME), wxColour(0,0,255));
}

void MainWindow::OnHelpAbout(wxCommandEvent& e)
{
	wxMessageBox(wxString::Format(
		"DK MaterialEditor\n\n"
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
	if (material && modified)
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
	e.Skip();

	wxPoint framePos = GetPosition();
	wxSize frameSize = GetSize();

	DKPropertySet::DefaultSet().SetValue("FrameRect", DKVariant::VVector4(framePos.x, framePos.y, frameSize.x, frameSize.y));
	DKPropertySet::DefaultSet().SetValue("AUILayout", (const wchar_t*)auiManager.SavePerspective());

	renderWindow->Close(true);
}

void MainWindow::PrintLog(const wxString& msg, const wxColour& c)
{
	logWindow->SetDefaultStyle(wxTextAttr(c));
	logWindow->AppendText(msg);
}

DKMaterial* MainWindow::GetMaterial(void)
{
	return material;
}

void MainWindow::SetMaterialModified(void)
{
	if (modified == false)
	{
		modified = true;

		UpdateTitle();
	}
}

void MainWindow::OpenEditor(DKMaterial::ShaderSource* shader)
{
	shaderEditor->Open(shader);
}

bool MainWindow::CloseEditor(DKMaterial::ShaderSource* shader)
{
	return shaderEditor->Close(shader);
}

void MainWindow::UpdateEditor(DKMaterial::ShaderSource* shader)
{
	shaderEditor->Update(shader);
}

void MainWindow::UpdateTitle(void)
{
	DKString name = materialPath;
	if (name.Length() == 0)
		name = material->Name();
	if (name.Length() == 0)
		name = "Untitled Material";

	wxString title = wxString::Format("%ls%s - %ls",
		(const wchar_t*)name,
		modified ? "*" : "",
		(const wchar_t*)wxTheApp->GetAppName());

	SetTitle(title);
}

void MainWindow::OpenProperties(MaterialNodeItemData* item)
{
	materialProperties->SetActiveNode(item);
}

void MainWindow::UpdateTreeItem(wxTreeItemId item)
{
	materialTree->UpdateTreeItem(item, false);
}

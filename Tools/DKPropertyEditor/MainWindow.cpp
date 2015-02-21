#include "StdAfx.h"
#include <wx/artprov.h>
#include "MainWindow.h"

#if !defined(__WXMSW__) && !defined(__WXPM__)
#include "DKPropertyEditor.xpm"
#endif


wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(wxID_NEW,								MainWindow::OnFileNew)
	EVT_MENU(wxID_OPEN,								MainWindow::OnFileOpen)
	EVT_MENU(wxID_SAVE,								MainWindow::OnFileSave)
	EVT_MENU(wxID_SAVEAS,							MainWindow::OnFileSaveAs)
	EVT_MENU(UICommandFileImportKey,				MainWindow::OnFileImportKey)
	EVT_MENU(UICommandFileExportKey,				MainWindow::OnFileExportKey)
	EVT_MENU(UICommandFileClose,					MainWindow::OnFileClose)
	EVT_MENU(wxID_EXIT,								MainWindow::OnFileQuit)

	EVT_MENU(wxID_UNDO,								MainWindow::OnEditUndo)
    EVT_MENU(wxID_REDO,								MainWindow::OnEditRedo)
	EVT_MENU(UICommandEditNew,						MainWindow::OnEditNew)
	EVT_MENU(wxID_CUT,								MainWindow::OnEditCut)
	EVT_MENU(wxID_COPY,								MainWindow::OnEditCopy)
	EVT_MENU(wxID_PASTE,							MainWindow::OnEditPaste)
	EVT_MENU(wxID_DUPLICATE,						MainWindow::OnEditDuplicate)
	EVT_MENU(wxID_DELETE,							MainWindow::OnEditDelete)
	EVT_MENU(wxID_SELECTALL,						MainWindow::OnEditSelectAll)
	EVT_MENU(UICommandEditFormatXML,				MainWindow::OnEditFormatXML)

	EVT_MENU(UICommandWindowClose,					MainWindow::OnWindowClose)
	EVT_MENU(UICommandWindowCloseOthers,			MainWindow::OnWindowCloseOthers)
	EVT_MENU(UICommandWindowCloseAll,				MainWindow::OnWindowCloseAll)

	EVT_MENU(wxID_ABOUT,							MainWindow::OnHelpAbout)

	EVT_MENU(UICommandKeyListOpen,					MainWindow::OnKeyListOpen)
	EVT_MENU(UICommandKeyListNew,					MainWindow::OnKeyListNew)
	EVT_MENU(UICommandKeyListCut,					MainWindow::OnKeyListCut)
	EVT_MENU(UICommandKeyListCopy,					MainWindow::OnKeyListCopy)
	EVT_MENU(UICommandKeyListPaste,					MainWindow::OnKeyListPaste)
	EVT_MENU(UICommandKeyListDuplicate,				MainWindow::OnKeyListDuplicate)
	EVT_MENU(UICommandKeyListDelete,				MainWindow::OnKeyListDelete)
	EVT_MENU(UICommandKeyListSelectAll,				MainWindow::OnKeyListSelectAll)

	EVT_LIST_END_LABEL_EDIT(UIControlKeyList,		MainWindow::OnEndEditKey)
	EVT_LIST_ITEM_ACTIVATED(UIControlKeyList,		MainWindow::OnActivateKey)

	EVT_MENU_OPEN(									MainWindow::OnMenuOpen)
	EVT_SIZE(										MainWindow::OnSize)
	EVT_CLOSE(										MainWindow::OnClose)
	EVT_CONTEXT_MENU(								MainWindow::OnContextMenu)

	EVT_VARIANT_EDITOR_MODIFIED(wxID_ANY,			MainWindow::OnVariantModified)
wxEND_EVENT_TABLE();

namespace
{
	const wxString fileDialogFilterPSet = "property files (*.pset;*.properties;*.propertySet;*.igpropertyset)|*.pset;*.properties;*.propertySet;*.igpropertyset|XML files (*.xml)|*.xml|All files (*.*)|*.*";
	const wxString fileDialogFilterVar = "variant files (*.var;*.variant;*.igvariant)|*.var;*.variant;*.igvariant|XML files (*.xml)|*.xml| All files (*.*)|*.*";
}

MainWindow::MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxFrame(NULL, wxID_ANY, title, pos, size, style)
	, exportXML(false)
	, readonly(false)
	, maxUndoLevel(10)
	, filePath("")
	, workingSetIndex(0)
{
	// set the frame icon
	SetIcon(wxICON(DKPropertyEditor));
	// auiManager 초기화
	auiManager.SetManagedWindow(this);

	// create a menu
	fileMenu = new wxMenu;
	editMenu = new wxMenu;
	windowMenu = new wxMenu;
	helpMenu = new wxMenu;

	// file submenu
	fileMenu->Append(wxID_NEW, "New", "New PropertySet");
	fileMenu->Append(wxID_OPEN, "Open...\tCTRL-O", "Open exist");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_SAVE, "Save\tCtrl-S", "Save Model");
	fileMenu->Append(wxID_SAVEAS, "Save As...\tCTRL-SHIFT-S", "Save Model as new file");
	fileMenu->AppendSeparator();
	fileMenu->Append(UICommandFileImportKey, "Import Key...", "Import Single Variant as New Key");
	fileMenu->Append(UICommandFileExportKey, "Export Key...", "Export Single Variant File");
	fileMenu->AppendSeparator();
	fileMenu->Append(UICommandFileClose, "Close Window\tCTRL-W", "Close Window");
	fileMenu->Append(wxID_EXIT, "E&xit\tALT-X", "Quit this program");

	editMenu->Append(wxID_UNDO, "Undo\tCTRL-Z", "Undo");
	editMenu->Append(wxID_REDO, "Redo\tCTRL-SHIFT-Z", "Redo");
	editMenu->AppendSeparator();
	editMenu->Append(UICommandEditNew, "New Key\tCTRL-N", "Insert New Key.");
	editMenu->AppendSeparator();
	editMenu->AppendCheckItem(UICommandEditFormatXML, "XML Format", "Save as XML format");
	editMenu->AppendSeparator();
	editMenu->Append(wxID_CUT, "Cut\tCTRL-X", "Cut Selected Key.");
	editMenu->Append(wxID_COPY, "Copy\tCTRL-C", "Copy Selected Key.");
	editMenu->Append(wxID_PASTE, "Paste\tCTRL-V", "Paste Key from Clipboard.");
	editMenu->Append(wxID_DUPLICATE, "Duplicate\tCTRL-D", "Duplicate Selected Key.");
	editMenu->AppendSeparator();
	editMenu->Append(wxID_DELETE, "Delete\tCTRL-BACK", "Delete Selected Key.");

	windowMenu->Append(UICommandWindowClose, "Close window", "Close current window");
	windowMenu->Append(UICommandWindowCloseOthers, "Close other windows", "Close other windows");
	windowMenu->Append(UICommandWindowCloseAll, "Close All", "Close all windows");

	helpMenu->Append(wxID_ABOUT, "&About...\tF1", "Show about dialog");

	// create a menu bar
	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "&File");
	menuBar->Append(editMenu, "&Edit");
	menuBar->Append(windowMenu, "&Window");
	menuBar->Append(helpMenu, "&Help");
	SetMenuBar(menuBar);

	// create a status bar just for fun (by default with 1 pane only)
	//CreateStatusBar(2);
	//SetStatusText("Welcome to wxWidgets!");

	SetMinSize(wxSize(400,400));

	// property-key list box
	wxImageList* keyImageList = new wxImageList(16, 16, true, 2);
	keyImageList->Add(wxArtProvider::GetBitmap(wxART_HELP_BOOK, wxART_OTHER, wxSize(16,16)));
	keyImageList->Add(wxArtProvider::GetBitmap(wxART_ERROR, wxART_OTHER, wxSize(16,16)));

	keyList = new wxListCtrl(this, UIControlKeyList, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_EDIT_LABELS | wxLC_SORT_DESCENDING);
	keyList->AssignImageList(keyImageList, wxIMAGE_LIST_SMALL);
	keyList->InsertColumn(0, "key", wxLIST_FORMAT_LEFT, 120);
	keyList->InsertColumn(1, "type", wxLIST_FORMAT_LEFT, 80);

	// variant editor notebook
	notebook = new wxAuiNotebook(this, UIControlEditorNotebook, wxDefaultPosition, wxDefaultSize,
								 wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_WINDOWLIST_BUTTON | wxAUI_NB_CLOSE_ON_ACTIVE_TAB | wxAUI_NB_SCROLL_BUTTONS );

	auiManager.AddPane(keyList, wxAuiPaneInfo().Name("KeyList").CaptionVisible(false)
		.Left().Layer(1).Position(0)
		.CloseButton(false).MaximizeButton(false).MinimizeButton(false)
		.Floatable(false)
		.BestSize(200,400));
	auiManager.AddPane(notebook, wxAuiPaneInfo().Name("VarEditor").CenterPane().PaneBorder(false));

	DKApplication* app = DKApplication::Instance();
	DKString configFile = DKDirectory::OpenDir(app->EnvironmentPath(DKApplication::SystemPathAppResource))->AbsolutePath() + "/" + (const wchar_t*)wxTheApp->GetAppName() + ".properties";
	DKPropertySet::DefaultSet().Import(configFile, true);
	DKPropertySet::DefaultSet().SetValue("ConfigFile", configFile);

	if (DKPropertySet::DefaultSet().Value("ExportXML").ValueType() == DKVariant::TypeInteger)
	{
		exportXML = DKPropertySet::DefaultSet().Value("ExportXML").Integer() != 0;
	}
	if (DKPropertySet::DefaultSet().Value("FrameRect").ValueType() == DKVariant::TypeVector4)
	{
		DKVector4 frameRectVec = DKPropertySet::DefaultSet().Value("FrameRect").Vector4();
		wxPoint framePos = wxPoint(frameRectVec.x, frameRectVec.y);
		wxSize frameSize = wxSize(Max<int>(frameRectVec.z, 400), Max<int>(frameRectVec.w, 400));

		this->SetPosition(framePos);
		this->SetSize(frameSize);
	}
	if (DKPropertySet::DefaultSet().Value("KeyListColumnWidth").ValueType() == DKVariant::TypeArray)
	{
		const DKVariant::VArray& columnArray = DKPropertySet::DefaultSet().Value("KeyListColumnWidth").Array();
		for (size_t i = 0; i < columnArray.Count() && i < keyList->GetColumnCount(); i++)
		{
			if (columnArray.Value(i).ValueType() == DKVariant::TypeInteger)
				keyList->SetColumnWidth((int)i, Max<int>((int)columnArray.Value(i).Integer(), 0));
		}
	}
	if (DKPropertySet::DefaultSet().Value("AUILayout").ValueType() == DKVariant::TypeString)
	{
		auiManager.LoadPerspective((const wchar_t*)DKPropertySet::DefaultSet().Value("AUILayout").String()); 
	}
	if (DKPropertySet::DefaultSet().Value("MaxUndoLevel").ValueType() == DKVariant::TypeInteger)
	{
		this->maxUndoLevel = Max<size_t>(DKPropertySet::DefaultSet().Value("MaxUndoLevel").Integer(), 10);
	}

	DKLog("Export XML: %d\n", (int)exportXML);
	editMenu->Check(UICommandEditFormatXML, exportXML);

	DKLog("MaxUndoLevel: %lu\n", maxUndoLevel);

	lastSavedPropertyMap.Clear();
	ResetRevisionHistory(lastSavedPropertyMap);

	ProcessCommand(wxID_NEW);
}

MainWindow::~MainWindow(void)
{
	auiManager.UnInit();

	DKString configFile = DKPropertySet::DefaultSet().Value(L"ConfigFile").String();
	DKPropertySet::DefaultSet().Export(configFile, true);
}

void MainWindow::OnFileNew(wxCommandEvent& e)
{
	if (IsModified())
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

	filePath = "";

	lastSavedPropertyMap.Clear();
	ResetRevisionHistory(lastSavedPropertyMap);

	CloseAllEditors();
	UpdateTitle();
	ReloadData();
}

void MainWindow::OnFileOpen(wxCommandEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);

	if (IsModified())
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

	bool reload = false;

	wxFileDialog file(this, "Select PropertySet", wxEmptyString, wxEmptyString, fileDialogFilterPSet, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (file.ShowModal() == wxID_OK)
	{
		DKTimer timer;
		timer.Reset();
		DKObject<DKPropertySet> prop = DKObject<DKPropertySet>::New();
		int numImported = prop->Import((const wchar_t*)file.GetPath(), true);

		DKLog("%d items imported. (elapsed: %f)\n", numImported, timer.Elapsed());
		if (numImported < 0)
		{
			DKLog("[%s] Error: Invalid format.\n", DKLIB_FUNCTION_NAME);
			wxMessageBox("Invalid format.", "Error");
		}
		else
		{
			CloseAllEditors();

			struct KeyExtract
			{
				KeyExtract(PropertyMap& p) : pmap(p) {}
				void operator () (const DKString& key, const DKVariant& value)
				{
					DKObject<VariantObject> vo = DKOBJECT_NEW VariantObject(value);
					pmap.Insert(key, vo);
				}
				PropertyMap& pmap;
			};
			PropertyMap propertyMap;
			prop->EnumerateForward(DKFunction(KeyExtract(propertyMap)));

			lastSavedPropertyMap = propertyMap;
			ResetRevisionHistory(lastSavedPropertyMap);

			filePath = file.GetPath();
			reload = true;
		}
	}
	if (reload)
	{
		UpdateTitle();
		ReloadData();
	}
}

void MainWindow::OnFileSave(wxCommandEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);

	if (filePath.Length() == 0)
	{
		ProcessCommand(wxID_SAVEAS);
	}
	else
	{
		DKPropertySet pset;
		CurrentWorkingSet().EnumerateForward([&pset](PropertyMap::Pair& pair)
		{
			pset.SetValue(pair.key, pair.value->Value());
		});

		DKTimer timer;
		timer.Reset();

		int numExported = pset.Export((const wchar_t*)filePath, exportXML);

		DKLog("%d items exported. (elapsed: %f)\n", numExported, timer.Elapsed());
		if (numExported < 0)
		{
			// error
			DKLog("export failed.(%d)\n", numExported);
			wxMessageBox("Cannot save file.", "Error");
		}
		else
		{
			DKLog("%ls exported.(%d items, %s format.)\n", (const wchar_t*)filePath, numExported, this->exportXML ? "XML" : "Binary");
			lastSavedPropertyMap = CurrentWorkingSet();
			ReloadData();
			UpdateTitle();	
		}
	}
}

void MainWindow::OnFileSaveAs(wxCommandEvent& e)
{
	wxFileDialog file(this, "Select File To Save", wxEmptyString, wxEmptyString, fileDialogFilterPSet, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (file.ShowModal() == wxID_OK)
	{
		filePath = (const wchar_t*)file.GetPath();
		ProcessCommand(wxID_SAVE);
	}
}

void MainWindow::OnFileImportKey(wxCommandEvent& e)
{
	wxFileDialog file(this, "Select Variant File", wxEmptyString, wxEmptyString, fileDialogFilterVar, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (file.ShowModal() == wxID_OK)
	{
		DKObject<DKVariant> importVar = NULL;
		// DKVariant (XML) 로 연다.
		DKObject<DKXMLDocument> doc = DKXMLDocument::Open(DKXMLDocument::TypeXML, (const wchar_t*)file.GetPath());
		if (doc && doc->RootElement())
		{
			DKObject<DKVariant> var = DKObject<DKVariant>::New();
			if (var->ImportXML(doc->RootElement()))
			{
				importVar = var;
			}
		}
		else	// DKVariant (Bin) 으로 연다.
		{
			DKObject<DKVariant> var = DKObject<DKVariant>::New();
			if (var->ImportStream(DKFile::Create((const wchar_t*)file.GetPath(), DKFile::ModeOpenReadOnly, DKFile::ModeShareAll)))
			{
				importVar = var;
			}
		}

		if (importVar)
		{
			DKObject<VariantObject> vo = DKOBJECT_NEW VariantObject(*importVar);
			PropertyMap nextRev = CurrentWorkingSet();
			for (unsigned int i = 1; true; i++)
			{
				DKString key = DKString::Format("Imported Key %d", i);
				if (nextRev.Insert(key, vo))
				{
					PushRevisionWithModifiedKey(nextRev, L"", key);
					break;
				}
			}

			ReloadData();
			UpdateTitle();
		}
		else
		{
			DKLog("[%s] Error: Invalid format.\n", DKLIB_FUNCTION_NAME);
			wxMessageBox("Invalid format.", "Error");
		}
	}	
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);	
}

void MainWindow::OnFileExportKey(wxCommandEvent& e)
{
	if (keyList->GetSelectedItemCount() == 1)
	{
		long index = keyList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (index >= 0)
		{
			const PropertyMap::Pair* p = reinterpret_cast<const PropertyMap::Pair*>(keyList->GetItemData(index));
			wxASSERT(p != NULL);

			DKObject<DKData> data = NULL;
			if (exportXML)
			{
				DKObject<DKXMLElement> e = p->value->Value().ExportXML();
				if (e)
				{
					data = DKXMLDocument(e).Export(DKStringEncoding::UTF8);
				}
			}
			else
			{
				DKBufferStream stream;
				if (p->value->Value().ExportStream(&stream))
				{
					data = stream.DataSource();
				}
			}

			if (data && data->Length() > 0)
			{
				wxFileDialog file(this, "Select File To Save", wxEmptyString, wxEmptyString, fileDialogFilterVar, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
				if (file.ShowModal() == wxID_OK)
				{
					wxString path = file.GetPath();
					if (data->WriteToFile((const wchar_t*)path, true))
					{
						DKLog("Export variant to file:%ls succeed.\n", (const wchar_t*)path);
					}
					else
					{
						DKLog("[%s] Error: Cannot export variant. (WriteToFile failed)\n", DKLIB_FUNCTION_NAME);
						wxMessageBox("Cannot export variant.", "Error");
					}
				}
			}
			else
			{
				DKLog("[%s] Error: Cannot export variant.\n", DKLIB_FUNCTION_NAME);
				wxMessageBox("Cannot export variant.", "Error");
			}
		}

	}
}

void MainWindow::OnFileClose(wxCommandEvent& e)
{
	Close(true);
}

void MainWindow::OnFileQuit(wxCommandEvent& e)
{
	e.Skip();
	Close(true);
}

void MainWindow::OnEditUndo(wxCommandEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
	if (CanUndo())
		UndoRevision();
	else
		DKLog("[%s] Cannot undo.\n", DKLIB_FUNCTION_NAME);
}

void MainWindow::OnEditRedo(wxCommandEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
	if (CanRedo())
		RedoRevision();
	else
		DKLog("[%s] Cannot redo.\n", DKLIB_FUNCTION_NAME);
}

void MainWindow::OnEditNew(wxCommandEvent& e)
{
	if (keyList->HasFocus())
	{
		ProcessCommand(UICommandKeyListNew);
	}
	else		// 열려있는 페이지에서 새 키 추가함
	{
	}
}

void MainWindow::OnEditCut(wxCommandEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);	
}

void MainWindow::OnEditCopy(wxCommandEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
}

void MainWindow::OnEditPaste(wxCommandEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
}

void MainWindow::OnEditDuplicate(wxCommandEvent& e)
{
	if (keyList->HasFocus())
	{
		ProcessCommand(UICommandKeyListDuplicate);
	}
	else
	{
	}
}

void MainWindow::OnEditDelete(wxCommandEvent& e)
{
	if (keyList->HasFocus())
	{
		ProcessCommand(UICommandKeyListDelete);
	}
	else
	{
	}
}

void MainWindow::OnEditSelectAll(wxCommandEvent& e)
{
	if (keyList->HasFocus())
	{
		ProcessCommand(UICommandKeyListSelectAll);
	}
	else
	{
	}	
}

void MainWindow::OnEditFormatXML(wxCommandEvent& e)
{
	exportXML = editMenu->IsChecked(UICommandEditFormatXML);
	DKLog("export XML: %d\n", (int)exportXML);
}

void MainWindow::OnWindowClose(wxCommandEvent& e)
{
	wxWindow* current = notebook->GetCurrentPage();
	if (current)
	{
		notebook->DeletePage(notebook->GetPageIndex(current));
	}
}

void MainWindow::OnWindowCloseOthers(wxCommandEvent& e)
{
	wxWindow* current = notebook->GetCurrentPage();
	DKArray<wxWindow*> pages;	
	pages.Reserve(notebook->GetPageCount());
	for (size_t i = 0; i < notebook->GetPageCount(); i++)
	{
		wxWindow* p = notebook->GetPage(i);
		if (p != current)
			pages.Add(p);
	}
	for (size_t i = 0; i < pages.Count(); ++i)
	{
		notebook->DeletePage( notebook->GetPageIndex(pages.Value(i)));
	}
}

void MainWindow::OnWindowCloseAll(wxCommandEvent& e)
{
	notebook->DeleteAllPages();
}

void MainWindow::OnHelpAbout(wxCommandEvent& e)
{
	wxMessageBox(wxString::Format(
		"DK PropertyEditor\n\n"
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
	if (IsModified())
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

	DKVariant::VArray columnWidth;
	for (int i = 0; i < keyList->GetColumnCount(); i++)
	{
		columnWidth.Add(DKVariant::VInteger(keyList->GetColumnWidth(i)));
	}
	DKPropertySet::DefaultSet().SetValue(L"ExportXML", DKVariant::VInteger(exportXML));
	DKPropertySet::DefaultSet().SetValue(L"KeyListColumnWidth", columnWidth);
	DKPropertySet::DefaultSet().SetValue(L"FrameRect", DKVariant::VVector4(framePos.x, framePos.y, frameSize.x, frameSize.y));
	DKPropertySet::DefaultSet().SetValue(L"AUILayout", (const wchar_t*)auiManager.SavePerspective());
	DKPropertySet::DefaultSet().SetValue(L"MaxUndoLevel", DKVariant::VInteger(maxUndoLevel));
}

void MainWindow::OnVariantModified(VariantEditorEvent& e)
{
	VariantEditor* editor = e.GetEditor();
	wxASSERT_MSG(editor, "editor cannot be null");

	VariantEditorData* data = static_cast<VariantEditorData*>(editor->GetClientObject());

	DKString key = (const wchar_t*)data->key;
	DKLog("[%s] (key:%ls)\n", DKLIB_FUNCTION_NAME, (const wchar_t*)key);

	// 현재 버전을 저장하고 새 리비전을 만들어서 넣음.
	PropertyMap nextRev = CurrentWorkingSet();

	nextRev.Update(key, editor->GetVariant());
	PushRevisionWithModifiedKey(nextRev, key, key);

	ReloadData();
	UpdateTitle();
}

void MainWindow::UpdateTitle(void)
{
	DKLog("Setting title (path:%ls)\n", (const wchar_t*)filePath);

	SetTitle(wxString::Format("%ls%s - %ls",
		(filePath.Length() > 0 ? (const wchar_t*)filePath : L"Untitled"),
		(this->readonly ? " (read-only)" : (IsModified() ? "*" : "")),
		(const wchar_t*)wxTheApp->GetAppName()));
}

void MainWindow::ReloadData(void)
{
	keyList->Freeze();

	keyList->DeleteAllItems();
	
	DKLog("reload data - propertySet\n");

	// 수정된 키 값들 (LastSaved 에서 변경된 것들)
	DKSet<DKString> modifiedKeys = GetModifiedKeys();
	// 키 추출
	CurrentWorkingSet().EnumerateForward([this, &modifiedKeys](const PropertyMap::Pair& pair)
	{
		int count = this->keyList->GetItemCount();
		long index = this->keyList->InsertItem(count, (const wxString&)((const wchar_t*)pair.key), 0);
		if (index >= 0)
		{
			this->keyList->SetItem(index, 1, VariantEditor::VariantTypeToString(pair.value->Type()));
			this->keyList->SetItemPtrData(index, reinterpret_cast<wxUIntPtr>(&pair));

			if (modifiedKeys.Contains(pair.key))
			{
				wxListItem item;
				item.m_itemId = index;
				item.SetBackgroundColour(*wxLIGHT_GREY);
				this->keyList->SetItem(item);
			}
		}
	});

	struct KeySort
	{
		static int wxCALLBACK Compare(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
		{
			const PropertyMap::Pair* lhs = reinterpret_cast<const PropertyMap::Pair*>(item1);
			const PropertyMap::Pair* rhs = reinterpret_cast<const PropertyMap::Pair*>(item2);

			int ret = lhs->key.CompareNoCase(rhs->key);
			if (ret == 0)
				return lhs->key.Compare(rhs->key);
			return ret;
		}
	};
	// 키 정렬
	keyList->SortItems(&KeySort::Compare, 0);

	keyList->Thaw();

	//DKLog("TODO: Update Editor windows if necessary!\n");
}


void MainWindow::OnKeyListOpen(wxCommandEvent& e)
{
	if (keyList->GetSelectedItemCount() > 0)
	{
		long index = -1;
		
		while ( true )
		{
			index = keyList->GetNextItem(index, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if (index < 0)
				break;
			else
			{
				const PropertyMap::Pair* p = reinterpret_cast<const PropertyMap::Pair*>(keyList->GetItemData(index));
				wxASSERT(p != NULL);
				
				OpenEditor((const wchar_t*)p->key);
			}
		}
	}	
}

void MainWindow::OnKeyListNew(wxCommandEvent& e)
{
	PropertyMap nextRev = CurrentWorkingSet();

	DKObject<VariantObject> vo = DKObject<VariantObject>::New();

	DKString key = L"";

	for (unsigned int i = 1; true; i++)
	{
		key = DKString::Format("Key %d", i);
		if (nextRev.Insert(key, vo))
		{
			PushRevisionWithModifiedKey(nextRev, L"", key);
			break;
		}
	}
	ReloadData();
	UpdateTitle();	

	// key 찾아서 선택후, edit 상태로 변경함
	for (int i = 0; i < keyList->GetItemCount(); ++i)
	{
		const PropertyMap::Pair* p = reinterpret_cast<const PropertyMap::Pair*>(keyList->GetItemData(i));
		if (p->key == key)
		{
			keyList->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
			keyList->EditLabel(i);
			break;
		}
	}
	DKLog("New Item (%ls) added.\n", (const wchar_t*)key);
}

void MainWindow::OnKeyListCut(wxCommandEvent& e)
{
}

void MainWindow::OnKeyListCopy(wxCommandEvent& e)
{
}

void MainWindow::OnKeyListPaste(wxCommandEvent& e)
{
}

void MainWindow::OnKeyListDuplicate(wxCommandEvent& e)
{
	if (keyList->GetSelectedItemCount() > 0)
	{
		DKArray<ModifiedKey> keys;
		
		PropertyMap nextRev = CurrentWorkingSet();
		
		long index = -1;
		
		while (true)
		{
			index = keyList->GetNextItem(index, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if (index < 0)
				break;
			else
			{
				const PropertyMap::Pair* p = reinterpret_cast<const PropertyMap::Pair*>(keyList->GetItemData(index));
				wxASSERT(p != NULL);
				for (size_t i = 2; true; i++)
				{
					DKString newKey = DKString::Format("%ls %u", (const wchar_t*)p->key, i);
					if (nextRev.Insert(newKey, p->value))
					{
						keys.Add(ModifiedKey(L"", newKey));
						break;
					}
				}
			}
		}
		
		if (keys.Count() > 0)
		{
			PushRevisionWithModifiedKeys(nextRev, keys, keys.Count());
			
			ReloadData();
			UpdateTitle();
		}
	}	
}

void MainWindow::OnKeyListDelete(wxCommandEvent& e)
{
	if (keyList->GetSelectedItemCount() > 0)
	{
		DKArray<ModifiedKey> keys;
		
		long index = -1;
		
		while (true)
		{
			index = keyList->GetNextItem(index, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if (index < 0)
				break;
			else
			{
				const PropertyMap::Pair* p = reinterpret_cast<const PropertyMap::Pair*>(keyList->GetItemData(index));
				wxASSERT(p != NULL);
				keys.Add(ModifiedKey(p->key, L""));
			}
		}
		
		if (keys.Count() > 0)
		{
			// 열려있는 에디터 닫음
			for (size_t i = 0; i < keys.Count(); ++i)
				CloseEditor((const wchar_t*)keys.Value(i).oldKey);

			PropertyMap nextRev = CurrentWorkingSet();
			
			for (size_t i = 0; i < keys.Count(); ++i)
				nextRev.Remove(keys.Value(i).oldKey);
			
			PushRevisionWithModifiedKeys(nextRev, keys, keys.Count());
			
			ReloadData();
			UpdateTitle();
		}
	}	
}

void MainWindow::OnKeyListSelectAll(wxCommandEvent& e)
{
	for (int i = 0; i < keyList->GetItemCount(); ++i)
	{
		keyList->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}	
}

void MainWindow::OnEndEditKey(wxListEvent& e)
{
	DKLog("[%s] \n", DKLIB_FUNCTION_NAME);
	if (!e.IsEditCancelled())
	{
		long itemIndex = e.GetIndex();
		const PropertyMap::Pair* p = reinterpret_cast<const PropertyMap::Pair*>(keyList->GetItemData(itemIndex));
		DKString newKey = (const wchar_t*)e.GetLabel();
		DKString oldKey = p->key;
		e.Veto();

		wxASSERT(p != NULL);

		DKLog("before:%ls, after:%ls\n", (const wchar_t*)p->key, (const wchar_t*)newKey);
		
		if (oldKey != newKey)
		{
			if (newKey.Length() > 0)
			{
				if (CurrentWorkingSet().Find(newKey) == NULL)
				{
					PropertyMap nextRev = CurrentWorkingSet();

					nextRev.Update(newKey, p->value);
					nextRev.Remove(oldKey);

					PushRevisionWithModifiedKey(nextRev, oldKey, newKey);

					VariantEditor* editor = FindEditor((const wchar_t*)oldKey);
					if (editor)
					{
						VariantEditorData* data = static_cast<VariantEditorData*>(editor->GetClientObject());
						data->key = (const wchar_t*)newKey;
						
						int pageIndex = notebook->GetPageIndex(editor);
						if (pageIndex != wxNOT_FOUND)
							notebook->SetPageText(pageIndex, data->key);
					}
					
					ReloadData();
					UpdateTitle();
				}
				else
				{
					DKLog("[%s] Error: Key(%ls) is already exist.\n", DKLIB_FUNCTION_NAME, (const wchar_t*)newKey);
					wxMessageBox(wxString::Format("Key(%ls) is already exist.", (const wchar_t*)newKey), "Error");
				}				
			}
			else
			{
				DKLog("[%s] Error: Key Name is Empty.\n", DKLIB_FUNCTION_NAME);
				wxMessageBox("Invalid Key:(name cannot be empty.)", "Error");
			}
		}		
	}
}

void MainWindow::OnActivateKey(wxListEvent& e)
{
	DKLog("[%s] \n", DKLIB_FUNCTION_NAME);
	const wxListItem& item = e.GetItem();
	const PropertyMap::Pair* p = reinterpret_cast<const PropertyMap::Pair*>(item.GetData());
	if (p)
	{
		OpenEditor((const wchar_t*)p->key);
	}
}

void MainWindow::OnContextMenu(wxContextMenuEvent& e)
{
	DKLog("%s id:%d self:%d, list:%d (obj:%x, type:%d)\n",
		DKLIB_FUNCTION_NAME, e.GetId(), this->GetId(), keyList->GetId(),
		e.GetEventObject(), e.GetEventType());

	if (e.GetId() == keyList->GetId())
	{
		keyList->SetFocus();

		wxMenu menu;
		menu.Append(UICommandKeyListOpen, "Open");
		menu.AppendSeparator();
		menu.Append(UICommandKeyListCut, "Cut\tCTRL-X");
		menu.Append(UICommandKeyListCopy, "Copy\tCTRL-C");
		menu.Append(UICommandKeyListPaste, "Paste\tCTRL-V");
		menu.Append(UICommandKeyListDuplicate, "Duplicate\tCTRL-D");
		menu.Append(UICommandKeyListDelete, "Delete\tCTRL-BACK");
		menu.AppendSeparator();
		menu.Append(UICommandKeyListNew, "New Key\tCTRL-N");
		menu.Append(UICommandFileImportKey, "Import Key...");
		menu.Append(UICommandFileExportKey, "Export Key...");
		menu.AppendSeparator();
		menu.Append(UICommandKeyListSelectAll, "Select All");

		if (keyList->GetItemCount() == 0)
		{
			menu.Enable(UICommandKeyListSelectAll, false);
		}

		if (keyList->GetSelectedItemCount() > 0)
		{
		}
		else
		{
			menu.Enable(UICommandKeyListOpen, false);
			menu.Enable(UICommandKeyListCut, false);
			menu.Enable(UICommandKeyListCopy, false);
			menu.Enable(UICommandKeyListDuplicate, false);
			menu.Enable(UICommandKeyListDelete, false);
			menu.Enable(UICommandFileExportKey, false);
		}
		if (wxTheClipboard->Open())
		{

			wxTheClipboard->Close();
		}
		else
		{
			menu.Enable(UICommandKeyListPaste, false);
		}

		PopupMenu(&menu, keyList->ScreenToClient(e.GetPosition()));	
	}
}

void MainWindow::OnMenuOpen(wxMenuEvent& e)
{
	wxMenu* menu = e.GetMenu();
	DKLog("[%s] menu:%p\n", DKLIB_FUNCTION_NAME, menu);
	if (menu == fileMenu)
	{
		if (keyList->HasFocus())
		{
			if (keyList->GetSelectedItemCount() == 1)
			{
				fileMenu->Enable(UICommandFileExportKey, true);
			}
			else
			{
				fileMenu->Enable(UICommandFileExportKey, false);
			}
		}
		DKLog("File menu open\n");
	}
	else if (menu == editMenu)
	{
		if (keyList->HasFocus())
		{
			if (keyList->GetSelectedItemCount() > 0)
			{
				editMenu->Enable(wxID_CUT, true);
				editMenu->Enable(wxID_COPY, true);
				editMenu->Enable(wxID_DUPLICATE, true);
				editMenu->Enable(wxID_DELETE, true);
			}
			else
			{
				editMenu->Enable(wxID_CUT, false);
				editMenu->Enable(wxID_COPY, false);
				editMenu->Enable(wxID_DUPLICATE, false);
				editMenu->Enable(wxID_DELETE, false);
			}
		}

		editMenu->Enable(wxID_UNDO, CanUndo());
		editMenu->Enable(wxID_REDO, CanRedo());

		DKLog("Edit menu open\n");
	}
	else if (menu == helpMenu)
	{
		DKLog("Help menu open\n");
	}
}

void MainWindow::OpenEditor(const wxString& key)
{
	DKLog("Open Key: %ls\n", (const wchar_t*)key);
	size_t c = notebook->GetPageCount();
	for (size_t i = 0; i < c; i++)
	{
		VariantEditorData* data = static_cast<VariantEditorData*>(notebook->GetPage(i)->GetClientObject());
		if (data->key == key)
		{
			notebook->SetSelection(i);
			return;
		}
	}
	
	VariantEditorData* data = new VariantEditorData(key);
	VariantEditor* editor = new VariantEditor(this, wxID_ANY);
	editor->SetClientObject(data);
	editor->SetVariant(CurrentWorkingSet().Find((const wchar_t*)key)->value, this->readonly);
	notebook->InsertPage(0, editor, key, true, wxNullBitmap);
}

VariantEditor* MainWindow::FindEditor(const wxString& key)
{
	size_t c = notebook->GetPageCount();
	for (size_t i = 0; i < c; i++)
	{
		VariantEditor* editor = static_cast<VariantEditor*>(notebook->GetPage(i));
		wxASSERT(editor);
		VariantEditorData* data = static_cast<VariantEditorData*>(editor->GetClientObject());
		wxASSERT(data);
		if (data->key == key)
			return editor;
	}
	return NULL;
}

void MainWindow::CloseEditor(const wxString& key)
{
	size_t c = notebook->GetPageCount();
	for (size_t i = 0; i < c; i++)
	{
		VariantEditorData* data = static_cast<VariantEditorData*>(notebook->GetPage(i)->GetClientObject());
		if (data->key == key)
		{
			notebook->DeletePage(i);
			return;
		}
	}
}

void MainWindow::CloseAllEditors(void)
{
	notebook->DeleteAllPages();
}

bool MainWindow::IsModified(void) const
{
	return IsPropertyMapEqual(CurrentWorkingSet(), lastSavedPropertyMap) == false;
}

DKSet<DKString> MainWindow::GetModifiedKeys(void) const
{
	DKSet<DKString> diff;
	
	CurrentWorkingSet().EnumerateForward([this, &diff](const PropertyMap::Pair& pair)
	{
		auto p = this->lastSavedPropertyMap.Find(pair.key);
		if (p == NULL || p->value->IsEqual(pair.value) == false)
		{
			diff.Insert(pair.key);
		}
	});

	return diff;
}

MainWindow::PropertyMap& MainWindow::CurrentWorkingSet(void)
{
	wxASSERT(propertyMapRevisions.Count() > 0);
	return propertyMapRevisions.Value(workingSetIndex).propertyMap;
}

const MainWindow::PropertyMap& MainWindow::CurrentWorkingSet(void) const
{
	wxASSERT(propertyMapRevisions.Count() > 0);
	return propertyMapRevisions.Value(workingSetIndex).propertyMap;
}

bool MainWindow::IsPropertyMapEqual(const PropertyMap& lhs, const PropertyMap& rhs)
{
	DKArray<const PropertyMap::Pair*> pairs1;
	DKArray<const PropertyMap::Pair*> pairs2;

	lhs.EnumerateForward([&pairs1](const PropertyMap::Pair& pair)
						 {
							 pairs1.Add(&pair);
						 });
	rhs.EnumerateForward([&pairs2](const PropertyMap::Pair& pair)
						 {
							 pairs2.Add(&pair);
						 });

	if (pairs1.Count() == pairs2.Count())
	{
		size_t count = pairs1.Count();
		for (size_t i = 0; i < count; ++i)
		{
			const PropertyMap::Pair* p1 = pairs1.Value(i);
			const PropertyMap::Pair* p2 = pairs2.Value(i);
			
			if (p1->key.Compare(p2->key) != 0 || p1->value->IsEqual(p2->value) == false)
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

void MainWindow::PushRevisionWithModifiedKeys(const PropertyMap& ws, const ModifiedKey* keys, size_t numKeys)
{
	wxASSERT(propertyMapRevisions.Count() > 0);

	RevisionData rev = {ws, DKArray<ModifiedKey>(keys, numKeys)};
	PushRevision(rev);
}

void MainWindow::PushRevisionWithModifiedKey(const PropertyMap& ws, const DKString& oldKey, const DKString& newKey)
{
	wxASSERT(propertyMapRevisions.Count() > 0);

	RevisionData rev = {ws, DKArray<ModifiedKey>(ModifiedKey(oldKey, newKey), 1)};
	PushRevision(rev);
}

void MainWindow::PushRevision(const RevisionData& rev)
{
	wxASSERT(propertyMapRevisions.Count() > 0);

	size_t count = propertyMapRevisions.Count();

	// 현재꺼 뒤에 있는 Redo 목록 지움
	if (workingSetIndex + 1 < count)
	{
		size_t index = workingSetIndex + 1;
		size_t numRemove = count - workingSetIndex - 1;
		propertyMapRevisions.Remove(index, numRemove);
		DKLog("Revision %u to %u removed. (%u revisions)\n", index, index + numRemove, numRemove);
	}

	propertyMapRevisions.Add(rev);
	if (this->maxUndoLevel > 0 && propertyMapRevisions.Count() > this->maxUndoLevel)
	{
		size_t c = propertyMapRevisions.Count() - this->maxUndoLevel;
		propertyMapRevisions.Remove(0, c);
		DKLog("%d Revisions truncated.\n", c);
	}

	wxASSERT(propertyMapRevisions.Count() > 0);
	workingSetIndex = propertyMapRevisions.Count() - 1;

	DKLog("Current Revision Index: %u. (total:%u)\n", workingSetIndex, propertyMapRevisions.Count());
}

void MainWindow::ResetRevisionHistory(const PropertyMap& map)
{
	propertyMapRevisions.Clear();
	RevisionData rev = {map, DKArray<ModifiedKey>()};
	propertyMapRevisions.Add(rev);
	workingSetIndex = 0;

	DKLog("Reset Revision to 0\n");
}

bool MainWindow::CanUndo(void) const
{
	return workingSetIndex > 0;
}

bool MainWindow::CanRedo(void) const
{
	return workingSetIndex + 1 < propertyMapRevisions.Count();
}

void MainWindow::UndoRevision(void)
{
	wxASSERT(workingSetIndex > 0);

	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);

	RevisionData& rdata = propertyMapRevisions.Value(workingSetIndex);
	for (size_t i = 0; i < rdata.keys.Count(); ++i)
	{
		const ModifiedKey& mkey = rdata.keys.Value(i);

		if (mkey.newKey.Length() > 0)
		{
			if (mkey.newKey == mkey.oldKey)		// 키 내용이 바뀌었음
			{
				VariantEditor* editor = FindEditor((const wchar_t*)mkey.oldKey);
				if (editor)		// 내용 갱신함
				{
					VariantObject* vobj = propertyMapRevisions.Value(workingSetIndex -1).propertyMap.Find(mkey.oldKey)->value;
					wxASSERT(vobj != NULL);
					editor->SetVariant(vobj, this->readonly);		// 이전버전으로 복구함
				}
			}
			else		// 키 이름 자체가 변경되었음
			{
				if (mkey.oldKey.Length() > 0)	// 키가 변경되었음 (oldKey -> newKey, 에디터 찾아서 변경함)
				{
					DKLog("Revert key %ls (revert to %ls)\n", (const wchar_t*)mkey.newKey, (const wchar_t*)mkey.oldKey);
					VariantEditor* editor = FindEditor((const wchar_t*)mkey.newKey);
					if (editor)
					{
						VariantEditorData* data = static_cast<VariantEditorData*>(editor->GetClientObject());
						wxASSERT_MSG(data->key == (const wchar_t*)mkey.newKey, "editor-key must be equal!");
						data->key = (const wchar_t*)mkey.oldKey;

						int pageIndex = notebook->GetPageIndex(editor);
						if (pageIndex != wxNOT_FOUND)
							notebook->SetPageText(pageIndex, data->key);
					}
				}
				else		// 이전 버전에 없던 키임
				{
					// 열려있는 에디터 찾아서 닫음
					DKLog("Revert key %ls (remove)\n", (const wchar_t*)mkey.newKey);
					CloseEditor((const wchar_t*)mkey.newKey);
				}
			}
		}
	}
	workingSetIndex -= 1;
	DKLog("[%s] CurrentRevisionIndex:%u (total:%u)\n", DKLIB_FUNCTION_NAME, workingSetIndex, propertyMapRevisions.Count());
	ReloadData();
	UpdateTitle();
}

void MainWindow::RedoRevision(void)
{
	wxASSERT(workingSetIndex + 1 < propertyMapRevisions.Count());

	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);

	RevisionData& rdata = propertyMapRevisions.Value(workingSetIndex + 1);
	for (size_t i = 0; i < rdata.keys.Count(); ++i)
	{
		const ModifiedKey& mkey = rdata.keys.Value(i);

		if (mkey.oldKey.Length() > 0)
		{
			if (mkey.newKey == mkey.oldKey)		// 키 내용이 바뀌었음
			{
				VariantEditor* editor = FindEditor((const wchar_t*)mkey.newKey);
				if (editor)		// 내용 갱신함
				{
					VariantObject* vobj = rdata.propertyMap.Find(mkey.newKey)->value;
					wxASSERT(vobj != NULL);
					editor->SetVariant(vobj, this->readonly);		// 이전버전으로 복구함
				}
			}
			else	// 키 이름 자체가 변경되었음
			{
				if (mkey.newKey.Length() > 0)	// 키가 변경되었음 (oldKey -> newKey, 에디터 찾아서 변경함)
				{
					DKLog("Revert key %ls (revert to %ls)\n", (const wchar_t*)mkey.oldKey, (const wchar_t*)mkey.newKey);
					VariantEditor* editor = FindEditor((const wchar_t*)mkey.oldKey);
					if (editor)
					{
						VariantEditorData* data = static_cast<VariantEditorData*>(editor->GetClientObject());
						wxASSERT_MSG(data->key == (const wchar_t*)mkey.oldKey, "editor-key must be equal!");
						data->key = (const wchar_t*)mkey.newKey;

						int pageIndex = notebook->GetPageIndex(editor);
						if (pageIndex != wxNOT_FOUND)
							notebook->SetPageText(pageIndex, data->key);
					}
				}
				else		// 다음 버전에서 삭제된 키임
				{
					// 열려있는 에디터 찾아서 닫음
					DKLog("Revert key %ls (remove)\n", (const wchar_t*)mkey.oldKey);
					CloseEditor((const wchar_t*)mkey.oldKey);
				}
			}
		}
	}

	workingSetIndex += 1;
	DKLog("[%s] CurrentRevisionIndex:%u (total:%u)\n", DKLIB_FUNCTION_NAME, workingSetIndex, propertyMapRevisions.Count());

	ReloadData();
	UpdateTitle();
}

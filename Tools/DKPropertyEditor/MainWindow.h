#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/listctrl.h>
#include <wx/clipbrd.h>
#include "VariantEditor.h"

class MainWindow : public wxFrame
{
	enum UICommand
	{
		UICommandBegin = wxID_HIGHEST,
		UICommandFileImportKey,			// import variant file as new key
		UICommandFileExportKey,			// export one key as single variant file
		UICommandFileClose,
		
		UICommandEditNew,				// new key or new value
		UICommandEditFormatXML,			// xml 포맷으로 저장할지 여부

		UICommandKeyListOpen,
		UICommandKeyListNew,
		UICommandKeyListCut,
		UICommandKeyListCopy,
		UICommandKeyListPaste,
		UICommandKeyListDuplicate,
		UICommandKeyListDelete,
		UICommandKeyListSelectAll,

		UICommandWindowClose,
		UICommandWindowCloseOthers,
		UICommandWindowCloseAll,
	};
	enum UIControl
	{
		UIControlBegin = wxID_HIGHEST,
		UIControlKeyList,
		UIControlEditorNotebook,
	};
public:
	MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size, long style = wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	~MainWindow(void);
	
private:
	void OnFileNew(wxCommandEvent& e);
	void OnFileOpen(wxCommandEvent& e);
	void OnFileSave(wxCommandEvent& e);
	void OnFileSaveAs(wxCommandEvent& e);
	void OnFileImportKey(wxCommandEvent& e);
	void OnFileExportKey(wxCommandEvent& e);
	void OnFileClose(wxCommandEvent& e);
	void OnFileQuit(wxCommandEvent& e);
	
	void OnEditUndo(wxCommandEvent& e);
	void OnEditRedo(wxCommandEvent& e);
	void OnEditNew(wxCommandEvent& e);
	void OnEditCut(wxCommandEvent& e);
	void OnEditCopy(wxCommandEvent& e);
	void OnEditPaste(wxCommandEvent& e);
	void OnEditDuplicate(wxCommandEvent& e);
	void OnEditDelete(wxCommandEvent& e);
	void OnEditSelectAll(wxCommandEvent& e);
	void OnEditFormatXML(wxCommandEvent& e);

	void OnKeyListOpen(wxCommandEvent& e);
	void OnKeyListNew(wxCommandEvent& e);
	void OnKeyListCut(wxCommandEvent& e);
	void OnKeyListCopy(wxCommandEvent& e);
	void OnKeyListPaste(wxCommandEvent& e);
	void OnKeyListDuplicate(wxCommandEvent& e);
	void OnKeyListDelete(wxCommandEvent& e);
	void OnKeyListSelectAll(wxCommandEvent& e);

	void OnWindowClose(wxCommandEvent& e);
	void OnWindowCloseOthers(wxCommandEvent& e);
	void OnWindowCloseAll(wxCommandEvent& e);

	void OnHelpAbout(wxCommandEvent& e);
	
	void OnVariantModified(VariantEditorEvent& e);

	void OnSize(wxSizeEvent& e);
	void OnClose(wxCloseEvent& e);
	
	void OnActivateKey(wxListEvent& e);
	void OnEndEditKey(wxListEvent& e);
	void OnContextMenu(wxContextMenuEvent& e);
	
	void OpenEditor(const wxString& key);
	VariantEditor* FindEditor(const wxString& key);
	void CloseEditor(const wxString& key);
	void CloseAllEditors(void);
	
	void OnMenuOpen(wxMenuEvent& e);
	
	void UpdateTitle(void);
	void ReloadData(void);
	bool IsModified(void) const;
	
	DKSet<DKString> GetModifiedKeys(void) const;	
	
	typedef DKMap<DKString, DKObject<VariantObject>> PropertyMap;
	PropertyMap lastSavedPropertyMap;	// 마지막 저장된 set

	// ModifiedKey: 이전 버전에서 변경된 키들을 저장함
	// PropertyMapRevision: Undo/Redo 를 위한 변화된 키를 저장함 (이전 리비전에서 변경된것 저장)
	struct ModifiedKey
	{
		ModifiedKey(void) : oldKey(L""), newKey(L"") {}
		ModifiedKey(const DKString& k1, const DKString& k2) : oldKey(k1), newKey(k2) {}
		DKString oldKey;	// 이전 키 (비었으면 새로 추가된 키)
		DKString newKey;	// 변경된 키 (이전 키와 같으면 내용이 바뀐거, 비었으면 지워진거)
	};
	struct RevisionData
	{
		PropertyMap propertyMap;		// 해당 버전의 맵
		DKArray<ModifiedKey> keys;		// 이전 버전에서 변경된 키들
	};
	// Undo/Redo 버전들
	DKArray<RevisionData> propertyMapRevisions;
	size_t workingSetIndex;			// 현재 작업중인 set 의 인덱스

	PropertyMap& CurrentWorkingSet(void);
	const PropertyMap& CurrentWorkingSet(void) const;
	
	void PushRevisionWithModifiedKeys(const PropertyMap& ws, const ModifiedKey* keys, size_t numKeys);
	void PushRevisionWithModifiedKey(const PropertyMap& ws, const DKString& oldKey, const DKString& newKey);
	void PushRevision(const RevisionData& rev);
	void ResetRevisionHistory(const PropertyMap& map);
	static bool IsPropertyMapEqual(const PropertyMap& lhs, const PropertyMap& rhs);	
	
	bool CanUndo(void) const;
	bool CanRedo(void) const;
	void UndoRevision(void);
	void RedoRevision(void);

	// 에디터 데이터
	class VariantEditorData : public wxClientData
	{
	public:
		VariantEditorData(const wxString& k) : key(k) {}
		wxString key;
	};
	
	wxAuiManager	auiManager;		// Advanced UI Manager
	wxAuiNotebook*	notebook;		// editor notebook
	wxListCtrl*		keyList;	
	
	wxString filePath;
	bool exportXML;
	bool readonly;
	size_t maxUndoLevel;
	
	wxMenu* fileMenu;
	wxMenu* editMenu;
	wxMenu* windowMenu;
	wxMenu* helpMenu;
	
	wxDECLARE_EVENT_TABLE();
};


#pragma once

#include <wx/wx.h>
#include <wx/dataview.h>

#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class VariantNode;
class VariantDataModel;
class VariantObject;
class VariantEditor;

////////////////////////////////////////////////////////////////////////////////
// VariantObject : Variant 정보를 담은 객체
// 중간 snapshot 으로 사용할 수 있으며, 내부적으로 이 형식의 데이터를 사용하게 된다.
////////////////////////////////////////////////////////////////////////////////
class VariantObject
{
public:
	VariantObject(void);
	VariantObject(const VariantNode* node);
	VariantObject(const DKVariant& v);
	~VariantObject(void);
	
	DKVariant Value(void) const;
	DKVariant::Type Type(void) const;
	bool IsEqual(const VariantObject* vo) const;
private:
	friend class VariantDataModel;
	VariantObject(const VariantObject&);
	VariantObject& operator = (const VariantObject&);
	VariantNode* node;
};

////////////////////////////////////////////////////////////////////////////////
// VariantEditorEvent : 이벤트
//
// VARIANT_EDITOR_MODIFIED : 값이 수정되었을때
////////////////////////////////////////////////////////////////////////////////
class VariantEditorEvent : public wxNotifyEvent
{
public:
	VariantEditorEvent(wxEventType eventType, int wid, VariantEditor* e)
		: wxNotifyEvent(eventType, wid)
		, editor(e)
	{
	}
	VariantEditorEvent(const VariantEditorEvent& e)
		: wxNotifyEvent(e), editor(e.editor)
	{
	}
	VariantEditor* GetEditor(void)
	{
		return editor;
	}
protected:
	wxEvent* Clone(void) const
	{
		return new VariantEditorEvent(*this);
	}
	VariantEditor* editor;
};

//typedef void (wxEvtHandler::*VariantEditorEventFunction)(VariantEditorEvent&);
//#define VariantEditorEventHandler(func)				wxEVENT_HANDLER_CAST(VariantEditorEventFunction, func)
//#define VariantEditorEventHandler(func)				(&func)
//#define EVT_VARIANT_EDITOR_MODIFIED(id, func)		wx__DECLARE_EVT1(VARIANT_EDITOR_MODIFIED, id, VariantEditorEventHandler(func))

wxDECLARE_EVENT(VARIANT_EDITOR_MODIFIED, VariantEditorEvent);
#define EVT_VARIANT_EDITOR_MODIFIED(id, func)		wx__DECLARE_EVT1(VARIANT_EDITOR_MODIFIED, id, &func)

////////////////////////////////////////////////////////////////////////////////
// VariantEditor : DKVariant 편집 창
// DKVariant 를 VariantObject 로 변환해서 사용해야 한다.
////////////////////////////////////////////////////////////////////////////////
class VariantEditor : public wxWindow
{
public:
	VariantEditor(void);
	VariantEditor(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = wxPanelNameStr 
		);

	~VariantEditor(void);

	static wxString VariantTypeToString(DKVariant::Type t);
	static DKVariant::Type StringToVariantType(const wxString& s);

	void SetVariant(const VariantObject* vo, bool readonly);
	DKObject<VariantObject> GetVariant(void) const;

	bool IsReadOnly(void) const;
private:
	void OnSize(wxSizeEvent& e);
	void OnClose(wxCloseEvent& e);
	void OnContextMenu(wxContextMenuEvent& e);

	void InitDataView(void);

	void OnDataViewItemActivated(wxDataViewEvent& e);
	void OnDataViewItemContextMenu(wxDataViewEvent& e);
	void OnDataViewItemStartEditing(wxDataViewEvent& e);
	void OnDataViewItemEditingDone(wxDataViewEvent& e);
	void OnDataViewItemValueChanged(wxDataViewEvent& e);
	void OnDataViewItemBeginDrag(wxDataViewEvent& e);
	void OnDataViewItemDropPossible(wxDataViewEvent& e);
	void OnDataViewItemDrop(wxDataViewEvent& e);

	void OnItemNew(wxCommandEvent& e);
	void OnItemCut(wxCommandEvent& e);
	void OnItemCopy(wxCommandEvent& e);
	void OnItemPaste(wxCommandEvent& e);
	void OnItemDuplicate(wxCommandEvent& e);
	void OnItemDelete(wxCommandEvent& e);

	void OnItemFileImport(wxCommandEvent& e);
	void OnItemFileExport(wxCommandEvent& e);
	void OnDataItemImport(wxCommandEvent& e);
	void OnDataItemExport(wxCommandEvent& e);
	void OnArrayItemReorder(wxCommandEvent& e);
	void OnDateTimeItemSetValue(wxCommandEvent& e);

	static size_t GetClipboardData(DKVariant& vout);
	static size_t SetClipboardData(DKVariant& vin);

	wxObjectDataPtr<VariantDataModel> dataModel;
	wxDataViewCtrl* dataView;
	bool enableDragSource;
	bool enableDropTarget;

	enum UICommand
	{
		UICommandBegin = wxID_HIGHEST,
		UICommandFileImport,			// 파일을 읽어와 현재 노드에 붙임
		UICommandFileExport,			// 현재 노드를 파일로 저장함
		UICommandDataImport,			// DKVariant::TypeData 형식에 파일을 넣음
		UICommandDataExport,			// DKVariant::TypeData 형식에서 파일을 꺼냄
		UICommandArrayReorder,			// array 안에서 순서 변경함
		UICommandDateTimeSetValue,		// 날짜, 시간 선택함
	};
	wxDECLARE_EVENT_TABLE();
};

#pragma once
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/propgrid/propgrid.h>
#include <wx/dataview.h>
#include "ModelNode.h"

class ModelProperties : public wxWindow
{
public:
	enum UIControl
	{
		UIControlPropertyGrid,
	};
	enum UICommand
	{
		UICommandBegin = wxID_HIGHEST,
		UICommandTest,
	};

	ModelProperties(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);
	~ModelProperties(void);

	void OnTest(wxCommandEvent& e);		// 테스트용

	void OnContextMenu(wxContextMenuEvent& e);
	void OnSize(wxSizeEvent& e);
	void OnClose(wxCloseEvent& e);

	void SetActiveNode(ModelNodeItemData* item);

	void OnPropertyChanging(wxPropertyGridEvent& e);
	void OnPropertyChanged(wxPropertyGridEvent& e);
	void OnPropertyLabelEditBegin(wxPropertyGridEvent& e);
	void OnPropertyLabelEditEnding(wxPropertyGridEvent& e);

	void OnDataViewItemStartEditing(wxDataViewEvent& e);
	void OnDataViewItemEditingDone(wxDataViewEvent& e);
	void OnDataViewItemValueChanged(wxDataViewEvent& e);
private:
	ModelNodeItemData*	modelNode;

	struct InfoPage
	{
		virtual ~InfoPage(void) {}
		wxAuiNotebook*		notebook;
		wxPropertyGrid*		infoView;

		// DKNSTransform
		wxFloatProperty*	rotationX;
		wxFloatProperty*	rotationY;
		wxFloatProperty*	rotationZ;
		wxFloatProperty*	positionX;
		wxFloatProperty*	positionY;
		wxFloatProperty*	positionZ;
	};
	struct ModelInfo : public InfoPage
	{
		wxEnumProperty*		type;
		wxStringProperty*	name;
	} modelInfo;
	struct RigidBodyInfo : public ModelInfo
	{
		wxWindow*			collisionView;
	} rigidBodyInfo;
	struct SoftBodyInfo : public ModelInfo
	{
		wxWindow*			collisionView;
	} softBodyInfo;
	struct MeshInfo : public ModelInfo
	{
		wxPropertyGrid*		meshView;
		wxDataViewCtrl*		streamView;		// 버텍스 스트림
		wxDataViewCtrl*		samplerView;	// 텍스쳐, 샘플러
		wxDataViewCtrl*		materialView;	// 머티리얼 프로퍼티

		wxBoolProperty*		hidden;
		wxStringProperty*	primitive;
		wxStringProperty*	cullMode;
		wxStringProperty*	material;
	} meshInfo;

	struct ShapeInfo : public InfoPage
	{
		wxEnumProperty*		type;
	} shapeInfo;

	struct ConstraintInfo : public InfoPage
	{
		wxEnumProperty*		type;
	} constraintInfo;

	wxDECLARE_EVENT_TABLE();
};


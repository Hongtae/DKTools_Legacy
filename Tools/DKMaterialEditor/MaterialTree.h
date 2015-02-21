#pragma once
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/treectrl.h>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>

#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class MaterialTree : public wxWindow
{
public:
	enum UIControl
	{
		UIControlNodeTree = 10,
	};
	enum UICommand
	{
		UICommandBegin = wxID_HIGHEST,
		UICommandAddProgram,
		UICommandAddUniform,
		UICommandAddSampler,
		UICommandAddStream,
		UICommandAddVertexShader,
		UICommandAddFragmentShader,
		UICommandAddGeometryShader,

		UICommandRemove,
		UICommandRename,
		UICommandMoveUp,
		UICommandMoveDown,

		UICommandEdit,
		UICommandBuild,
		UICommandCompile,

		UICommandTest,
	};

	MaterialTree(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);
	~MaterialTree(void);

	void OnTest(wxCommandEvent& e);		// 테스트용

	void OnContextMenu(wxContextMenuEvent& e);
	void OnSize(wxSizeEvent& e);
	void OnClose(wxCloseEvent& e);

	void OnAddProgram(wxCommandEvent& e);
	void OnAddSampler(wxCommandEvent& e);
	void OnAddUniform(wxCommandEvent& e);
	void OnAddStream(wxCommandEvent& e);
	void OnAddVertexShader(wxCommandEvent& e);
	void OnAddGeometryShader(wxCommandEvent& e);
	void OnAddFragmentShader(wxCommandEvent& e);

	void OnItemMenu(wxTreeEvent& e);
	void OnBeginLabelEdit(wxTreeEvent& e);
	void OnEndLabelEdit(wxTreeEvent& e);
	void OnItemActivated(wxTreeEvent& e);
	void OnItemSelected(wxTreeEvent& e);

	void OnRemove(wxCommandEvent& e);
	void OnRename(wxCommandEvent& e);
	void OnMoveUp(wxCommandEvent& e);
	void OnMoveDown(wxCommandEvent& e);

	void OnEdit(wxCommandEvent& e);
	void OnBuild(wxCommandEvent& e);
	void OnCompile(wxCommandEvent& e);

	void RebuildTree(void);
	void RebuildTree(wxTreeItemData* p);
	void UpdateTreeItem(wxTreeItemId item, bool recursive);

	void ShowMenu(wxTreeItemId item, const wxPoint& pt);

private:
	wxTreeCtrl*	nodeTree;
	wxDECLARE_EVENT_TABLE();
};

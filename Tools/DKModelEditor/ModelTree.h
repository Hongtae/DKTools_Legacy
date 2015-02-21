#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/treectrl.h>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class ModelTree : public wxWindow
{
public:
	enum UIControl
	{
		UIControlNodeTree = 10,
	};
	enum UICommand
	{
		UICommandBegin = wxID_HIGHEST,
		UICommandRename,
		UICommandCreateCollision,
		UICommandCreateShapeHull,
		UICommandSetMaterial,
		UICommandSetDiffuseMap,
		UICommandUpdateBoundingInfo,

		UICommandTest,
	};

	ModelTree(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);
	~ModelTree(void);

	void OnTest(wxCommandEvent& e);		// 테스트용

	void OnContextMenu(wxContextMenuEvent& e);
	void OnSize(wxSizeEvent& e);
	void OnClose(wxCloseEvent& e);

	void OnItemMenu(wxTreeEvent& e);
	void OnBeginLabelEdit(wxTreeEvent& e);
	void OnEndLabelEdit(wxTreeEvent& e);
	void OnItemActivated(wxTreeEvent& e);
	void OnItemSelected(wxTreeEvent& e);

	void ShowMenu(wxTreeItemId item, const wxPoint& pt);

	void RebuildTree(void);
	wxTreeItemId RebuildTree(DKModel* model, wxTreeItemId parent);

	void UpdateTreeItem(wxTreeItemId item, bool recursive);
	void SortEnclosingItem(wxTreeItemId item);

	void OnRename(wxCommandEvent& e);
	void OnCreateCollision(wxCommandEvent& e);
	void OnCreateShapeHull(wxCommandEvent& e);
	void OnSetMaterial(wxCommandEvent& e);
	void OnSetDiffuseMap(wxCommandEvent& e);
	void OnUpdateBoundingInfo(wxCommandEvent& e);

private:
	wxTreeCtrl*	nodeTree;
	wxDECLARE_EVENT_TABLE();
};


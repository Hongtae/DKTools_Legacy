#pragma once
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/treectrl.h>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/propgrid/propgrid.h>
#include "MaterialNode.h"

class MaterialProperties : public wxWindow
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

	MaterialProperties(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);
	~MaterialProperties(void);

	void OnTest(wxCommandEvent& e);		// 테스트용

	void OnContextMenu(wxContextMenuEvent& e);
	void OnSize(wxSizeEvent& e);
	void OnClose(wxCloseEvent& e);

	void SetActiveNode(MaterialNodeItemData* item);

	void OnPropertyChanging(wxPropertyGridEvent& e);
	void OnPropertyChanged(wxPropertyGridEvent& e);

private:
	MaterialNodeItemData*	materialNode;
	wxPropertyGrid*			properties;

	wxDECLARE_EVENT_TABLE();
};

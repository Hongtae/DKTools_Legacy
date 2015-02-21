#pragma once
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/treectrl.h>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include <wx/clrpicker.h>
#include <wx/propgrid/propgrid.h>
#include "RenderWindow.h"

class MainWindow : public wxFrame
{
public:
	MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size, long style = wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	~MainWindow(void);

	void PrintLog(const wxString& msg, const wxColour& c = wxColour(0,0,0));

private:
	void OnFileNew(wxCommandEvent& e);
	void OnFileOpen(wxCommandEvent& e);
	void OnFileSave(wxCommandEvent& e);
	void OnFileSaveAs(wxCommandEvent& e);
	void OnFileQuit(wxCommandEvent& e);

	void OnEditCut(wxCommandEvent& e);
	void OnEditCopy(wxCommandEvent& e);
	void OnEditPaste(wxCommandEvent& e);
	void OnEditDelete(wxCommandEvent& e);

	void OnViewLog(wxCommandEvent& e);

	void OnToolsOptions(wxCommandEvent& e);
	void OnHelpAbout(wxCommandEvent& e);

	void OnSize(wxSizeEvent& e);
	void OnClose(wxCloseEvent& e);

	void OnSliderUpdated(wxCommandEvent& e);
	void OnCheckBoxClicked(wxCommandEvent& e);
	void OnColourPickerChanged(wxColourPickerEvent& e);
	void OnResizeVolumeClicked(wxCommandEvent& e);

	unsigned int volumeWidth;
	unsigned int volumeHeight;
	unsigned int volumeDepth;

	wxSlider* sliderX;
	wxSlider* sliderY;
	wxSlider* sliderZ;
	wxCheckBox* checkboxX;
	wxCheckBox* checkboxY;
	wxCheckBox* checkboxZ;

	wxColourPickerCtrl*	voxelColorPicker;
	wxColourPickerCtrl*	wireColorPicker;
	wxColourPickerCtrl*	bgColorPicker;

	wxAuiManager	auiManager;		// Advanced UI Manager
	RenderWindow*	renderWindow;	// 메인 뷰어
	wxWindow*		editorWindow;	// 편집 윈도우
	wxTextCtrl*		logWindow;		// 로그 윈도우

	wxDECLARE_EVENT_TABLE();
};


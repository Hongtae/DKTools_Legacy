#pragma once

#include <wx/wx.h>
#include "../Frame/MediaFrame.h"

class MainWindow : public wxFrame
{
	enum UICommand
	{
		UICommandBegin = wxID_HIGHEST,
		UICommandFileOpenLocal,
		UICommandFileOpenURL,
		UICommandFileExit,

		UICommandViewResize50,
		UICommandViewResize100,
		UICommandViewResize150,
		UICommandViewResize200,
		UICommandViewAutoResize,
		UICommandViewFullScreen,

		UICommandHelpAbout,
	};

public:
	MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size, long style = wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	~MainWindow(void);

	void OpenFiles(const wxArrayString& files);
private:
	void OnFileOpenLocal(wxCommandEvent& e);
	void OnFileOpenURL(wxCommandEvent& e);
	void OnFileExit(wxCommandEvent& e);

	void OnViewResize(wxCommandEvent& e);
	void OnViewRatio(wxCommandEvent& e);
	void OnViewAutoResize(wxCommandEvent& e);
	void OnViewFullScreen(wxCommandEvent& e);

	void OnHelpAbout(wxCommandEvent& e);

	void OnSize(wxSizeEvent& e);
	void OnClose(wxCloseEvent& e);
	void OnMouseEvent(wxMouseEvent& e);
	void OnDropFiles(wxDropFilesEvent& e);

	void OnDropFiles(const wxArrayString& files);
	bool OpenMediaURL(const wxString& url);

	DKObject<MediaFrame>	mediaFrame;
	DKObject<DKScreen>		screen;
	DKObject<DKWindow>		window;
	DKPoint					mousePosition;
	bool					autoResize;
	
	wxDECLARE_EVENT_TABLE();
};


#pragma once
#include <wx/wx.h>
#include "RenderFrame.h"

class RenderWindow : public wxWindow
{
public:
	RenderWindow(wxWindow *parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		const wxString& name = wxPanelNameStr);
	~RenderWindow(void);
	bool InitScreen(void);
	
	const DKScreen* GetScreen(void) const	{return screen;}
	const DKWindow*	GetWindow(void) const	{return window;}
	RenderFrame				frame;

private:
	//wxSize windowSize;
	void OnSize(wxSizeEvent& e);
	void OnClose(wxCloseEvent& e);
	void OnMouseEvent(wxMouseEvent& e);

	DKObject<DKScreen>		screen;
	DKObject<DKWindow>		window;
	DKPoint					mousePosition;

	wxDECLARE_EVENT_TABLE();
};

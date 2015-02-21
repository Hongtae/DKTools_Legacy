// DKVolumeEditor.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

#include "DKVolumeEditor.h"
#include "MainWindow.h"

class DKVolumeEditorApp : public wxApp
{
public:
	bool OnInit(void)
	{
		if (!wxApp::OnInit())
			return false;

		MainWindow* frame = new MainWindow(wxTheApp->GetAppName(),
			wxDefaultPosition,
			wxSize(1024, 768));
		frame->Show(true);

		SetTopWindow(frame);

		return true;
	}
	DKApplication app;
};

wxIMPLEMENT_APP(DKVolumeEditorApp);


void PrintLog(const wxString& msg, const wxColour& c)
{
	reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->PrintLog(msg, c);
}

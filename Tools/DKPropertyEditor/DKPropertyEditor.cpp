// DKVolumeEditor.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DKPropertyEditor.h"
#include "MainWindow.h"

class DKPropertyEditorApp : public wxApp
{
public:
	bool OnInit(void)
	{
		if (!wxApp::OnInit())
			return false;

		MainWindow* frame = new MainWindow(wxTheApp->GetAppName(),
			wxDefaultPosition,
			wxSize(640, 480));
		frame->Show(true);

		SetTopWindow(frame);

		return true;
	}
	DKApplication app;
};

wxIMPLEMENT_APP(DKPropertyEditorApp);

// DKMediaPlayer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DKMediaPlayer.h"
#include "MainWindow.h"

class DKMediaPlayerApp : public wxApp
{
public:
	bool OnInit(void)
	{
//		if (!wxApp::OnInit())
//			return false;

#ifndef DKLIB_DEBUG_ENABLED
		struct AvoidLog {static void Log(const DKString&) {}};
		DKLogInit(AvoidLog::Log);
#endif
		wxArrayString openFiles;
		for (int i = 1; i < this->argc; ++i)
			openFiles.Add((const wchar_t*)argv[i]);

		MainWindow* frame = new MainWindow(wxTheApp->GetAppName(), wxDefaultPosition, wxSize(640, 480),
			wxCAPTION|wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxCLOSE_BOX|wxRESIZE_BORDER|wxSYSTEM_MENU );
		frame->Show(true);

		SetTopWindow(frame);

		if (openFiles.Count() > 0)
			MacOpenFiles(openFiles);

		return true;
	}
	void MacOpenFiles(const wxArrayString& fileNames)	
	{
		((MainWindow*)GetTopWindow())->OpenFiles(fileNames);
	}

	DKApplication app;
};

wxIMPLEMENT_APP(DKMediaPlayerApp);

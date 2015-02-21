// DKModelEditor.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DKModelEditor.h"
#include "MainWindow.h"

class DKModelEditorApp : public wxApp
{
public:
	bool OnInit(void)
	{
		if (!wxApp::OnInit())
			return false;

#ifndef DKLIB_DEBUG_ENABLED
		struct AvoidLog {static void Log(const DKString&) {}};
		DKLogInit(AvoidLog::Log);
#endif

		MainWindow* frame = new MainWindow(
			wxTheApp->GetAppName(),
			wxDefaultPosition,
			wxSize(1280, 800));
		frame->Show(true);

		SetTopWindow(frame);

		return true;
	}
	DKApplication app;
};

wxIMPLEMENT_APP(DKModelEditorApp);

void PrintLog(const wxString& msg, const wxColour& c)
{
	reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->PrintLog(msg, c);
}

DKModel* GetModel(void)
{
	return reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->GetModel();
}

void SetModelModified(void)
{
	reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->SetModelModified();
}

DKObject<DKMaterial> OpenMaterial(void)
{
	return reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->OpenMaterial();
}

DKObject<DKTexture> OpenTexture(void)
{
	return reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->OpenTexture();
}

void LockRenderer(void)
{
	reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->LockRenderer();
}

void UnlockRenderer(void)
{
	reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->UnlockRenderer();
}

void SetActiveItem(ModelNodeItemData* item)
{
	reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->SetActiveItem(item);
}

void UpdateTreeItem(wxTreeItemId item)
{
	reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->UpdateTreeItem(item);
}

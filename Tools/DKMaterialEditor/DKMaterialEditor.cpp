// DKMaterialEditor.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DKMaterialEditor.h"
#include "MainWindow.h"

class DKMaterialEditorApp : public wxApp
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

wxIMPLEMENT_APP(DKMaterialEditorApp);

void PrintLog(const wxString& msg, const wxColour& c)
{
	reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->PrintLog(msg, c);
}

DKMaterial* GetMaterial(void)
{
	return reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->GetMaterial();
}

void SetMaterialModified(void)
{
	reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->SetMaterialModified();
}

void OpenEditor(DKMaterial::ShaderSource* shader)
{
	reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->OpenEditor(shader);
}

bool CloseEditor(DKMaterial::ShaderSource* shader)
{
	return reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->CloseEditor(shader);
}

void UpdateEditor(DKMaterial::ShaderSource* shader)
{
	reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->UpdateEditor(shader);
}

void OpenProperties(MaterialNodeItemData* item)
{
	reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->OpenProperties(item);
}

void UpdateTreeItem(wxTreeItemId item)
{
	reinterpret_cast<MainWindow*>(wxGetApp().GetTopWindow())->UpdateTreeItem(item);
}

#pragma once
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/aui/auibook.h>
#include <wx/stc/stc.h>
#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class ShaderEditor : public wxAuiNotebook
{
public:
	enum UIControl
	{
	};
	enum UICommand
	{
		UICommandBegin = wxID_HIGHEST,
		UICommandTest,
	};
	class PageData : public wxClientData
	{
	public:
		PageData(DKMaterial::ShaderSource* p) : shader(p) {}
		DKMaterial::ShaderSource*	shader;
	};
	struct Block
	{
		DKString	begin;
		DKString	end;
		wxColour	color;
	};
	ShaderEditor(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxAUI_NB_DEFAULT_STYLE);
	~ShaderEditor(void);

	void Open(DKMaterial::ShaderSource* shader);		// 소스 편집기 열기
	bool Close(DKMaterial::ShaderSource* shader);		// 해당 소스창만 닫기
	void CloseAll(void);								// 모든 소스창 닫기
	void Update(DKMaterial::ShaderSource* shader);		// 해당 쉐이더가 열려있다면 갱신

	int FindPage(DKMaterial::ShaderSource* shader) const;
	void OnPageClose(wxAuiNotebookEvent& e);

	void OnChange(wxStyledTextEvent &e);
	void OnCharAdded(wxStyledTextEvent &e);
	void OnUpdateUI(wxStyledTextEvent &e);
	void OnMarginClick(wxStyledTextEvent &e);

private:
	void SetInternalStyle(wxStyledTextCtrl* ctrl, int style, wxFont& fnt, const wxColour& fc, const wxColour& bc, bool bold = false, bool italic = false, bool underline = false, bool visible = true, int caseforce = wxSTC_CASE_MIXED);
	int lineNoMarker;
	int folderMarker;

	wxDECLARE_EVENT_TABLE();
};

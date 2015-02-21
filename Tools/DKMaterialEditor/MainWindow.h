#pragma once
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/treectrl.h>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include "RenderWindow.h"
#include "MaterialProperties.h"
#include "MaterialTree.h"
#include "ShaderEditor.h"

class MainWindow : public wxFrame
{
	enum UIControl
	{
	};
	enum UICommand
	{
		UICommandBegin = wxID_HIGHEST,
		UICommandViewRenderProperties,
		UICommandViewMaterialProperties,
		UICommandViewLightingProperties,
		UICommandViewSceneProperties,
		UICommandViewScreenSelector,
		UICommandViewLog,
	};
public:
	MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size, long style = wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	~MainWindow(void);

	void PrintLog(const wxString& msg, const wxColour& c = wxColour(0,0,0));
	DKMaterial* GetMaterial(void);
	void SetMaterialModified(void);
	void OpenEditor(DKMaterial::ShaderSource* shader);		// ShaderEditor 에 소스 열기
	bool CloseEditor(DKMaterial::ShaderSource* shader);		// ShaderEditor 에서 열려있는 소스 닫기
	void UpdateEditor(DKMaterial::ShaderSource* shader);	// ShaderEditor 텍스트 갱신 (열려있으면 갱신)
	void OpenProperties(MaterialNodeItemData* item);		// MaterialProperties 갱신
	void UpdateTreeItem(wxTreeItemId item);				// MaterialTree 의 텍스트 갱신

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

	void OnViewRenderProperties(wxCommandEvent& e);
	void OnViewMaterialProperties(wxCommandEvent& e);
	void OnViewLightingProperties(wxCommandEvent& e);
	void OnViewSceneProperties(wxCommandEvent& e);
	void OnViewLog(wxCommandEvent& e);

	void OnToolsOptions(wxCommandEvent& e);
	void OnHelpAbout(wxCommandEvent& e);

	void OnSize(wxSizeEvent& e);
	void OnClose(wxCloseEvent& e);

	void UpdateTitle(void);

	wxAuiManager	auiManager;		// Advanced UI Manager

	MaterialTree*		materialTree;			// 머티리얼 노드 트리
	MaterialProperties*	materialProperties;		// 머티리얼 노드 속성
	wxWindow*			sceneProperties;		// 모델, 시야, 행렬 유니폼 등등
	ShaderEditor*		shaderEditor;

	// 모달 다이얼로그들
	wxWindow*		textureProperties;
	wxWindow*		blendProperties;
	wxWindow*		streamProperties;
	wxWindow*		colorPicker;

	// 메인 뷰어
	RenderWindow*	renderWindow;		// 임시로 텍스트 콘트롤로 생성.
	// 로그 윈도우
	wxTextCtrl*		logWindow;

	DKObject<DKMaterial>		material;
	bool						modified;
	DKString					materialPath;

	wxDECLARE_EVENT_TABLE();
};

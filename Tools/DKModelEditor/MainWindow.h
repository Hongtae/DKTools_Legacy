#pragma once
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/treectrl.h>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include <wx/propgrid/propgrid.h>
#include "RenderWindow.h"
#include "ModelTree.h"
#include "ModelProperties.h"

class MainWindow : public wxFrame
{
	enum UICommand
	{
		UICommandBegin = wxID_HIGHEST,

		UICommandFileFormatXML,
		UICommandFileFormatXMLb,
		UICommandFileFormatBin,
		UICommandFileFormatCBin,

		UICommandViewNodeTree,
		UICommandViewResources,
		UICommandViewScreenController,
		UICommandViewAnimation,
		UICommandViewLog,
	};
	enum UIControl
	{
		UIControlBegin = wxID_HIGHEST,
	};
public:
	MainWindow(const wxString& title, const wxPoint& pos, const wxSize& size, long style = wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	~MainWindow(void);

	void PrintLog(const wxString& msg, const wxColour& c = wxColour(0,0,0));
	DKModel* GetModel(void);
	void SetModelModified(void);

	DKObject<DKMaterial> OpenMaterial(void);
	DKObject<DKTexture> OpenTexture(void);

	void LockRenderer(void) const;
	void UnlockRenderer(void) const;
	void SetActiveItem(ModelNodeItemData* item);	// 선택된 아이템 정보 갱신
	void UpdateTreeItem(wxTreeItemId item);			// ModelTree 의 텍스트 갱신

private:
	void OnFileNew(wxCommandEvent& e);
	void OnFileOpen(wxCommandEvent& e);
	void OnFileSave(wxCommandEvent& e);
	void OnFileSaveAs(wxCommandEvent& e);
	void OnFileFormat(wxCommandEvent& e);
	void OnFileQuit(wxCommandEvent& e);

	void OnEditCut(wxCommandEvent& e);
	void OnEditCopy(wxCommandEvent& e);
	void OnEditPaste(wxCommandEvent& e);
	void OnEditDelete(wxCommandEvent& e);

	void OnViewNodeTree(wxCommandEvent& e);
	void OnViewResources(wxCommandEvent& e);
	void OnViewScreenController(wxCommandEvent& e);
	void OnViewAnimation(wxCommandEvent& e);
	void OnViewLog(wxCommandEvent& e);

	void OnToolsOptions(wxCommandEvent& e);
	void OnHelpAbout(wxCommandEvent& e);

	void OnSize(wxSizeEvent& e);
	void OnClose(wxCloseEvent& e);

	void UpdateTitle(void);

	// 메인 UI 매니져
	wxAuiManager	auiManager;					// Advanced UI Manager

	// 모델 관련 데이터
	ModelTree*			nodeTree;
	ModelProperties*	nodeProperties;

	// 렌더링 프로퍼티 그룹에 포함될 윈도우들 (모델 데이터와는 상관 없는 것들)
	wxAuiNotebook*	screenControllerGroup;		// 스크린 제어, 컬러 그리드 등등..
	wxWindow*		sceneController;			// 컬러, 그리드, 카메라
	wxWindow*		lightingController;
	wxWindow*		animationController;

	// 메인 뷰어
	RenderWindow*	renderWindow;

	// 로그 윈도우
	wxTextCtrl*		logWindow;

	DKObject<DKModel>			model;			// 현재 작업중인 모델
	DKObject<DKModel>			displayModel;	// 화면에 보여지는 모델
	bool						modified;
	DKString					modelPath;
	DKSerializer::SerializeForm	fileFormat;		// 파일 저장용 포맷

	wxDECLARE_EVENT_TABLE();
};

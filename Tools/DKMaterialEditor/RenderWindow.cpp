#include "StdAfx.h"
#include "RenderWindow.h"


void PrintLog(const wxString& msg, const wxColour& c = wxColour(0,0,0));

wxBEGIN_EVENT_TABLE(RenderWindow, wxWindow)
	EVT_SIZE(RenderWindow::OnSize)
	EVT_CLOSE(RenderWindow::OnClose)
	EVT_MOUSE_EVENTS(RenderWindow::OnMouseEvent)
wxEND_EVENT_TABLE();

RenderWindow::RenderWindow(wxWindow *parent,
						   wxWindowID id,
						   const wxPoint& pos,
						   const wxSize& size,
						   const wxString& name)
						   : wxWindow(parent, id, pos, size, wxTRANSPARENT_WINDOW | wxBORDER_NONE , name)
{

	// 실제 렌더링될 윈도우를 생성하여 wxWidgets 으로 변환하고 기존 wxWidgets 의 자식으로 붙이는 방법을 연구
	// 이미 생성된 wxWidgets 을 사용하는것보다 나을거 같음.
	
	//SetExtraStyle(wxWS_EX_TRANSIENT);
	SetBackgroundColour(wxColour(255,0,255));
//	windowSize = GetSize();
}

RenderWindow::~RenderWindow(void)
{
}

bool RenderWindow::InitScreen(void)
{
	window = DKWindow::CreateProxy(GetHandle());
	
	if (window)
	{
		PrintLog("Proxy Window created.\n");
		// 렌더러 생성 및 연계
		screen = DKObject<DKScreen>::New();
		if (screen->Run(window, &frame))
		{
		//	screen->SetMinimumActiveFrameTime(1.0f/30.0f);		// 활성시 30 fps 로 한다
		//	screen->SetMinimumInactiveFrameTime(1.0f/10.0f);	// 비활성시 10 fps 로 한다
			// 딜레이를 적용시키기 위해 액티베이트 시킨다.
			window->PostWindowEvent(DKWindow::EventWindowActivated, window->ContentSize(), window->Origin(), false);
			DKOpenGLContext::SharedInstance()->Bind();
			return true;
		}
		else
		{
			PrintLog("Error: Failed to create renderer.\n", wxColour(255,0,0));
		}
	}
	else
	{
		PrintLog("Error: cannot create Proxy Window.\n", wxColour(255,0,0));
	}

	window = NULL;
	screen = NULL;

	return false;
}

void RenderWindow::OnSize(wxSizeEvent& e)
{
	e.Skip();

	if (window)
		window->UpdateProxy();
/*	
	wxSize size = e.GetSize();
	if (size != windowSize)
	{
		wxString str = wxString::Format("Resize screen (%d x %d)\n", size.x, size.y);
		//wxLogWarning(str);
		PrintLog(str);
		windowSize = size;

		window->UpdateProxy();
	}
*/ 
}

void RenderWindow::OnClose(wxCloseEvent& e)
{
	screen->Terminate(true);
	screen = NULL;
	window = NULL;
	e.Skip();
}

void RenderWindow::OnMouseEvent(wxMouseEvent& e)
{
	e.Skip();

	if (window && screen)
	{
		DKPoint	pos(e.m_x, GetClientSize().GetHeight() - e.m_y);
		DKVector2 delta = pos.Vector() - mousePosition.Vector();
		mousePosition = pos;

		int id = e.GetEventType();

		if		(id == wxEVT_LEFT_DOWN)			window->PostMouseEvent(DKWindow::EventMouseDown,	0, 0, mousePosition, delta, false);
		else if (id == wxEVT_LEFT_DCLICK)		window->PostMouseEvent(DKWindow::EventMouseDown,	0, 0, mousePosition, delta, false);
		else if (id == wxEVT_LEFT_UP)			window->PostMouseEvent(DKWindow::EventMouseUp,		0, 0, mousePosition, delta, false);
		else if (id == wxEVT_RIGHT_DOWN)		window->PostMouseEvent(DKWindow::EventMouseDown,	0, 1, mousePosition, delta, false);
		else if (id == wxEVT_RIGHT_DCLICK)		window->PostMouseEvent(DKWindow::EventMouseDown,	0, 1, mousePosition, delta, false);
		else if (id == wxEVT_RIGHT_UP)			window->PostMouseEvent(DKWindow::EventMouseUp,		0, 1, mousePosition, delta, false);
		else if (id == wxEVT_MIDDLE_DOWN)		window->PostMouseEvent(DKWindow::EventMouseDown,	0, 2, mousePosition, delta, false);
		else if (id == wxEVT_MIDDLE_DCLICK)		window->PostMouseEvent(DKWindow::EventMouseDown,	0, 2, mousePosition, delta, false);
		else if (id == wxEVT_MIDDLE_UP)			window->PostMouseEvent(DKWindow::EventMouseUp,		0, 2, mousePosition, delta, false);
		else if (id == wxEVT_MOTION)			window->PostMouseEvent(DKWindow::EventMouseMove,	0, 0, mousePosition, delta, false);
		else if (id == wxEVT_ENTER_WINDOW)		window->PostMouseEvent(DKWindow::EventMouseMove,	0, 0, mousePosition, delta, false);
		else if (id == wxEVT_LEAVE_WINDOW)		window->PostMouseEvent(DKWindow::EventMouseMove,	0, 0, mousePosition, delta, false);
		else if (id == wxEVT_MOUSEWHEEL)
		{
			window->PostMouseEvent(DKWindow::EventMouseWheel,	0, 0, mousePosition, DKVector2(0, e.m_wheelRotation > 0 ? 1 : (e.m_wheelRotation < 0 ? -1 : 0)), false);
		}
		else
		{
			//wxEVT_ENTER_WINDOW:
			//wxEVT_LEAVE_WINDOW:
		}
	}
}


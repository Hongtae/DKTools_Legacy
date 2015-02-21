// TestApp4.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <new>
#include "TestApp4.h"

#include "TestFrame1.h"

#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

DKString g_resDir;

class TestApp4 : public DKApplication
{
public:
	void OnInitialize()
	{
		// 리소스 디렉토리
		DKObject<DKDirectory> resDir = DKDirectory::OpenDir(this->EnvironmentPath(SystemPathAppExecutable));
		for (int i = 0; i < 2; ++i)
			if (resDir)
				resDir = resDir->OpenParent();
		if (resDir)
			resDir = resDir->OpenSubdir("Resources");
		if (resDir)
			g_resDir = resDir->AbsolutePath();
		else
			g_resDir = DKDirectory::OpenDir(EnvironmentPath(SystemPathAppResource))->AbsolutePath();

		DKLog("Current resource dir = %ls\n", (const wchar_t*)g_resDir);
		
		DKRect rcScreen = DisplayBounds(0);
		DKRect rcWorkspace = ScreenContentBounds(0);
		DKLog("Screen: %d x %d\n", (int)rcScreen.size.width, (int)rcScreen.size.height);
		DKLog("Workspace: (%d,%d), size: %d x %d.\n", (int)rcWorkspace.origin.x, (int)rcWorkspace.origin.y, (int)rcWorkspace.size.width, (int)rcWorkspace.size.height);

		DKRect rcWindow = rcWorkspace;
		rcWindow.size.width = Min<float>(rcWindow.size.width, 1024.0);
		rcWindow.size.height = Min<float>(rcWindow.size.height, 1024.0);
		window = DKWindow::Create("TestApp4", rcWindow.size);
		if (window)
		{
			screen = DKObject<DKScreen>::New();
			if (screen->Run(window, DKObject<TestFrame1>::New()))
			{
				screen->SetActiveFrameLatency(0.0);
				screen->SetInactiveFrameLatency(0.0);
				//	screen->SetActiveFrameLatency(1.0/60.0);
				//	screen->SetInactiveFrameLatency(1.0/30.0);	

				window->Activate();
				return;
			}
		}
		Terminate(0);
	}
	void OnTerminate()
	{
		screen->Terminate(true);
		window = NULL;
		screen = NULL;
	}
private:
	DKObject<DKWindow> window;
	DKObject<DKScreen> screen;
};

#ifdef WIN32
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
#else
	int main(int argc, char* argv[])
#endif
{
	return TestApp4().Run();
}

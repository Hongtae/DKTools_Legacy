#include "stdafx.h"
#include "TestFrame1.h"
#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

DKString g_resDir;

class TestApp5 : public DKApplication
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
			g_resDir = DKDirectory::OpenDir(this->EnvironmentPath(SystemPathAppResource))->AbsolutePath();

		DKLog("Current resource dir = %ls\n", (const wchar_t*)g_resDir);
		
		DKRect rcWorkspace = ScreenContentBounds(0);
		DKRect rcScreen = DisplayBounds(0);

		DKRect rcWindow = rcWorkspace;
		rcWindow.size.width = Min<float>(rcWindow.size.width, 1024.0);
		rcWindow.size.height = Min<float>(rcWindow.size.height, 1024.0);

		window = DKWindow::Create("TestApp5", rcWindow.size);
		if (window)
		{
			screen = DKObject<DKScreen>::New();
			if (screen->Run(window, DKObject<TestFrame1>::New()))
			{
				screen->SetActiveFrameLatency(0);
				screen->SetInactiveFrameLatency(0);
				//	screen->SetMinimumActiveFrameTime(1.0f/30.0f);
				//	screen->SetMinimumInactiveFrameTime(1.0f/30.0f);

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
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR    lpCmdLine,
                     int       nCmdShow)
#else
	int main(int argc, char* argv[])
#endif
{
	return TestApp5().Run();
}

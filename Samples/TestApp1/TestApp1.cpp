// TestApp1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TestApp1.h"

#include "TestFrame1.h"

#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

DKString g_resDir;

class TestApp1 : public DKApplication
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
			
		window1 = DKWindow::Create("TestApp1", DKSize(640,480));
		screen1 = DKObject<DKScreen>::New();
		screen1->Run(window1, DKObject<TestFrame1>::New());
		window1->Activate();

	//	window2 = DKWindow::Create("TestApp1", DKRect(DKPoint(800,200), DKSize(640,480)), true, true);
	//	screen2 = DKScreen::Create(window2, DKObject<TestFrame1>::New());
	//	window2->Activate();

		return;
	}
	void OnTerminate()
	{
		screen1->Terminate(true);
	//	screen2->Terminate(true);

		DKLog("screen1->IsRunning:%d\n", screen1->IsRunning());
	//	DKLog("screen2->IsRunning:%d\n", screen2->IsRunning());
		DKLog("screen1 ref:%d\n", screen1.SharingCount());
	//	DKLog("screen2 ref:%d\n", screen2.SharingCount());

		screen1 = NULL;
	//	screen2 = NULL;

		DKLog("window1 ref:%d\n", window1.SharingCount());
	//	DKLog("window2 ref:%d\n", window2.SharingCount());

		window1 = NULL;
	//	window2 = NULL;

		DKLog("terminated\n");
	}
private:
	DKObject<DKWindow> window1;
//	DKObject<DKWindow> window2;
	DKObject<DKScreen> screen1;
//	DKObject<DKScreen> screen2;
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
	return TestApp1().Run();
}

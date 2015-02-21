#include "stdafx.h"  
#include "TestApp3.h"  

#include "TestFrame1.h"
#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

DKObject<DKResourcePool>	resourcePool = NULL;

class TestApp3 : public DKApplication
{
public:
	void OnInitialize()
	{
		DKString resourcePath = L"";

		// 리소스 디렉토리
		DKObject<DKDirectory> resDir = DKDirectory::OpenDir(this->EnvironmentPath(SystemPathAppExecutable));
		for (int i = 0; i < 2; ++i)
			if (resDir)
				resDir = resDir->OpenParent();
		if (resDir)
			resDir = resDir->OpenSubdir("Resources");
		if (resDir)
			resourcePath = resDir->AbsolutePath();
		else
			resourcePath = DKDirectory::OpenDir(EnvironmentPath(SystemPathAppResource))->AbsolutePath();

		DKLog("Current resource dir = %ls\n", (const wchar_t*)resourcePath);
		
		resourcePool = DKObject<DKResourcePool>::New();
		resourcePool->AddSearchPath(resourcePath);
		resourcePool->AddSearchPath(resourcePath + "/Fonts");
	//	resourcePool->AddSearchPath(resourcePath + "/AppData.zip");
	//	resourcePool->AddSearchPath(resourcePath + "/kon.zip");
		resourcePool->AddSearchPath(EnvironmentPath(SystemPathAppResource));

		DKRect rcWorkspace = ScreenContentBounds(0);
		DKRect rcScreen = DisplayBounds(0);

		DKRect rcWindow = rcWorkspace;
		rcWindow.size.width = Min<float>(rcWindow.size.width, 1024.0);
		rcWindow.size.height = Min<float>(rcWindow.size.height, 1024.0);

		window = DKWindow::Create("TestApp3", rcWindow.size);
		if (window)
		{
			screen = DKObject<DKScreen>::New();
			if (screen->Run(window, DKObject<TestFrame1>::New()))
			{
				window->Activate();
				return;
			}
		}
		Terminate(0);
	}
	void OnTerminate()
	{
		screen->Terminate(true);
		resourcePool = NULL;

		DKLog("window ref:%d\n", window.SharingCount());
		DKLog("screen ref:%d\n", screen.SharingCount());

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
	return TestApp3().Run();
}

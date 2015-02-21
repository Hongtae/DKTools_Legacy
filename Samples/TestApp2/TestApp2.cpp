// TestApp2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TestApp2.h"
#include "TestFrame1.h"

DKString g_resDir;

void PrintDir(const DKDirectory* dir)
{
	DKLog("Directory = \"%ls\" (dirs:%d, files:%d)\n", (const wchar_t*)dir->AbsolutePath(), dir->NumberOfSubdirs(), dir->NumberOfFiles());
	for (int i = 0; i < dir->NumberOfSubdirs(); i++)
	{
		DKLog("[%ls]\n", (const wchar_t*)dir->SubdirNameAtIndex(i));
	}
	for (int i = 0; i < dir->NumberOfFiles(); i++)
	{
		DKLog("%ls\n", (const wchar_t*)dir->FileNameAtIndex(i));
	}
}

class TestApp2 : public DKApplication
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
		
		// 윈도우, 렌더러 생성
		window = DKWindow::Create("TestApp2", DKSize(800,600));
		if (window)
		{
			screen = DKObject<DKScreen>::New();
			if (screen->Run(window, DKObject<TestFrame1>::New()))
			{
				//screen->SetActiveFrameLatency(0);
				//screen->SetInactiveFrameLatency(0);	

				window->Activate();
				return;
			}
			else
				DKLog("DKScreen creation failed.\n");
		}
		else
			DKLog("DKWindow creation failed.\n");
		
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
	return TestApp2().Run();
}

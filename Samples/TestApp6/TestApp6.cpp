#include "stdafx.h"
#include <DK.h>
#include <PyDKPython.h>
#include <PyDKInterpreter.h>
#include <PyDKApplication.h>

using namespace DKFoundation;
using namespace DKFramework;

class TestApp6 : public PyDKApplication
{
public:
	DKObject<PyDKPython> python;
	DKObject<DKThread> thread;
	DKString scriptPath;

	void OnInitialize(void)
	{
		// PyDK 경로 설정
		const char* pythonZip = "python34.zip";
		const char* mainScript = "TestApp.py";
		const char* scriptsDir = "Scripts";

		DKString pythonPath;
		DKObject<DKDirectory> scriptRoot;

		// 윈도우 환경의 빌드 경로
		// +- Resources (리소스)
		// |         +- python34.zip
		// |
		// +- Build -+- win32_Debug    (윈도우의 output 위치)
		// |         +- win32_Release
		// |         +- x64_Debug
		// |         +- x64_Release
		// |
		// +- Scripts  (스크립트 디렉토리)
		// |
		// ...
		DKObject<DKDirectory> projDir = DKDirectory::OpenDir(this->EnvironmentPath(SystemPathAppResource));
		for (int i = 0; i < 2; ++i)
			if (projDir)
				projDir = projDir->OpenParent();

		DKObject<DKDirectory> appRes = DKDirectory::OpenDir(this->EnvironmentPath(SystemPathAppResource));

		if (projDir && projDir->IsSubdirExist("Resources"))
			pythonPath = projDir->OpenSubdir("Resources")->AbsolutePathOfFile(pythonZip);
		else
			pythonPath = appRes->AbsolutePathOfFile(pythonZip);

		if (projDir && projDir->IsSubdirExist(scriptsDir))
			scriptRoot = projDir->OpenSubdir(scriptsDir);
		else
			scriptRoot = appRes->OpenSubdir(scriptsDir);

		if (scriptRoot)
			scriptPath = scriptRoot->AbsolutePathOfFile(mainScript);

		if (pythonPath.Length() > 0 && scriptPath.Length() > 0)
		{
			DKLog("pythonPath: %ls\n", (const wchar_t*)pythonPath);
			DKLog("scriptPath: %ls\n", (const wchar_t*)scriptPath);

			this->python = PyDKPython::Create({ pythonPath, scriptRoot->AbsolutePath() });
			if (this->python)
			{
				auto runScript = [this]()
				{
					this->python->RunFile(this->scriptPath);
					this->CancelScriptBinding();
					this->Terminate(0);
				};

				this->thread = DKThread::Create(DKFunction(runScript)->Invocation());
				return PyDKApplication::OnInitialize();
			}
		}
		Terminate(0);
	}
	void OnTerminate(void)
	{
		if (python)
		{
			PyDKApplication::OnTerminate();
			this->thread->WaitTerminate();
			python = NULL;
		}
	}
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
	return TestApp6().Run();
}

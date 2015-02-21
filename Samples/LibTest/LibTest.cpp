#include "stdafx.h"
#include "TestCase.h"
#include <DK.h>

using namespace DKFoundation;
using namespace DKFramework;


class LibTest : public DKApplication
{
public:
	void OnInitialize()
	{
		TestCase::Run();
		Terminate(0);
	}
	void OnTerminate()
	{
	}
private:
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
	return LibTest().Run();
}

call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\x86_amd64\vcvarsx86_amd64.bat"

cd trunk\build\msw
nmake -f makefile.vc TARGET_CPU=x64 BUILD=release UNICODE=1 SHARED=0 MONOLITHIC=1 DEBUG_FLAG=0 DEBUG_INFO=0
cd ..\..\..

pause
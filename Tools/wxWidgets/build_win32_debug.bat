call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\vcvars32.bat"

cd trunk\build\msw
nmake -f makefile.vc BUILD=debug UNICODE=1 SHARED=0 MONOLITHIC=1 DEBUG_FLAG=2 DEBUG_INFO=1
cd ..\..\..

pause
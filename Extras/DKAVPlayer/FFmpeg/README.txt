MINGW 라이브러리는
x64 로 빌드할때는 라이브러리 내에서 #pragma comment 를 사용해서 링크하면 안되며,
DKLib 에 Additional Dependencies 에 넣어서 명시적으로 DKLib 안에 포함시키거나
실행파일에 링크하여야 한다.

이 라이브러리는
FFmpeg/mingw_base 프로젝트로 생성된 라이브러리를 써야한다.
#pragma once


#ifdef _WIN32
#if defined(_DEBUG) || defined(DEBUG)
// 2011-10-21: 윈도우에서 FFmpeg 가 static 이며 release 로만 빌드되었기 때문에 디버그에선 msvcrt.lib 를 제거해야함
#pragma comment(linker, "/NODEFAULTLIB:msvcrt")
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "./include/libavcodec/avcodec.h"
#include "./include/libavdevice/avdevice.h"
#include "./include/libavfilter/avfilter.h"
#include "./include/libavformat/avformat.h"
#include "./include/libswscale/swscale.h"
#include "./include/libswresample/swresample.h"
#include "./include/libavutil/avutil.h"
#include "./include/libavutil/opt.h"

#ifdef __cplusplus
}
#endif

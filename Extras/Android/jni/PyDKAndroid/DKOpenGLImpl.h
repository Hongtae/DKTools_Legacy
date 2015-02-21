//
//  File: DKOpenGLImpl.h
//  Encoding: UTF-8 ☃
//  Platform: Android
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2013,2014 ICONDB.COM. All rights reserved.
//

#pragma once

#ifdef __ANDROID__
#include <DKFoundation.h>
#include <DKFramework/Interface/DKOpenGLInterface.h>
#include <DKFramework/DKWindow.h>
#include <EGL/egl.h>
#include <pthread.h>

using namespace DKFoundation;
using namespace DKFramework;

class DKWindowImpl;
class DKOpenGLImpl : public DKOpenGLInterface
{
public:
	DKOpenGLImpl(void);
	~DKOpenGLImpl(void);

	void Bind(void* target) const;		// 쓰레드 바인딩
	void Unbind(void) const;			// 쓰레드 언바인딩
	bool IsBound(void) const;

	void Flush(void) const;				// glFlush 및 디버깅 체크
	void Finish(void) const;			// glFinish 및 디버깅 체크
	void Present(void) const;			// 버퍼 스왑

	void Update(void) const;			// 윈도우 상태 갱신

	bool GetSwapInterval(void) const;
	void SetSwapInterval(bool interval) const;

	unsigned int FramebufferId(void) const		{return 0;}

	EGLContext mainContext;
	EGLDisplay display;
	EGLConfig config;

	struct BoundContext
	{
		EGLContext			ctxt;
		EGLSurface			surface;
		DKWindowImpl*		window;
		ANativeWindow*		target;
		int					count;
	};

	// 콘텍스트 관련 풀
	typedef DKFoundation::DKMap<pthread_t, BoundContext, DKSpinLock> SharedContextMap;
	mutable SharedContextMap		sharedContexts;
};

#endif // ifdef __ANDROID__

//
//  File: DKOpenGLImpl.cpp
//  Encoding: UTF-8 ☃
//  Platform: Android
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2013,2014 ICONDB.COM. All rights reserved.
//


#ifdef __ANDROID__
#include <jni.h>
#include <android/log.h>
#include "DKOpenGLImpl.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include "DKWindowImpl.h"

DKWindowImpl* WindowImplWithPlatformHandle(void* handle);

#define  ANDROID_LOG_TAG    "DKOpenGLImpl"
#define  ANDROID_LOG_I(...)  __android_log_print(ANDROID_LOG_INFO, ANDROID_LOG_TAG, __VA_ARGS__)
#define  ANDROID_LOG_D(...)  __android_log_print(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, __VA_ARGS__)
#define  ANDROID_LOG_E(...)  __android_log_print(ANDROID_LOG_ERROR, ANDROID_LOG_TAG, __VA_ARGS__)

static PFNGLBINDVERTEXARRAYOESPROC		glBindVertexArrayOESProc = NULL;
static PFNGLDELETEVERTEXARRAYSOESPROC	glDeleteVertexArraysOESProc = NULL;
static PFNGLGENVERTEXARRAYSOESPROC		glGenVertexArraysOESProc = NULL;
static PFNGLISVERTEXARRAYOESPROC		glIsVertexArrayOESProc = NULL;

extern "C" {
	GL_APICALL void GL_APIENTRY glBindVertexArrayOES (GLuint array)
	{
		return glBindVertexArrayOESProc(array);
	}

	GL_APICALL void GL_APIENTRY glDeleteVertexArraysOES (GLsizei n, const GLuint *arrays)
	{
		return glDeleteVertexArraysOESProc(n, arrays);
	}

	GL_APICALL void GL_APIENTRY glGenVertexArraysOES (GLsizei n, GLuint *arrays)
	{
		return glGenVertexArraysOESProc(n, arrays);
	}

	GL_APICALL GLboolean GL_APIENTRY glIsVertexArrayOES (GLuint array)
	{
		return glIsVertexArrayOESProc(array);
	}
}

static const EGLint contextAttribs[] = {
	EGL_CONTEXT_CLIENT_VERSION, 2,
	EGL_NONE
};

static const EGLint pbufferAttribs[] = {
	EGL_TEXTURE_TARGET, EGL_NO_TEXTURE,
	EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE,
	EGL_HEIGHT, 4,
	EGL_WIDTH, 4,
	EGL_NONE
};

static const EGLint surfaceAttribs[] = {
	EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
	EGL_NONE
};

DKOpenGLInterface* DKOpenGLInterface::CreateInterface(DKOpenGLContext*)
{
	return new DKOpenGLImpl();
}

DKOpenGLImpl::DKOpenGLImpl(void)
: mainContext(NULL)
, display(NULL)
, config(NULL)
{
	if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        ANDROID_LOG_E("eglGetDisplay() returned error %d", eglGetError());
		DKERROR_THROW("eglGetDisplay failed");
    }
    if (!eglInitialize(display, 0, 0)) {
        ANDROID_LOG_E("eglInitialize() returned error %d", eglGetError());
		DKERROR_THROW("eglInitialize failed");
    }
	if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        ANDROID_LOG_E("eglBindAPI() returned error %d", eglGetError());
		DKERROR_THROW("eglBindAPI failed");
	}

    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT|EGL_PBUFFER_BIT,
		EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_NONE
    };
    EGLint numConfigs;

    if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs)) {
        ANDROID_LOG_E("eglChooseConfig() returned error %d", eglGetError());
		DKERROR_THROW("eglChooseConfig failed");
    }

	EGLSurface surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
	if (surface == NULL)
	{
		ANDROID_LOG_E("eglCreatePbufferSurface failed");
	}
	else
	{
		ANDROID_LOG_I("eglCreatePbufferSurface succeed!");
	}

	mainContext = eglCreateContext(display, config, NULL, contextAttribs);
	if (mainContext == NULL)
	{
		ANDROID_LOG_E("eglCreateContext() returned error %d", eglGetError());
		DKERROR_THROW("eglCreateContext failed");
	}

	if (eglMakeCurrent(display, surface, surface, mainContext) == EGL_FALSE) {
        ANDROID_LOG_E("Unable to eglMakeCurrent");
		DKERROR_THROW("Unable to eglMakeCurrent");
    }

	auto initExt = []()->bool
	{
		glBindVertexArrayOESProc = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
		glDeleteVertexArraysOESProc = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");
		glGenVertexArraysOESProc = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
		glIsVertexArrayOESProc = (PFNGLISVERTEXARRAYOESPROC)eglGetProcAddress("glIsVertexArrayOES");
		ANDROID_LOG_I("glBindVertexArrayOES: %p\n", glBindVertexArrayOESProc);
		ANDROID_LOG_I("glDeleteVertexArraysOES: %p\n", glDeleteVertexArraysOESProc);
		ANDROID_LOG_I("glGenVertexArraysOES: %p\n", glGenVertexArraysOESProc);
		ANDROID_LOG_I("glIsVertexArrayOES: %p\n", glIsVertexArrayOESProc);

		return true;
	};
	static bool initOnce = initExt();

	const char* versionString = (char*)glGetString(GL_VERSION);
	ANDROID_LOG_I("OpenGL Initialized succeed! (GLVersion:%s)", versionString);

	eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	if (surface)
	{
		eglDestroySurface(display, surface);
	}
}

DKOpenGLImpl::~DKOpenGLImpl(void)
{
	eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglDestroyContext(display, mainContext);
	eglTerminate(display);
}

void DKOpenGLImpl::Bind(void* target) const
{
	pthread_t currentThreadId = pthread_self();
	SharedContextMap::Pair* p = sharedContexts.Find(currentThreadId);
	if (p)
	{
		DKASSERT_DEBUG(p->value.count >= 0);
		p->value.count++;
	}
	else
	{
		DKWindowImpl* window = NULL;
		EGLSurface surface = NULL;
		if (target)
			window = WindowImplWithPlatformHandle(target);

		EGLContext context = eglCreateContext(display, config, mainContext, contextAttribs);
		if (context)
		{
			BoundContext	bc = {context, surface, window, NULL, 1};
			if (!sharedContexts.Insert(currentThreadId, bc))
				DKERROR_THROW("SharedContext insert failed!");

			Update();
		}
		else
		{
			ANDROID_LOG_E("eglCreateContext() returned error %d", eglGetError());
			DKERROR_THROW("eglCreateContext failed");
		}
	}
}

void DKOpenGLImpl::Unbind(void) const
{
	pthread_t currentThreadId = pthread_self();

	SharedContextMap::Pair* p = sharedContexts.Find(currentThreadId);
	if (p)
	{
		BoundContext& bc = p->value;

		DKASSERT_DEBUG(bc.count >= 0);
		bc.count--;
		if (bc.count == 0)
		{
			glFinish();

			eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

			if (bc.surface)
				eglDestroySurface(display, bc.surface);
			if (bc.target)
				ANativeWindow_release(bc.target);

			eglDestroyContext(display, bc.ctxt);

			sharedContexts.Remove(currentThreadId);
		}
	}
	else
	{
		DKERROR_THROW("cannot find context");
	}
}

bool DKOpenGLImpl::IsBound(void) const
{
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
	{
		DKASSERT_DESC_DEBUG(p->value.ctxt == eglGetCurrentContext(), "Current Context Mismatch!");
		return true;
	}
	return false;
}

void DKOpenGLImpl::Flush(void) const
{
#ifdef DK_DEBUG_ENABLED
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
		glFlush();
	else
		DKLog("Error: No context for this thread(%x)\n", pthread_self());
#else
	glFlush();
#endif
}

void DKOpenGLImpl::Finish(void) const
{
#ifdef DK_DEBUG_ENABLED
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
		glFinish();
	else
		DKLog("Error: No context for this thread(%x)\n", pthread_self());
#else
	glFinish();
#endif
}

void DKOpenGLImpl::Present(void) const
{
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
	{
		BoundContext& bc = p->value;
		if (bc.surface)
		{
			if (eglSwapBuffers(display, bc.surface) != EGL_TRUE)
			{
				DKLog("eglSwapBuffers failed");
			}
		}
		else
			glFinish();
	}
}

void DKOpenGLImpl::Update(void) const
{
	SharedContextMap::Pair* p = sharedContexts.Find(pthread_self());
	if (p)
	{
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		BoundContext& bc = p->value;

		if (bc.window)
		{
			DKCriticalSection<DKSpinLock> guard(bc.window->nativeWindowLock);
			ANativeWindow* nativeWindow = bc.window->nativeWindow;
			if (bc.target)
			{
				if (bc.surface)
					eglDestroySurface(display, bc.surface);
				ANativeWindow_release(bc.target);
				bc.surface = NULL;
				bc.target = NULL;
			}

			bc.target = nativeWindow;
			if (bc.target)
			{
				ANativeWindow_acquire(bc.target);
				bc.surface = eglCreateWindowSurface(display, config, bc.target, surfaceAttribs);
			}
		}

		if (bc.surface == NULL)
		{
			bc.surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
		}

		eglSurfaceAttrib(display, bc.surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_DESTROYED);

		if (eglMakeCurrent(display, bc.surface, bc.surface, bc.ctxt) == EGL_FALSE) {
			ANDROID_LOG_E("Unable to eglMakeCurrent");
			DKERROR_THROW("Unable to eglMakeCurrent");
		}
	}
}

bool DKOpenGLImpl::GetSwapInterval(void) const
{
	return false;
}

void DKOpenGLImpl::SetSwapInterval(bool interval) const
{
}

#endif

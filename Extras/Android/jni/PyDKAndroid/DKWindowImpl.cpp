//
//  File: DKWindowImpl.cpp
//  Encoding: UTF-8 ☃
//  Platform: Android
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2013,2014 ICONDB.COM. All rights reserved.
//


#ifdef __ANDROID__
#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "DKWindowImpl.h"
#include "JNIOperationQueue.h"

#define  ANDROID_LOG_TAG    "DKWindowImpl"
#define  ANDROID_LOG_I(...)  __android_log_print(ANDROID_LOG_INFO, ANDROID_LOG_TAG, __VA_ARGS__)
#define  ANDROID_LOG_D(...)  __android_log_print(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, __VA_ARGS__)
#define  ANDROID_LOG_E(...)  __android_log_print(ANDROID_LOG_ERROR, ANDROID_LOG_TAG, __VA_ARGS__)

typedef DKMap<jlong, DKWindowImpl*> JNIWindowMap;

static JNIWindowMap windowMap;			// jin 생성된 윈도우 맵
static jweak windowManager = NULL;
static DKCondition windowCond;

DKWindowImpl* WindowImplWithPlatformHandle(void* handle)
{
	jlong keyValue = (jlong)handle;
	DKCriticalSection<DKCondition> guard(windowCond);
	auto p = windowMap.Find(keyValue);
	if (p)
	{
		DKASSERT_DEBUG((jlong)p->value->objectRef == keyValue);
		return p->value;
	}
	return NULL;
}

DKWindowImpl* WindowImplByJObjectLocalRef(JNIEnv* env, jobject obj)
{
	jclass objClass = env->GetObjectClass(obj);
	jfieldID keyFieldId = env->GetFieldID(objClass, "nativeWindowKey", "J");
	if (keyFieldId)
	{
		jlong keyValue = env->GetLongField(obj, keyFieldId);
		return WindowImplWithPlatformHandle((void*)keyValue);
	}
	return NULL;
}

void RequestAppOperationProcess(void)
{
	auto op = []()
	{
		JNIEnv* env = JNIOperationQueueGetEnv();
		DKCriticalSection<DKCondition> guard(windowCond);
		if (windowManager)
		{
			jclass wmClass = env->GetObjectClass(windowManager);
			jmethodID mid = env->GetMethodID(wmClass, "processAppOperationsOnUiThread", "()V");
			env->CallVoidMethod(windowManager, mid);
		}
	};
	JNIOperationQueuePost(DKFunction(op)->Invocation());
}


extern "C" {
	JNIEXPORT jboolean Java_com_icondb_IGActivity_bindNativeWindowManager(JNIEnv* env, jobject obj)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);

		DKCriticalSection<DKCondition> guard(windowCond);
		if (windowManager)
		{
			ANDROID_LOG_E("WindowManager already exist.");
			return JNI_FALSE;
		}

		jclass wmClass = env->FindClass("com/icondb/DKActivity");
		if (wmClass)
		{
			if (env->IsInstanceOf(obj, wmClass))
			{
				windowManager = env->NewGlobalRef(obj);
				return JNI_TRUE;
			}
			else
				ANDROID_LOG_E("Object is not instance of com.icondb.DKActivity");
		}
		else
			ANDROID_LOG_E("FindClass(com.icondb.DKActivity) failed.");

		return JNI_FALSE;
	}
	JNIEXPORT void Java_com_icondb_IGActivity_unbindNativeWindowManager(JNIEnv* env, jobject obj)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);

		DKCriticalSection<DKCondition> guard(windowCond);
		// 남은 윈도우는 0 이어야 한다.
		size_t numWindows = windowMap.Count();
		if (numWindows > 0)
			ANDROID_LOG_E("WindowManager: DKWindow remains: %d.", numWindows);

		if (windowManager)
			env->DeleteGlobalRef(windowManager);
		windowManager = NULL;

		ANDROID_LOG_I("WindowManager released.");
	}
	JNIEXPORT void Java_com_icondb_IGActivity_closeNativeWindows(JNIEnv* env, jobject obj)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);

		windowCond.Lock();
		DKArray<DKObject<DKOperation>> destructors;
		destructors.Reserve(windowMap.Count());

		// 현재 쓰레드(메인 쓰레드)에서 종료 호출함.
		windowMap.EnumerateForward([&](JNIWindowMap::Pair& pair) {
			DKObject<DKOperation> op = pair.value->DestroyOperation(env);
			if (op)
				destructors.Add(op);
		});
		windowCond.Unlock();

		for (DKOperation* op : destructors)
		op->Perform();

		DKCriticalSection<DKCondition> guard(windowCond);
		ANDROID_LOG_I("WindowManager: DKWindow remains: %d.", windowMap.Count());
		windowMap.Clear();
	}
	JNIEXPORT jboolean Java_com_icondb_IGSurfaceView_nativeWindowSurfaceCreated(JNIEnv* env, jobject obj, jobject surface)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKWindowImpl* win = WindowImplByJObjectLocalRef(env, obj);
		if (win)
		{
			DKCriticalSection<DKSpinLock> guard(win->nativeWindowLock);
			DKASSERT(win->nativeWindow == NULL);
			win->nativeWindow = ANativeWindow_fromSurface(env, surface);
			win->nativeWindowSize = DKSize(ANativeWindow_getWidth(win->nativeWindow), ANativeWindow_getHeight(win->nativeWindow));
			win->nativeWindowPos = DKPoint(0, 0);

			DKSize windowSize = win->nativeWindowSize;
			DKPoint windowPos = win->nativeWindowPos;

			if (win->objectRef)	// not destroyed yet
			{
				win->window->PostWindowEvent(DKWindow::EventWindowActivated, windowSize, windowPos, false);
				win->window->PostWindowEvent(DKWindow::EventWindowResized, windowSize, windowPos, false);
				win->window->PostWindowEvent(DKWindow::EventWindowUpdate, windowSize, windowPos, false);
			}

			return JNI_TRUE;
		}
		return JNI_FALSE;
	}
	JNIEXPORT jboolean Java_com_icondb_IGSurfaceView_nativeWindowSurfaceChanged(JNIEnv* env, jobject obj, jint width, jint height)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKWindowImpl* win = WindowImplByJObjectLocalRef(env, obj);
		if (win)
		{
			DKCriticalSection<DKSpinLock> guard(win->nativeWindowLock);
			DKASSERT(win->nativeWindow != NULL);
			win->nativeWindowSize = DKSize(ANativeWindow_getWidth(win->nativeWindow), ANativeWindow_getHeight(win->nativeWindow));
			win->nativeWindowPos = DKPoint(0, 0);

			DKSize windowSize = win->nativeWindowSize;
			DKPoint windowPos = win->nativeWindowPos;

			if (win->objectRef)	// not destroyed yet
			{
				win->window->PostWindowEvent(DKWindow::EventWindowResized, windowSize, windowPos, false);
				win->window->PostWindowEvent(DKWindow::EventWindowUpdate, windowSize, windowPos, false);
			}
			return JNI_TRUE;
		}
		return JNI_FALSE;
	}
	JNIEXPORT jboolean Java_com_icondb_IGSurfaceView_nativeWindowSurfaceDestroyed(JNIEnv* env, jobject obj)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKWindowImpl* win = WindowImplByJObjectLocalRef(env, obj);
		if (win)
		{
			DKCriticalSection<DKSpinLock> guard(win->nativeWindowLock);
			DKSize windowSize = win->nativeWindowSize;
			DKPoint windowPos = win->nativeWindowPos;
			ANativeWindow* nw = win->nativeWindow;
			win->nativeWindow = NULL;

			if (win->objectRef)	// not destroyed yet
			{
				win->window->PostWindowEvent(DKWindow::EventWindowHidden, windowSize, windowPos, true);
			}

			if (nw)
				ANativeWindow_release(nw);

			return JNI_TRUE;
		}
		return JNI_FALSE;
	}
	JNIEXPORT void Java_com_icondb_IGSurfaceView_nativeWindowUpdate(JNIEnv* env, jobject obj)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKWindowImpl* win = WindowImplByJObjectLocalRef(env, obj);
		if (win)
		{
			DKCriticalSection<DKSpinLock> guard(win->nativeWindowLock);
			if (win->nativeWindow)
			{
				win->nativeWindowSize = DKSize(ANativeWindow_getWidth(win->nativeWindow), ANativeWindow_getHeight(win->nativeWindow));
				win->nativeWindowPos = DKPoint(0, 0);

				DKSize windowSize = win->nativeWindowSize;
				DKPoint windowPos = win->nativeWindowPos;

				if (win->objectRef)	// not destroyed yet
				{
					win->window->PostWindowEvent(DKWindow::EventWindowUpdate, windowSize, windowPos, false);
				}
			}
		}
	}
	JNIEXPORT void Java_com_icondb_IGSurfaceView_nativeWindowVisibilityChanged(JNIEnv* env, jobject obj, jboolean visible)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKWindowImpl* win = WindowImplByJObjectLocalRef(env, obj);
		if (win)
		{
			DKCriticalSection<DKSpinLock> guard(win->nativeWindowLock);
			ANativeWindow* nw = win->nativeWindow;
			DKSize windowSize = win->nativeWindowSize;
			DKPoint windowPos = win->nativeWindowPos;

			if (win->objectRef)	// not destroyed yet
			{
				if (visible)
					win->window->PostWindowEvent(DKWindow::EventWindowShown, windowSize, windowPos, false);
				else
					win->window->PostWindowEvent(DKWindow::EventWindowHidden, windowSize, windowPos, true);
			}
		}
	}
	JNIEXPORT void Java_com_icondb_IGSurfaceView_nativeWindowTouchEvent(JNIEnv* env, jobject obj, jint id, jint act, jfloat x, jfloat y)
	{
		//ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKWindowImpl* win = WindowImplByJObjectLocalRef(env, obj);
		if (win && win->objectRef)
		{
			enum Action : int { ActionDown = 0, ActionMove = 1, ActionUp = 2 };
			DKPoint pos(x, y);
			DKVector2 delta(0, 0);

			auto p = win->mousePositionMap.Find(id);
			if (p)
				delta = DKVector2(x - p->value.x, y - p->value.y );

			switch (act)
			{
				case ActionDown:
				case ActionMove:
					if (p)
					{
						if (fabs(delta.x) > 0.0001 || fabs(delta.y) > 0.0001)
							win->window->PostMouseEvent(DKWindow::EventMouseMove, id, 0, pos, delta, false);
					}
					else
					{
						win->window->PostMouseEvent(DKWindow::EventMouseDown, id, 0, pos, delta, false);
					}
					win->mousePositionMap.Value(id) = pos;
					break;
				case ActionUp:
					if (p)
					{
						win->window->PostMouseEvent(DKWindow::EventMouseUp, id, 0, pos, delta, false);
						win->mousePositionMap.Remove(id);
					}
					break;
				default:
					DKLog("Unknown mouse action!\n");
					break;
			}
		}
	}
}

DKWindowInterface* DKWindowInterface::CreateInterface(DKWindow* window)
{
	return new DKWindowImpl(window);
}

DKWindowImpl::DKWindowImpl(DKWindow* w)
: window(w)
, objectRef(NULL)
, nativeWindow(NULL)
{
}

DKWindowImpl::~DKWindowImpl(void)
{
}

bool DKWindowImpl::CreateProxy(void* systemHandle)
{
	return false;
}

void DKWindowImpl::UpdateProxy(void)
{
}

bool DKWindowImpl::IsProxy(void) const
{
	return false;
}

bool DKWindowImpl::Create(const DKString& title, const DKSize& size, const DKPoint& origin, int style)
{
	ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
	if (this->objectRef)
	{
		ANDROID_LOG_E("Window created already!");
		return false;
	}

	DKCriticalSection<DKSpinLock> guard(this->nativeWindowLock);

	auto createWindow = [](int x, int y, int w, int h, jboolean r, DKWindowImpl* ptr)
	{
		JNIEnv* env = JNIOperationQueueGetEnv();
		DKCriticalSection<DKCondition> guard(windowCond);
		if (env && windowManager)
		{
			jclass wmClass = env->GetObjectClass(windowManager);
			jmethodID midCreate = env->GetMethodID(wmClass, "createSurfaceView", "(IIIIZ)Lcom/icondb/DKSurfaceView;");
			if (midCreate)
			{
				jobject obj = env->CallObjectMethod(windowManager, midCreate, x, y, w, h, r);
				ANDROID_LOG_D("JNI createSurfaceView returns: %p", obj);
				if (obj)
				{
					jobject objectRef = env->NewGlobalRef(obj);
					jclass viewClass = env->GetObjectClass(objectRef);
					jfieldID keyFieldId = env->GetFieldID(viewClass, "nativeWindowKey", "J");
					if (keyFieldId)
					{
						jlong keyValue = (jlong)objectRef;	// object 의 global ref 를 키로 사용함.

						if (windowMap.Insert(keyValue, ptr))
						{
							env->SetLongField(objectRef, keyFieldId, keyValue);
							ptr->objectRef = objectRef;

							windowCond.Broadcast();
							return;
						}
						env->SetLongField(objectRef, keyFieldId, (jlong)0);
					}
					else
					{
						ANDROID_LOG_E("GetFieldID(nativeWindowKey) failed.");
					}
					env->DeleteGlobalRef(objectRef);
				}
				else
				{
					ANDROID_LOG_E("createSurfaceView failed.");
				}
			}
			else
			{
				ANDROID_LOG_E("Cannot find method (createSurfaceView)");
			}
		}
	};
	int x, y, w, h;
	if (size.width < 1 || size.height < 1)
	{
		w = -1;
		h = -1;
	}
	else
	{
		w = floor(size.width + 0.5f);
		h = floor(size.height + 0.5f);
	}
	if (origin.x + w < 0 || origin.y + h < 0)
	{
		x = 0;
		y = 0;
	}
	else
	{
		x = floor(origin.x + 0.5f);
		y = floor(origin.y + 0.5f);
	}

	JNIOperationQueueProcess(DKFunction(createWindow)->Invocation(x,y,w,h, style & DKWindow::StyleResizableBorder, this));

	if (this->objectRef)
	{
		ANDROID_LOG_D("Window(0x%llx) Created.", (jlong)this->objectRef);
		nativeWindowSize = DKSize(Max<int>(w, 1), Max<int>(h, 1));
		nativeWindowPos = DKPoint(x, y);
		window->PostWindowEvent(DKWindow::EventWindowCreated, nativeWindowSize, nativeWindowPos, false);
		return true;
	}
	ANDROID_LOG_E("Window Creation Failed");
	return false;
}

void DKWindowImpl::Destroy(void)
{
	DKObject<DKOperation> op = this->DestroyOperation(NULL);

	if (op)
	{
		JNIOperationQueuePost(op);
	}

	DKCriticalSection<DKSpinLock> guard(this->nativeWindowLock);
	this->objectRef = NULL;
}

DKObject<DKOperation> DKWindowImpl::DestroyOperation(JNIEnv* env)
{
	DKCriticalSection<DKSpinLock> guard(this->nativeWindowLock);
	if (this->objectRef)
	{
		window->PostWindowEvent(DKWindow::EventWindowClosed, nativeWindowSize, nativeWindowPos, false);

		auto destroyWindow = [](JNIEnv* env, jobject ref)
		{
			if (env == NULL)
				env = JNIOperationQueueGetEnv();
			if (env)
			{
				DKCriticalSection<DKCondition> guard(windowCond);

				jclass wmClass = env->GetObjectClass(windowManager);
				jmethodID closeSurfaceView = env->GetMethodID(wmClass, "closeSurfaceView", "(Lcom/icondb/DKSurfaceView;)V");
				jclass objClass = env->GetObjectClass(ref);
				env->CallVoidMethod(windowManager, closeSurfaceView, ref);
				env->DeleteGlobalRef(ref);

				auto p = windowMap.Find((jlong)ref);
				if (p)
					ANDROID_LOG_I("Window(0x%llx) destroyed.", (jlong)ref);

				windowMap.Remove((jlong)ref);
				windowCond.Broadcast();
			}
			else
			{
				ANDROID_LOG_E("DestroyOperation failed. (No JNIEnv)");
			}
		};
		DKObject<DKOperation> op = DKFunction(destroyWindow)->Invocation(env, this->objectRef).SafeCast<DKOperation>();
		this->objectRef = NULL;
		return op;
	}
	return NULL;
}

void DKWindowImpl::Show(void)
{
	DKCriticalSection<DKSpinLock> guard(this->nativeWindowLock);

	if (this->objectRef)
	{
		auto showWindow = [](jobject ref)
		{
			JNIEnv* env = JNIOperationQueueGetEnv();
			if (env)
			{
				jclass objClass = env->GetObjectClass(ref);
				jmethodID mid = env->GetMethodID(objClass, "show", "()V");
				env->CallVoidMethod(ref, mid, ref);
			}
		};
		JNIOperationQueuePost(DKFunction(showWindow)->Invocation(objectRef));
	}
}

void DKWindowImpl::Hide(void)
{
	DKCriticalSection<DKSpinLock> guard(this->nativeWindowLock);

	if (this->objectRef)
	{
		auto hideWindow = [](jobject ref)
		{
			JNIEnv* env = JNIOperationQueueGetEnv();
			if (env)
			{
				jclass objClass = env->GetObjectClass(ref);
				jmethodID mid = env->GetMethodID(objClass, "hide", "()V");
				env->CallVoidMethod(ref, mid, ref);
			}
		};
		JNIOperationQueuePost(DKFunction(hideWindow)->Invocation(objectRef));
	}
}

void DKWindowImpl::Activate(void)
{
	DKCriticalSection<DKSpinLock> guard(this->nativeWindowLock);

	if (this->objectRef)
	{
		auto activateWindow = [](jobject ref)
		{
			JNIEnv* env = JNIOperationQueueGetEnv();
			if (env)
			{
				jclass objClass = env->GetObjectClass(ref);
				jmethodID mid = env->GetMethodID(objClass, "activate", "()V");
				env->CallVoidMethod(ref, mid, ref);
			}
		};
		JNIOperationQueuePost(DKFunction(activateWindow)->Invocation(objectRef));
	}
}

void DKWindowImpl::Minimize(void)
{
	DKCriticalSection<DKSpinLock> guard(this->nativeWindowLock);

	if (this->objectRef)
	{
		auto minimizeWindow = [](jobject ref)
		{
			JNIEnv* env = JNIOperationQueueGetEnv();
			if (env)
			{
				jclass objClass = env->GetObjectClass(ref);
				jmethodID mid = env->GetMethodID(objClass, "minimize", "()V");
				env->CallVoidMethod(ref, mid, ref);
			}
		};
		JNIOperationQueuePost(DKFunction(minimizeWindow)->Invocation(objectRef));
	}
}

void DKWindowImpl::Resize(const DKSize& s, const DKPoint* pt)
{
	DKCriticalSection<DKSpinLock> guard(this->nativeWindowLock);

	if (this->objectRef)
	{
		jint w = floor(s.width + 0.5f);
		jint h = floor(s.height + 0.5f);
		if (pt)
		{
			auto resizeWindow = [](jobject ref, jint x, jint y, jint w, jint h)
			{
				JNIEnv* env = JNIOperationQueueGetEnv();
				if (env)
				{
					jclass objClass = env->GetObjectClass(ref);
					jmethodID mid = env->GetMethodID(objClass, "resize", "(IIII)V");
					env->CallVoidMethod(ref, mid, ref, x, y, w, h);
				}
			};
			jint x = floor(pt->x + 0.5f);
			jint y = floor(pt->y + 0.5f);
			JNIOperationQueuePost(DKFunction(resizeWindow)->Invocation(objectRef, x, y, w, h));
		}
		else
		{
			auto resizeWindow = [](jobject ref, int w, int h)
			{
				JNIEnv* env = JNIOperationQueueGetEnv();
				if (env)
				{
					jclass objClass = env->GetObjectClass(ref);
					jmethodID mid = env->GetMethodID(objClass, "resize", "(II)V");
					env->CallVoidMethod(ref, mid, ref, w, h);
				}
			};
			JNIOperationQueuePost(DKFunction(resizeWindow)->Invocation(objectRef, w, h));
		}
	}
}

void DKWindowImpl::SetOrigin(const DKPoint& pt)
{
	DKCriticalSection<DKSpinLock> guard(this->nativeWindowLock);

	if (this->objectRef)
	{
		auto setOrigin = [](jobject ref, jint x, jint y)
		{
			JNIEnv* env = JNIOperationQueueGetEnv();
			if (env)
			{
				jclass objClass = env->GetObjectClass(ref);
				jmethodID mid = env->GetMethodID(objClass, "setOrigin", "(II)V");
				env->CallVoidMethod(ref, mid, ref, x, y);
			}
		};
		jint x = floor(pt.x + 0.5f);
		jint y = floor(pt.y + 0.5f);
		JNIOperationQueuePost(DKFunction(setOrigin)->Invocation(objectRef, x, y));
	}
}

void DKWindowImpl::SetTitle(const DKString& title)
{
	DKCriticalSection<DKSpinLock> guard(this->nativeWindowLock);

	if (this->objectRef)
	{
		auto setWindowTitle = [](jobject ref, DKStringU8 title)
		{
			JNIEnv* env = JNIOperationQueueGetEnv();
			if (env)
			{
				jstring str = env->NewStringUTF((const char*)title);
				jclass objClass = env->GetObjectClass(ref);
				jmethodID mid = env->GetMethodID(objClass, "setTitle", "(Ljava/lang/String;)V");
				env->CallVoidMethod(ref, mid, ref, str);
			}
		};
		JNIOperationQueuePost(DKFunction(setWindowTitle)->Invocation(objectRef, DKStringU8(title)));
	}
}

DKString DKWindowImpl::Title(void) const
{
	DKCriticalSection<DKSpinLock> guard(this->nativeWindowLock);

	if (this->objectRef)
	{
		DKStringU8 title;
		auto getWindowTitle = [&title](jobject ref)
		{
			JNIEnv* env = JNIOperationQueueGetEnv();
			if (env)
			{
				jclass objClass = env->GetObjectClass(ref);
				jmethodID mid = env->GetMethodID(objClass, "getTitle", "()Ljava/lang/String;");
				jstring str = (jstring)env->CallObjectMethod(ref, mid, ref);

				const char* tmp = env->GetStringUTFChars(str, 0);
				title = tmp;
				env->ReleaseStringUTFChars(str, tmp);
			}
		};
		JNIOperationQueueProcess(DKFunction(getWindowTitle)->Invocation(objectRef));

		return DKString(title);
	}
	return "";
}

bool DKWindowImpl::IsVisible(void) const
{
	DKCriticalSection<DKSpinLock> guard(nativeWindowLock);
	return this->nativeWindow != NULL;
}

void* DKWindowImpl::PlatformHandle(void) const
{
	DKCriticalSection<DKSpinLock> guard(nativeWindowLock);
	return objectRef;
}

void DKWindowImpl::ShowMouse(int deviceId, bool bShow)
{
}

bool DKWindowImpl::IsMouseVisible(int deviceId) const
{
	return false;
}

void DKWindowImpl::HoldMouse(int deviceId, bool bHold)
{
}

bool DKWindowImpl::IsMouseHeld(int deviceId) const
{
	return false;
}

void DKWindowImpl::SetMousePosition(int deviceId, const DKPoint& pt)
{
}

DKPoint DKWindowImpl::MousePosition(int deviceId) const
{
	return DKPoint(0,0);
}

void DKWindowImpl::EnableTextInput(int deviceId, bool bTextInput)
{
}

bool DKWindowImpl::IsValid(void) const
{
	DKCriticalSection<DKSpinLock> guard(nativeWindowLock);
	return objectRef != NULL;
}

#endif

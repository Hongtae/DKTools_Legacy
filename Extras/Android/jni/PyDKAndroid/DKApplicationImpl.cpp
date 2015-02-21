//
//  File: DKApplicationImpl.cpp
//  Encoding: UTF-8 ☃
//  Platform: Android
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2013,2014 ICONDB.COM. All rights reserved.
//


#ifdef __ANDROID__
#include <jni.h>
#include <android/log.h>
#include <errno.h>
#include <string.h>
#include "DKApplicationImpl.h"
#include "JNIOperationQueue.h"

#define  ANDROID_LOG_TAG    "DKApplicationImpl"
#define  ANDROID_LOG_I(...)  __android_log_print(ANDROID_LOG_INFO, ANDROID_LOG_TAG, __VA_ARGS__)
#define  ANDROID_LOG_D(...)  __android_log_print(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, __VA_ARGS__)
#define  ANDROID_LOG_E(...)  __android_log_print(ANDROID_LOG_ERROR, ANDROID_LOG_TAG, __VA_ARGS__)

static DKCondition appCond;
static bool appInitialized = false;
static bool appRequestTerminate = false;
static bool appRunning = false;

static DKString appModulePath = L"";
static DKObject<DKZipUnarchiver> appResource = NULL;
static DKString appResourcePrefix = L"";
static DKMap<DKApplication::SystemPath, DKString> appEnvPathMap;

static DKRect appDisplayBounds = DKRect(0,0,0,0);
static DKRect appContentBounds = DKRect(0,0,0,0);
static bool appContentReady = false;

static int appResult = 0;

static DKCondition appOperationCond;
static DKArray<DKObject<DKOperation>> appOperations;


extern "C" {
	JNIEXPORT void Java_com_icondb_DK_processOperations(JNIEnv* env, jobject obj)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKCriticalSection<DKCondition> guard(appOperationCond);
		for (DKOperation* op : appOperations)
		{
			op->Perform();
		}
		appOperationCond.Broadcast();
	}
	JNIEXPORT jboolean Java_com_icondb_DK_setTemporaryDirectory(JNIEnv* env, jobject obj, jstring dir)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKCriticalSection<DKCondition> guard(appCond);
		while (appInitialized)
			appCond.Wait();

		const char* tmpDir = env->GetStringUTFChars(dir, 0);
		ANDROID_LOG_I("SetTemporaryDirectory: %s", tmpDir);
		int r = setenv("TEMPDIR", tmpDir, 1);
		env->ReleaseStringUTFChars(dir, tmpDir);

		if (r == 0)
			return JNI_TRUE;

		char errorStr[1024];
		strerror_r(errno, errorStr, 1024);
		ANDROID_LOG_E("setenv failed (%s)", errorStr);
		return JNI_FALSE;
	}
	JNIEXPORT void Java_com_icondb_DK_setEnvironmentPath(JNIEnv* env, jobject obj, jstring k, jstring v)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKCriticalSection<DKCondition> guard(appCond);
		while (appInitialized)
			appCond.Wait();

		const char* key = env->GetStringUTFChars(k, 0);
		const char* value = env->GetStringUTFChars(v, 0);

		ANDROID_LOG_I("SetEnv[%s]: %s", key, value);

		if (strcmp(key, "SystemRoot") == 0)
			appEnvPathMap.Update(DKApplication::SystemPathSystemRoot, value);
		else if (strcmp(key, "AppRoot") == 0)
			appEnvPathMap.Update(DKApplication::SystemPathAppRoot, value);
		else if (strcmp(key, "AppResource") == 0)
			appEnvPathMap.Update(DKApplication::SystemPathAppResource, value);
		else if (strcmp(key, "AppExecutable") == 0)
			appEnvPathMap.Update(DKApplication::SystemPathAppExecutable, value);
		else if (strcmp(key, "AppData") == 0)
			appEnvPathMap.Update(DKApplication::SystemPathAppData, value);
		else if (strcmp(key, "UserHome") == 0)
			appEnvPathMap.Update(DKApplication::SystemPathUserHome, value);
		else if (strcmp(key, "UserDocuments") == 0)
			appEnvPathMap.Update(DKApplication::SystemPathUserDocuments, value);
		else if (strcmp(key, "UserPreferences") == 0)
			appEnvPathMap.Update(DKApplication::SystemPathUserPreferences, value);
		else if (strcmp(key, "UserCache") == 0)
			appEnvPathMap.Update(DKApplication::SystemPathUserCache, value);
		else if (strcmp(key, "UserTemp") == 0)
			appEnvPathMap.Update(DKApplication::SystemPathUserTemp, value);
		else
			ANDROID_LOG_E("Error: Unknown key!");

		env->ReleaseStringUTFChars(k, key);
		env->ReleaseStringUTFChars(v, value);
	}
	JNIEXPORT void Java_com_icondb_DK_setModulePath(JNIEnv* env, jobject obj, jstring val)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKCriticalSection<DKCondition> guard(appCond);
		while (appInitialized)
			appCond.Wait();

		const char* str = env->GetStringUTFChars(val, 0);
		ANDROID_LOG_I("SetModulePath: %s", str);
		appModulePath = str;
		env->ReleaseStringUTFChars(val, str);
	}
	JNIEXPORT jboolean Java_com_icondb_DK_setResourcePath(JNIEnv* env, jobject obj, jstring path, jstring prefix)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKCriticalSection<DKCondition> guard(appCond);
		while (appInitialized)
			appCond.Wait();

		const char* resPath = env->GetStringUTFChars(path, 0);
		const char* resPrefix = env->GetStringUTFChars(prefix, 0);

		ANDROID_LOG_I("SetResourcePath: %s (prefix: %s)", resPath, resPrefix);

		appResource = DKZipUnarchiver::Create(resPath);
		appResourcePrefix = resPrefix;

		env->ReleaseStringUTFChars(path, resPath);
		env->ReleaseStringUTFChars(prefix, resPrefix);

		if (appResource)
			return JNI_TRUE;
		return JNI_FALSE;
	}
	JNIEXPORT void Java_com_icondb_DK_setDisplayResolution(JNIEnv* env, jobject obj, jint x, jint y, jint w, jint h)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKCriticalSection<DKCondition> guard(appCond);
		ANDROID_LOG_I("SetDisplayResolution: (%d, %d) (%d x %d)", x, y, w, h);
		appDisplayBounds.origin.x = x;
		appDisplayBounds.origin.y = y;
		appDisplayBounds.size.width = w;
		appDisplayBounds.size.height = h;
	}
	JNIEXPORT void Java_com_icondb_DK_setContentResolution(JNIEnv* env, jobject obj, jint x, jint y, jint w, jint h)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKCriticalSection<DKCondition> guard(appCond);
		ANDROID_LOG_I("SetContentResolution: (%d, %d) (%d x %d)", x, y, w, h);
		appContentBounds.origin.x = x;
		appContentBounds.origin.y = y;
		appContentBounds.size.width = w;
		appContentBounds.size.height = h;
		appContentReady = true;
		appCond.Signal();
	}
	JNIEXPORT jboolean Java_com_icondb_DK_initApp(JNIEnv* env, jobject obj)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKCriticalSection<DKCondition> guard(appCond);
		while (appInitialized)
			appCond.Wait();

		JavaVM* jvm = NULL;
		env->GetJavaVM(&jvm);
		appInitialized = JNIOperationQueueCreate(jvm, 2);
		if (appInitialized)
		{
			appCond.Broadcast();
			return JNI_TRUE;
		}
		return JNI_FALSE;
	}
	JNIEXPORT void Java_com_icondb_DK_releaseApp(JNIEnv* env, jobject obj)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);
		DKCriticalSection<DKCondition> guard(appCond);
		if (appInitialized)
		{
			if (appRunning)
			{
				if (!appRequestTerminate)		// 종료 요청 보냄.
				{
					appResult = 0;
					appRequestTerminate = true;
					appCond.Broadcast();
				}

				while (appRunning)		// 종료 기다림.
					appCond.Wait();
			}
			appInitialized = false;

			appModulePath = L"";
			appResource = NULL;
			appResourcePrefix = L"";
			appEnvPathMap.Clear();
			appDisplayBounds = DKRect(0,0,0,0);
			appContentBounds = DKRect(0,0,0,0);
			appContentReady = false;
			appCond.Broadcast();
		}
		JNIOperationQueueDestroy();
	}
}


DKApplicationInterface* DKApplicationInterface::CreateInterface(DKApplication* app)
{
	return new DKApplicationImpl(app);
}

DKApplicationImpl::DKApplicationImpl(DKApplication* app)
: mainApp(app)
{
}

DKApplicationImpl::~DKApplicationImpl(void)
{
}

int DKApplicationImpl::Run(DKFoundation::DKArray<char*>& args)
{
	appCond.Lock();

	while (appRunning)
		appCond.Wait();

	while (appInitialized == false)
		appCond.Wait();

	appResult = 0;
	appRunning = true;

	appRequestTerminate = false;
	appCond.Unlock();

	AppInitialize(mainApp);

	appCond.Lock();
	while (appRequestTerminate == false)
		appCond.Wait();
	int ret = appResult;
	appCond.Unlock();

	AppDeinitialize(mainApp);

	appCond.Lock();
	appRunning = false;
	appCond.Broadcast();
	appCond.Unlock();

	return ret;
}

void DKApplicationImpl::Terminate(int exitCode)
{
	DKCriticalSection<DKCondition> guard(appCond);
	if (appRunning)
	{
		appResult = exitCode;
		appRequestTerminate = true;
		appCond.Signal();
	}
}

void RequestAppOperationProcess(void);		// defined in DKWindowImpl.cpp
void DKApplicationImpl::PerformOperationOnMainThread(DKOperation* op, bool waitUntilDone)
{
	if (op)
	{
		appOperationCond.Lock();
		appOperations.Add(op);
		// notify!
		RequestAppOperationProcess();
		appOperationCond.Unlock();

		if (waitUntilDone)
		{
			DKCriticalSection<DKCondition> guard(appOperationCond);
			bool found = false;
			do {
				for (DKOperation* op2 : appOperations)
				{
					if (op2 == op)
					{
						found = true;
						break;
					}
				}
				if (found)
					appOperationCond.Wait();
			} while(found);
		}
	}
}

DKLogger& DKApplicationImpl::DefaultLogger(void)
{
	struct Logger : public DKLogger
	{
		void Log(const DKString& msg)
		{
			// 안드로이드에서 wchar_t (%ls, %S) 를 찍을수 없기 때문에 utf8 로 변경함.
			DKStringU8 msgU8(msg);
			__android_log_print(ANDROID_LOG_INFO, "DKLog", "%s", (const char*)msgU8);
		}
	};
	static Logger logger;
	return logger;
}

DKString DKApplicationImpl::EnvironmentPath(SystemPath sp)
{
	auto p = appEnvPathMap.Find(sp);
	if (p)
		return p->value;
	return L"/";
}

DKString DKApplicationImpl::ModulePath(void)
{
	return appModulePath;
}

DKObject<DKData> DKApplicationImpl::LoadResource(const DKString& res, DKAllocator& alloc)
{
	if (appResource)
	{
		DKString resPath = appResourcePrefix + res;
		DKObject<DKStream> stream = appResource->OpenFileStream(resPath);
		if (stream)
		{
			return DKBuffer::Create(stream).SafeCast<DKData>();
		}
	}
	return NULL;
}

DKObject<DKData> DKApplicationImpl::LoadStaticResource(const DKString& res)
{
	return LoadResource(res, DKAllocator::DefaultAllocator());
}

DKRect DKApplicationImpl::DisplayBounds(int displayId) const
{
	DKCriticalSection<DKCondition> guard(appCond);
	return appDisplayBounds;
}

DKRect DKApplicationImpl::ScreenContentBounds(int displayId) const
{
	DKCriticalSection<DKCondition> guard(appCond);

	while (appContentReady == false)
	{
		DKLog("Waiting for Content-Bounds-Info message from main window");
		appCond.Wait();
	}

	return appContentBounds;
}

DKString DKApplicationImpl::HostName(void) const
{
	return "";
}

DKString DKApplicationImpl::OSName(void) const
{
	return "";
}

DKString DKApplicationImpl::UserName(void) const
{
	return "";
}

#endif

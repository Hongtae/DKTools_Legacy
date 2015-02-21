#include <unistd.h>
#include <jni.h>
#include <android/log.h>

#include <dk.h>
#include <PyDKPython.h>
#include <PyDKInterpreter.h>
#include "JNIOperationQueue.h"

#define  ANDROID_LOG_TAG    "PyDKAndroid"
#define  ANDROID_LOG_I(...)  __android_log_print(ANDROID_LOG_INFO, ANDROID_LOG_TAG, __VA_ARGS__)
#define  ANDROID_LOG_D(...)  __android_log_print(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, __VA_ARGS__)
#define  ANDROID_LOG_E(...)  __android_log_print(ANDROID_LOG_ERROR, ANDROID_LOG_TAG, __VA_ARGS__)

using namespace DKFoundation;
using namespace DKFramework;

DKObject<PyDK> python = NULL;

struct JNIPyInterpreter
{
	DKObject<PyDKInterpreter> interpreter;
	jweak ref;
};
typedef DKMap<jlong, JNIPyInterpreter> JInterpreterMap;
static JInterpreterMap interpreterMap;
static DKSpinLock interpreterMapLock;

#define NATIVE_INTERPRETER_KEY	"nativeInterpreterKey"

extern "C"
{
	JNIEXPORT jboolean Java_com_icondb_DK_isPythonInitialized(JNIEnv* env, jobject obj)
	{
		if (python)
			return JNI_TRUE;
		return JNI_FALSE;
	}
	JNIEXPORT jboolean Java_com_icondb_DK_initPython(JNIEnv* env, jobject obj, jstring name, jobjectArray pathArray)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);

		static bool installStderrToLogcat = true;
		if (installStderrToLogcat)
		{
			installStderrToLogcat = false;
			auto r = []()
			{
				ANDROID_LOG_D("ForwardStderr Running");
				int pipes[2];
				pipe(pipes);
				dup2(pipes[1], STDERR_FILENO);
				close(pipes[1]);
				FILE *inputFile = fdopen(pipes[0], "r");
				char readBuffer[1024];
				while (1) {
					char* s = fgets(readBuffer, sizeof(readBuffer), inputFile);
					if (s)
						__android_log_write(ANDROID_LOG_ERROR, "stderr_fw", s);
					else
						break;
				}
				ANDROID_LOG_D("ForwardStderr Terminated");
			};
			DKObject<DKOperation> op = DKFunction(r)->Invocation().SafeCast<DKOperation>();
			DKThread::Create(op);
		}

		if (python)
		{
			ANDROID_LOG_E("Python initialized already.");
			return JNI_FALSE;
		}

		DKString::StringArray envPaths;
		int numPathCount = env->GetArrayLength(pathArray);
		for (int i = 0; i < numPathCount; ++i)
		{
			jstring str = (jstring)env->GetObjectArrayElement(pathArray, i);
			const char* p = env->GetStringUTFChars(str, 0);

			envPaths.Add(p);
			env->ReleaseStringUTFChars(str, p);
		}
		const char* name2 = env->GetStringUTFChars(name, 0);
		DKString appName = name2;
		env->ReleaseStringUTFChars(name, name2);

		ANDROID_LOG_I("Initialize PyDK (name:%s)", (const char*)DKStringU8(appName));
		for (int i = 0; i < envPaths.Count(); ++i)
		{
			ANDROID_LOG_I("Python path[%d]: %s", i, (const char*)DKStringU8(envPaths.Value(i)));
		}

		python = PyDK::Create(envPaths);
		ANDROID_LOG_I("PyDK Initialized.");

		return JNI_TRUE;
	}
	JNIEXPORT void Java_com_icondb_DK_releasePython(JNIEnv* env, jobject obj)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);

		DKCriticalSection<DKSpinLock> guard(interpreterMapLock);

		interpreterMap.EnumerateForward([&](JInterpreterMap::Pair& pair) {
			jweak obj = pair.value.ref;
			jclass objClass = env->GetObjectClass(obj);
			jfieldID fieldId = env->GetFieldID(objClass, NATIVE_INTERPRETER_KEY, "J");
			if (fieldId)
				env->SetLongField(obj, fieldId, (jlong)0);
			env->DeleteWeakGlobalRef(obj);
			ANDROID_LOG_I("PyInterpreter for key:%llu removed.", pair.key);
		});
		interpreterMap.Clear();
		python = NULL;
	}

	JNIEXPORT jboolean Java_com_icondb_IGPyInterpreter_bindNative(JNIEnv* env, jobject obj)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);

		if (python == NULL)
		{
			ANDROID_LOG_E("Python not initialized.");
			return JNI_FALSE;
		}

		jweak objectRef = env->NewWeakGlobalRef(obj);	// key 로 사용함.
		DKObject<PyDKInterpreter> interp = NULL;

		interpreterMapLock.Lock();
		// 맨 처음 바인딩하는 객체는 Python 객체로 바인딩 한다. 그 후에는 NewInterpreter 를 호출하여 새로 생성함.
		if (interpreterMap.IsEmpty())
		{
			interp = python;
			ANDROID_LOG_I("Binding main interpreter.");
		}
		else
		{
			interp = python->NewInterpreter();
			ANDROID_LOG_I("Binding sub interpreter.");
		}
		interpreterMapLock.Unlock();

		jclass objClass = env->GetObjectClass(objectRef);
		jfieldID keyFieldId = env->GetFieldID(objClass, NATIVE_INTERPRETER_KEY, "J");
		if (keyFieldId == NULL)
		{
			ANDROID_LOG_E("Interpreter registeration failed: field nativeInterpreterKey not foud");
			return JNI_FALSE;
		}
		jlong keyFieldValue = env->GetLongField(objectRef, keyFieldId);
		keyFieldValue = (jlong)objectRef;		// weak-reference 를 key 로 사용함.

		JNIPyInterpreter pyInterp;
		pyInterp.interpreter = interp;
		pyInterp.ref = objectRef;

		DKCriticalSection<DKSpinLock> guard(interpreterMapLock);
		if (interpreterMap.Insert(keyFieldValue, pyInterp))
		{
			struct JNIObjectCallback
			{
				jweak objectRef;
				jmethodID methodId;

				void Print(const DKString& s)
				{
					auto fn = [this](DKStringU8 msg)
					{
						JNIEnv* e = JNIOperationQueueGetEnv();
						if (e)
						{
							jstring message = e->NewStringUTF((const char*)msg);
							e->CallVoidMethod(objectRef, methodId, message);
						}
					};
					JNIOperationQueuePost(DKFunction(fn)->Invocation(DKStringU8(s)));
				}
				bool Input(const DKString& pr, DKString& inp)
				{
					auto fn = [this](const DKStringU8& pr, DKStringU8& inp, bool& result)
					{
						JNIEnv* e = JNIOperationQueueGetEnv();
						if (e)
						{
							jstring prompt = e->NewStringUTF((const char*)pr);
							jstring input = (jstring)e->CallObjectMethod(objectRef, methodId, prompt);

							const char* inputStr = e->GetStringUTFChars(input, NULL);
							if (inputStr && inputStr[0])
							{
								result = true;
								inp = inputStr;
							}
							e->ReleaseStringUTFChars(input, inputStr);
						}
					};
					bool result = false;
					DKStringU8 input;
					if (JNIOperationQueueProcess(DKFunction(fn)->Invocation(DKStringU8(pr), input, result)))
					{
						if (result)
						{
							inp = DKString(input);
							return true;
						}
					}
					return false;
				}
			};

			JNIObjectCallback outputCb, errorCb, inputCb;
			outputCb.objectRef = objectRef;
			errorCb.objectRef = objectRef;
			inputCb.objectRef = objectRef;
			outputCb.methodId = env->GetMethodID(objClass, "printOutput", "(Ljava/lang/String;)V");
			errorCb.methodId = env->GetMethodID(objClass, "printError", "(Ljava/lang/String;)V");
			inputCb.methodId = env->GetMethodID(objClass, "requestInput", "(Ljava/lang/String;)Ljava/lang/String;");

			interp->SetOutputCallback(DKFunction(outputCb, &JNIObjectCallback::Print));
			interp->SetErrorCallback(DKFunction(errorCb, &JNIObjectCallback::Print));
			interp->SetInputCallback(DKFunction(inputCb, &JNIObjectCallback::Input));

			env->SetLongField(objectRef, keyFieldId, keyFieldValue);
			ANDROID_LOG_I("Interpreter registered with key:%llu", keyFieldValue);
			return JNI_TRUE;
		}
		env->SetLongField(objectRef, keyFieldId, (jlong)0);
		env->DeleteWeakGlobalRef(objectRef);
		ANDROID_LOG_E("Interpreter registeration failed for key:%llu", keyFieldValue);
		return JNI_FALSE;
	}
	JNIEXPORT void Java_com_icondb_IGPyInterpreter_unbindNative(JNIEnv* env, jobject obj)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);

		jclass objClass = env->GetObjectClass(obj);
		jfieldID keyFieldId = env->GetFieldID(objClass, NATIVE_INTERPRETER_KEY, "J");
		if (keyFieldId)
		{
			jlong keyFieldValue = env->GetLongField(obj, keyFieldId);
			DKCriticalSection<DKSpinLock> guard(interpreterMapLock);
			auto p = interpreterMap.Find(keyFieldValue);
			if (p)
			{
				p->value.interpreter = NULL;
				jweak ref = p->value.ref;
				env->DeleteWeakGlobalRef(ref);
				ANDROID_LOG_I("Interpreter unregistered for key:%llu", keyFieldValue);
				interpreterMap.Remove(keyFieldValue);
			}
			else
			{
				ANDROID_LOG_E("No Interpreter found for key:%llu", keyFieldValue);
			}
			env->SetLongField(obj, keyFieldId, (jlong)0);
		}
	}

	JNIEXPORT jboolean Java_com_icondb_IGPyInterpreter_runStringNative(JNIEnv* env, jobject obj, jstring str)
	{
		ANDROID_LOG_I("%s", DK_FUNCTION_NAME);

		jclass objClass = env->GetObjectClass(obj);
		jfieldID keyFieldId = env->GetFieldID(objClass, NATIVE_INTERPRETER_KEY, "J");
		if (keyFieldId == NULL)
		{
			ANDROID_LOG_E("Interpreter registeration failed: field nativeInterpreterKey not foud");
			return JNI_FALSE;
		}
		jlong keyFieldValue = env->GetLongField(obj, keyFieldId);

		DKCriticalSection<DKSpinLock> guard(interpreterMapLock);

		auto p = interpreterMap.Find(keyFieldValue);
		if (p)
		{
			const char* s = env->GetStringUTFChars(str, 0);
			bool result = p->value.interpreter->RunString(s);
			env->ReleaseStringUTFChars(str, s);
			if (result)
				return JNI_TRUE;
		}
		else
		{
			ANDROID_LOG_E("No Interpreter found for key:%llu", keyFieldValue);
		}
		return JNI_FALSE;
	}

	JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
	{
		ANDROID_LOG_I("JNI_OnLoad");
		return JNI_VERSION_1_6;
	}

	JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved)
	{
		ANDROID_LOG_I("JNI_OnUnload");
	}
}

#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include "JNIOperationQueue.h"

#define  ANDROID_LOG_TAG    "JNIOperationQueue"
#define  ANDROID_LOG_I(...)  __android_log_print(ANDROID_LOG_INFO, ANDROID_LOG_TAG, __VA_ARGS__)
#define  ANDROID_LOG_D(...)  __android_log_print(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, __VA_ARGS__)
#define  ANDROID_LOG_E(...)  __android_log_print(ANDROID_LOG_ERROR, ANDROID_LOG_TAG, __VA_ARGS__)

using namespace DKFoundation;
using namespace DKFramework;

static JavaVM* javaVM = NULL;
static DKOperationQueue* queue = NULL;
static DKCondition operationCond;
static DKSpinLock queueLock;

struct JNIQueueFilter : public DKOperationQueue::ThreadFilter
{
	void OnThreadInitialized(void) override
	{
		JNIEnv* env = NULL;
		javaVM->AttachCurrentThread(&env, NULL);
		//javaVM->AttachCurrentThreadAsDaemon(&env, NULL);
		ANDROID_LOG_D("javaVM->AttachCurrentThread (thread:%lx)", (long)pthread_self());
	}
	void OnThreadTerminate(void) override
	{
		javaVM->DetachCurrentThread();
		ANDROID_LOG_D("javaVM->DetachCurrentThread (thread:%lx)", (long)pthread_self());
	}
} filter;

bool JNIOperationQueueCreate(JavaVM* jvm, size_t numThreads)
{
	DKCriticalSection<DKSpinLock> guard(queueLock);
	if (javaVM || queue)
		return false;

	javaVM = jvm;
	queue = new DKOperationQueue(&filter);
	queue->SetMaxConcurrentOperations(numThreads);
	ANDROID_LOG_D("JNIOperationQueue created.");
	return true;
}

void JNIOperationQueueDestroy(void)
{
	DKCriticalSection<DKSpinLock> guard(queueLock);
	if (queue)
	{
		delete queue;
	}

	queue = NULL;
	javaVM = NULL;
	ANDROID_LOG_D("JNIOperationQueue destroyed.");
}

bool JNIOperationQueuePost(DKOperation* operation)
{
	DKCriticalSection<DKSpinLock> guard(queueLock);
	if (queue)
	{
		queue->Post(operation);
		return true;
	}
	return false;
}

bool JNIOperationQueueProcess(DKOperation* operation)
{
	DKObject<DKOperationQueue::OperationSync> sync = NULL;
	queueLock.Lock();
	if (queue)
	{
		sync = queue->ProcessAsync(operation);
	}
	queueLock.Unlock();

	if (sync)
		return sync->Sync();
	return false;
}

JNIEnv* JNIOperationQueueGetEnv(void)
{
	DKCriticalSection<DKSpinLock> guard(queueLock);
	JNIEnv* env = NULL;
	if (javaVM)
	{
		javaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
	}
	return env;
}

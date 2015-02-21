#include <jni.h>
#include <dk.h>


bool JNIOperationQueueCreate(JavaVM* jvm, size_t numThreads);
void JNIOperationQueueDestroy(void);
bool JNIOperationQueuePost(DKFoundation::DKOperation*);
bool JNIOperationQueueProcess(DKFoundation::DKOperation*);
JNIEnv* JNIOperationQueueGetEnv(void);


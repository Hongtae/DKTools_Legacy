#include "../TestCase.h"


struct OperationTest : public TestCase
{
	void TestFunc1(float s)
	{
		DKLog("[Thread:%x] TestFunc sleep for %f seconds\n", DKThread::CurrentThreadId(), s);
		DKThread::Sleep(s);
		DKLog("[Thread:%x] TestFunc done.\n", DKThread::CurrentThreadId());
	}
	void TestQueue(int maxConcurrent)
	{
		DKTimer timer;
		DKOperationQueue queue;
		queue.SetMaxConcurrentOperations(maxConcurrent);

		timer.Reset();

		queue.Post(DKFunction(this, &OperationTest::TestFunc1)->Invocation(3.0));
		queue.Post(DKFunction(this, &OperationTest::TestFunc1)->Invocation(3.0));
		queue.Post(DKFunction(this, &OperationTest::TestFunc1)->Invocation(3.0));

		DKLog("queue data:%d, thread:%d (maxConcurrent:%d)\n", queue.QueueLength(), queue.RunningOperations(), queue.MaxConcurrentOperations());

		double elapsed = timer.Reset();
		DKLog("queue add %f elapsed.\n", elapsed);
		queue.WaitForCompletion();
		elapsed = timer.Reset();
		DKLog("queue wait %f seconds\n", elapsed);
		DKLog("queue data:%d, thread:%d\n", queue.QueueLength(), queue.RunningOperations());
	}
	void RunTest()
	{
		TestQueue(255);
		TestQueue(1);
	}
} operationTest;

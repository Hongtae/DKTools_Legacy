#include "../TestCase.h"


struct RunLoopTest : public TestCase
{
	void TimerFunc(void)
	{
		static DKTimer timer;
		DKDateTime current = DKDateTime::Now();
		DKLog("[RunLoopTimer] Interval: %f, Current: %d-%02d-%02d %d:%02d:%02d.%06d\n", 
			timer.Reset(),
			current.Year(), current.Month(), current.Day(), current.Hour(), current.Minute(), current.Second(), current.Microsecond());
	}
	void TestFunc(void)
	{
		DKLog("CurrentThread:%x\n", DKThread::CurrentThreadId());
		DKLog("[RunLoopProcess] Sleeping 5 sec in thread\n");
		DKThread::Sleep(5.0);
	}
	void PostByTimeFunc(int a, DKDateTime posting)
	{
		DKLog("[RunLoopPostByTime] Order:%d, PostingDate: %d-%02d-%02d %d:%02d:%02d.%06d\n", a,
			posting.Year(), posting.Month(), posting.Day(), posting.Hour(), posting.Minute(), posting.Second(), posting.Microsecond());
	}
	void PostByTickFunc(int a, double d)
	{
		DKLog("[RunLoopPostByTick] Order:%d, Delay: %f / CurrentTick:%p\n", a, d, DKTimer::SystemTick());
	}

	void RunTest()
	{
		DKObject<DKRunLoop> runLoop = DKObject<DKRunLoop>::New();
		runLoop->Run();

		DKLog("Timer Operation add\n");

		DKObject<DKRunLoopTimer> timer = DKRunLoopTimer::Create(DKFunction(this, &RunLoopTest::TimerFunc)->Invocation(), 1, runLoop);

		DKLog("Sleeping 10 sec\n");
		DKThread::Sleep(20.0);
		DKLog("Destorying timer.\n");
		timer = NULL;

		DKLog("Testing ProcessOperation\n");
		runLoop->ProcessOperation(DKFunction(this, &RunLoopTest::TestFunc)->Invocation());
		DKLog("Testing ProcessOperation [done]\n");

		DKLog("Testing Random order\n");
		DKDateTime current = DKDateTime::Now();
		int maxOps = 20;
		DKArray<int> order;
		for (int i = 0; i < maxOps; i++)
		{
			order.Add(DKRandom() % (maxOps+1));
		}
		for (int i = 0; i < order.Count(); i++)
		{
			int value = order.Value(i);
			DKDateTime date = current + static_cast<double>(value) * 0.5f + 1.0f;	// 1 + value*0.5초후에 실행
			runLoop->PostOperation(DKFunction(this, &RunLoopTest::PostByTimeFunc)->Invocation(value, date), date); // 시간 형식 (DKDateTime)
		}
		for (int i = 0; i < order.Count(); i++)
		{
			int value = order.Value(i);
			double delay = static_cast<double>(value) * 0.5 + 1.0;				// 1 + value*0.5 초 후 실행
			runLoop->PostOperation(DKFunction(this, &RunLoopTest::PostByTickFunc)->Invocation(value, delay), delay); // 딜레이 형식
		}
		DKLog("Sleeping 10 secs\n");
		DKThread::Sleep(10.0);
		DKLog("Testing Random order [done]\n");

		DKLog("Destory runloop.\n");
		runLoop->Terminate(true);
		runLoop = NULL;
	}
} runLoopTest;

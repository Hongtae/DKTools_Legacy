#include "../TestCase.h"


void TestFunction0(void)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}
void TestFunction1(const int& a)
{
	DKLog("%s a:%d(%x)\n", DKLIB_FUNCTION_NAME, a, &a);
}
void TestFunction2(const int& a, const int& b)
{
	DKLog("%s a:%d(%x), b:%d(%x)\n", DKLIB_FUNCTION_NAME, a, &a, b, &b);
}

void SleepFunction(double s)
{
	DKLog("Sleep function sleep:%f (in thread:%x)\n", s, (unsigned int)DKThread::CurrentThreadId());
	DKThread::Sleep(s);
	DKLog("Sleep function end\n");
}

inline void InlineFunction0(void)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}
inline void InlineFunction1(const int& a)
{
	DKLog("%s a:%d(%x)\n", DKLIB_FUNCTION_NAME, a, &a);
}
inline void InlineFunction2(const int& a, const int& b)
{
	DKLog("%s a:%d(%x), b:%d(%x)\n", DKLIB_FUNCTION_NAME, a, &a, b, &b);
}

void MiddleParameterFunc(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, 
						 int a11, int a12, int a13, int a14, int a15, int a16, int a17, int a18, int a19, int a20, 
						 int a21, int a22, int a23, int a24, int a25, int a26, int a27, int a28, int a29, int a30, 
						 int a31, int a32, int a33, int a34, int a35, int a36, int a37, int a38, int a39, int a40, 
						 int a41, int a42, int a43, int a44, int a45, int a46, int a47, int a48, int a49, int a50)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	DKLog("a1 = %d\n", a1);
	DKLog("a2 = %d\n", a2);
	DKLog("a3 = %d\n", a3);
	DKLog("a4 = %d\n", a4);
	DKLog("a5 = %d\n", a5);
	DKLog("a6 = %d\n", a6);
	DKLog("a7 = %d\n", a7);
	DKLog("a8 = %d\n", a8);
	DKLog("a9 = %d\n", a9);
	DKLog("a10 = %d\n", a10);
	DKLog("a11 = %d\n", a11);
	DKLog("a12 = %d\n", a12);
	DKLog("a13 = %d\n", a13);
	DKLog("a14 = %d\n", a14);
	DKLog("a15 = %d\n", a15);
	DKLog("a16 = %d\n", a16);
	DKLog("a17 = %d\n", a17);
	DKLog("a18 = %d\n", a18);
	DKLog("a19 = %d\n", a19);
	DKLog("a20 = %d\n", a20);
	DKLog("a21 = %d\n", a21);
	DKLog("a22 = %d\n", a22);
	DKLog("a23 = %d\n", a23);
	DKLog("a24 = %d\n", a24);
	DKLog("a25 = %d\n", a25);
	DKLog("a26 = %d\n", a26);
	DKLog("a27 = %d\n", a27);
	DKLog("a28 = %d\n", a28);
	DKLog("a29 = %d\n", a29);
	DKLog("a30 = %d\n", a30);
	DKLog("a31 = %d\n", a31);
	DKLog("a32 = %d\n", a32);
	DKLog("a33 = %d\n", a33);
	DKLog("a34 = %d\n", a34);
	DKLog("a35 = %d\n", a35);
	DKLog("a36 = %d\n", a36);
	DKLog("a37 = %d\n", a37);
	DKLog("a38 = %d\n", a38);
	DKLog("a39 = %d\n", a39);
	DKLog("a40 = %d\n", a40);
	DKLog("a41 = %d\n", a41);
	DKLog("a42 = %d\n", a42);
	DKLog("a43 = %d\n", a43);
	DKLog("a44 = %d\n", a44);
	DKLog("a45 = %d\n", a45);
	DKLog("a46 = %d\n", a46);
	DKLog("a47 = %d\n", a47);
	DKLog("a48 = %d\n", a48);
	DKLog("a49 = %d\n", a49);
	DKLog("a50 = %d\n", a50);
}

void LongParameterFunc(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, 
					   int a11, int a12, int a13, int a14, int a15, int a16, int a17, int a18, int a19, int a20, 
					   int a21, int a22, int a23, int a24, int a25, int a26, int a27, int a28, int a29, int a30, 
					   int a31, int a32, int a33, int a34, int a35, int a36, int a37, int a38, int a39, int a40, 
					   int a41, int a42, int a43, int a44, int a45, int a46, int a47, int a48, int a49, int a50, 
					   int a51, int a52, int a53, int a54, int a55, int a56, int a57, int a58, int a59, int a60, 
					   int a61, int a62, int a63, int a64, int a65, int a66, int a67, int a68, int a69, int a70, 
					   int a71, int a72, int a73, int a74, int a75, int a76, int a77, int a78, int a79, int a80, 
					   int a81, int a82, int a83, int a84, int a85, int a86, int a87, int a88, int a89, int a90, 
					   int a91, int a92, int a93, int a94, int a95, int a96, int a97, int a98, int a99, int a100)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	DKLog("a1 = %d\n", a1);
	DKLog("a2 = %d\n", a2);
	DKLog("a3 = %d\n", a3);
	DKLog("a4 = %d\n", a4);
	DKLog("a5 = %d\n", a5);
	DKLog("a6 = %d\n", a6);
	DKLog("a7 = %d\n", a7);
	DKLog("a8 = %d\n", a8);
	DKLog("a9 = %d\n", a9);
	DKLog("a10 = %d\n", a10);
	DKLog("a11 = %d\n", a11);
	DKLog("a12 = %d\n", a12);
	DKLog("a13 = %d\n", a13);
	DKLog("a14 = %d\n", a14);
	DKLog("a15 = %d\n", a15);
	DKLog("a16 = %d\n", a16);
	DKLog("a17 = %d\n", a17);
	DKLog("a18 = %d\n", a18);
	DKLog("a19 = %d\n", a19);
	DKLog("a20 = %d\n", a20);
	DKLog("a21 = %d\n", a21);
	DKLog("a22 = %d\n", a22);
	DKLog("a23 = %d\n", a23);
	DKLog("a24 = %d\n", a24);
	DKLog("a25 = %d\n", a25);
	DKLog("a26 = %d\n", a26);
	DKLog("a27 = %d\n", a27);
	DKLog("a28 = %d\n", a28);
	DKLog("a29 = %d\n", a29);
	DKLog("a30 = %d\n", a30);
	DKLog("a31 = %d\n", a31);
	DKLog("a32 = %d\n", a32);
	DKLog("a33 = %d\n", a33);
	DKLog("a34 = %d\n", a34);
	DKLog("a35 = %d\n", a35);
	DKLog("a36 = %d\n", a36);
	DKLog("a37 = %d\n", a37);
	DKLog("a38 = %d\n", a38);
	DKLog("a39 = %d\n", a39);
	DKLog("a40 = %d\n", a40);
	DKLog("a41 = %d\n", a41);
	DKLog("a42 = %d\n", a42);
	DKLog("a43 = %d\n", a43);
	DKLog("a44 = %d\n", a44);
	DKLog("a45 = %d\n", a45);
	DKLog("a46 = %d\n", a46);
	DKLog("a47 = %d\n", a47);
	DKLog("a48 = %d\n", a48);
	DKLog("a49 = %d\n", a49);
	DKLog("a50 = %d\n", a50);
	DKLog("a51 = %d\n", a51);
	DKLog("a52 = %d\n", a52);
	DKLog("a53 = %d\n", a53);
	DKLog("a54 = %d\n", a54);
	DKLog("a55 = %d\n", a55);
	DKLog("a56 = %d\n", a56);
	DKLog("a57 = %d\n", a57);
	DKLog("a58 = %d\n", a58);
	DKLog("a59 = %d\n", a59);
	DKLog("a60 = %d\n", a60);
	DKLog("a61 = %d\n", a61);
	DKLog("a62 = %d\n", a62);
	DKLog("a63 = %d\n", a63);
	DKLog("a64 = %d\n", a64);
	DKLog("a65 = %d\n", a65);
	DKLog("a66 = %d\n", a66);
	DKLog("a67 = %d\n", a67);
	DKLog("a68 = %d\n", a68);
	DKLog("a69 = %d\n", a69);
	DKLog("a70 = %d\n", a70);
	DKLog("a71 = %d\n", a71);
	DKLog("a72 = %d\n", a72);
	DKLog("a73 = %d\n", a73);
	DKLog("a74 = %d\n", a74);
	DKLog("a75 = %d\n", a75);
	DKLog("a76 = %d\n", a76);
	DKLog("a77 = %d\n", a77);
	DKLog("a78 = %d\n", a78);
	DKLog("a79 = %d\n", a79);
	DKLog("a80 = %d\n", a80);
	DKLog("a81 = %d\n", a81);
	DKLog("a82 = %d\n", a82);
	DKLog("a83 = %d\n", a83);
	DKLog("a84 = %d\n", a84);
	DKLog("a85 = %d\n", a85);
	DKLog("a86 = %d\n", a86);
	DKLog("a87 = %d\n", a87);
	DKLog("a88 = %d\n", a88);
	DKLog("a89 = %d\n", a89);
	DKLog("a90 = %d\n", a90);
	DKLog("a91 = %d\n", a91);
	DKLog("a92 = %d\n", a92);
	DKLog("a93 = %d\n", a93);
	DKLog("a94 = %d\n", a94);
	DKLog("a95 = %d\n", a95);
	DKLog("a96 = %d\n", a96);
	DKLog("a97 = %d\n", a97);
	DKLog("a98 = %d\n", a98);
	DKLog("a99 = %d\n", a99);
	DKLog("a100 = %d\n", a100);
}
					   
					   
struct TestClass
{
	int& ConstFunction(int& a) const
	{
		return a;
	}
	int& FunctionReturnInt(int& a)
	{
		return a;
	}
	void MemberFunction0(void)
	{
		DKLog("%s(%x)\n", DKLIB_FUNCTION_NAME, this);
	}
	void MemberFunction1(const int& a)
	{
		DKLog("%s(%x) a:%d(%x)\n", DKLIB_FUNCTION_NAME, this, a, &a);
	}
	void MemberFunction2(const int& a, const int& b)
	{
		DKLog("%s(%x) a:%d(%x), b:%d(%x)\n", DKLIB_FUNCTION_NAME, this, a, &a, b, &b);
	}
	static int StaticMemberFunction(const int& a, const int& b)
	{
		DKLog("%s a:%d(%x), b:%d(%x)\n", DKLIB_FUNCTION_NAME, a, &a, b, &b);
		return 0;
	}
	int ffff(int x, int y) const
	{
		return x + y;
	}
	int& FirstFunc(int& a)
	{
		a += 10;
		DKLog("%s a:%d(%x)\n", DKLIB_FUNCTION_NAME, a, &a);
		return a;
	}
	int SecondFunc(int a)
	{
		a += 10;
		DKLog("%s a:%d(%x)\n", DKLIB_FUNCTION_NAME, a, &a);
		return a;
	}
	int ReceiverTest(int& a) const
	{
		a += 10;
		return a;
	}

	~TestClass(void)
	{
		DKLog("TestClass(%x) destroyed\n", this);
	}
};

static int functorRefCount = 0;
struct TestFunctor
{
	TestFunctor(void)
	{
		functorRefCount++;
		DKLog("%s (ref:%d)\n", DKLIB_FUNCTION_NAME, functorRefCount);
	}
	TestFunctor(const TestFunctor& f)
	{
		functorRefCount++;
		DKLog("%s -copied- (ref:%d)\n", DKLIB_FUNCTION_NAME, functorRefCount);
	}
	~TestFunctor(void)
	{
		functorRefCount--;
		DKLog("%s (ref:%d)\n", DKLIB_FUNCTION_NAME, functorRefCount);
	}
	int operator() (int x, int y) const
	{
		DKLog("%s (x:%d, y:%d) (ref:%d)\n", DKLIB_FUNCTION_NAME, x, y, functorRefCount);
		return x + y;
	}
};

#ifdef __clang__
	#if __has_feature(cxx_lambdas)
		#define LAMBDAS_ENABLED
	#endif
#endif
	
#ifdef __GNUC__
	#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 5
		#define LAMBDAS_ENABLED
	#endif
#endif
#ifdef _MSC_VER
	#if _MSC_VER >= 1600
		#define LAMBDAS_ENABLED
	#endif
#endif	

struct FunctionTest : public TestCase
{
	void RunTest()
	{
		int a = 113;
		int b = 200;

		DKObject<DKInvocation<int>> functor = NULL;
		{
			functor = DKFunction(TestFunctor())->Invocation(1,2);
		}
		functor->Invoke();
		functor = NULL;
		DKLog("functorRefCount#1 = %d\n", functorRefCount);
		{
			DKObject<DKFunctionSignature<int (int,int)>> sig = DKFunction(TestFunctor());
			sig->Invoke(1,2);
			sig->Invoke(3,4);
			sig->Invoke(5,6);
		}
		DKLog("functorRefCount#2 = %d\n", functorRefCount);


#ifdef LAMBDAS_ENABLED
		{
			DKLog("Lambda test\n");
			DKObject<DKInvocation<int>> lambda = NULL;
			{
				int y = 10;
				lambda = DKFunction( [y] (int x) -> int
				{
					DKLog("%s (x:%d)\n", DKLIB_FUNCTION_NAME, x);
					return x + y;
				})->Invocation(20);
			}
			DKLog("Lambda->Invoke: %d\n", lambda->Invoke());
		}
		{
			DKLog("Lambda test\n");
			DKObject<DKInvocation<int>> lambda = NULL;
			{
				int y = 10;
				lambda = DKFunction( [y] (int x) -> int
				{
					DKLog("%s (x:%d)\n", DKLIB_FUNCTION_NAME, x);
					return x + y;
				})->Invocation(20);
			}
			DKLog("Lambda->Invoke: %d\n", lambda->Invoke());
		}

#endif

		{
			DKLog("Direct invoke from DKFunctionSignature.\n");
			DKObject<TestClass> test = DKObject<TestClass>::New();
			DKFunction(test, &TestClass::MemberFunction0)->Invoke();
			DKFunction(test, &TestClass::MemberFunction1)->Invoke(a);
			DKFunction(test, &TestClass::MemberFunction2)->Invoke(a,b);
			DKFunction(&TestClass::StaticMemberFunction)->Invoke(a, b);			
		}
		{
			DKLog("Class-member function test\n");
			DKObject<TestClass> test = DKObject<TestClass>::New();
			DKObject<DKOperation> op1 = (DKOperation*)DKFunction(test, &TestClass::MemberFunction0)->Invocation();
			DKObject<DKOperation> op2 = (DKOperation*)DKFunction(test, &TestClass::MemberFunction1)->Invocation(a);
			DKObject<DKOperation> op3 = (DKOperation*)DKFunction(test, &TestClass::MemberFunction2)->Invocation(a,b);
			DKObject<DKOperation> op4 = (DKOperation*)DKFunction(&TestClass::StaticMemberFunction)->Invocation(a, b);

			const DKObject<TestClass> test2 = test;
			DKObject<DKOperation> op5 = (DKOperation*)DKFunction(test2, &TestClass::ConstFunction)->Invocation(a);

			DKLog("Operation will invoked object:%x, a:%d(%x), b:%d(%x)\n", (TestClass*)test, a, &a, b, &b);
			op1->Perform();
			op2->Perform();
			op3->Perform();
			op4->Perform();
			DKLog("Test object will destroyed.\n");
		}
		{
			DKLog("Invocation function test\n");
			DKObject<TestClass> test = DKObject<TestClass>::New();
			DKObject<DKInvocation<int&>> inv1 = DKFunction(test, &TestClass::FunctionReturnInt)->Invocation(a);
			DKObject<DKInvocation<int&>> inv2 = DKFunction(DKObject<TestClass>::New(), &TestClass::ConstFunction)->Invocation(b);
			int& ret1 = inv1->Invoke();
			int& ret2 = inv2->Invoke();
			DKLog("Invocation-1 with return: %d(%x)\n", ret1, &ret1);
			DKLog("Invocation-2 with return: %d(%x)\n", ret2, &ret2);
		}
		{
			DKLog("Global function test\n");
			DKObject<DKOperation> op1 = (DKOperation*)DKFunction(TestFunction0)->Invocation();
			DKObject<DKOperation> op2 = (DKOperation*)DKFunction(TestFunction1)->Invocation(a);
			DKObject<DKOperation> op3 = (DKOperation*)DKFunction(TestFunction2)->Invocation(a, b);
			DKLog("Operation will invoked a:%d(%x), b:%d(%x)\n", a, &a, b, &b);
			op1->Perform();
			op2->Perform();
			op3->Perform();
		}
		{
			DKLog("inline function test\n");
			DKObject<DKOperation> op1 = (DKOperation*)DKFunction(InlineFunction0)->Invocation();
			DKObject<DKOperation> op2 = (DKOperation*)DKFunction(InlineFunction1)->Invocation(a);
			DKObject<DKOperation> op3 = (DKOperation*)DKFunction(InlineFunction2)->Invocation(a, b);
			DKLog("Operation will invoked a:%d(%x), b:%d(%x)\n", a, &a, b, &b);
			op1->Perform();
			op2->Perform();
			op3->Perform();
		}
/*		{
			DKLog("Middle/Long parameter function test.\n");
			DKObject<DKOperation> op1 = DKFunction(MiddleParameterFunc)->Invocation
			(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
			 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
			 41, 42, 43, 44, 45, 46, 47, 48, 49, 50);
			DKObject<DKOperation> op2 = DKFunction(LongParameterFunc)->Invocation
			(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
			 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
			 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
			 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
			 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100);
			DKLog("Operation will invoked.\n");
			op1->Perform();
			op2->Perform();
		}
*/
		{
			DKLog("Value to Invocation test\n");
			DKObject<DKInvocation<int>> inv1 = DKValue(a);
			DKObject<DKInvocation<int&>> inv2 = DKValue<int&>(a);
			int ret1 = inv1->Invoke();
			int& ret2 = inv2->Invoke();
			DKLog("Invocation<int> return:%d\n", ret1);
			DKLog("Invocation<int&> return:%d(%x)\n", ret2, &ret2);
		}

/*		{
			DKLog("DKAtomicFunction test begin\n");
			// DKAtomicFunction 테스트
			DKObject<DKFunctionSignature<void (double)>> sleepFunc = DKAtomicFunction(SleepFunction);
			DKObject<DKThread> thread1 = DKThread::Create(sleepFunc->Invocation(3.0));
			DKObject<DKThread> thread2 = DKThread::Create(sleepFunc->Invocation(3.0));
			DKObject<DKThread> thread3 = DKThread::Create(sleepFunc->Invocation(3.0));
			DKObject<DKThread> thread4 = DKThread::Create(sleepFunc->Invocation(3.0));

			thread1->WaitTerminate();
			thread2->WaitTerminate();
			thread3->WaitTerminate();
			thread4->WaitTerminate();
			DKLog("DKAtomicFunction test end.\n");
		}
*/
	}
} functionTest;

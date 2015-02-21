#include "TestCase.h"

typedef DKArray<TestCase*, DKSpinLock> TestCasesArray;
TestCasesArray& TestCases(void)
{
	static TestCasesArray array;
	return array;	
}

TestCase::TestCase(void)
{
	TestCases().Add(this);
}

TestCase::~TestCase(void)
{
}

void TestCase::Run(void)
{
	for (int i = 0; i < TestCases().Count(); i++)
	{
		TestCase* test = TestCases().Value(i);
		DKLog("--------- %s Begin ---------\n", typeid(*test).name());
		test->RunTest();
		DKLog("--------- %s End -----------\n", typeid(*test).name());
	}
}

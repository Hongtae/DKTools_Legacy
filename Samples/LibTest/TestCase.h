#pragma once
#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class TestCase
{
public:
	TestCase(void);
	virtual ~TestCase(void);

	virtual void RunTest(void) = 0;

	static void Run(void);
};

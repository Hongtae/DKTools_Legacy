#include "../TestCase.h"


#define OUTPUTLOG_INT(expr)		DKLog(#expr" = %d\n", expr)
#define OUTPUTLOG_STRING(expr)	DKLog(#expr" = %ls\n", expr)

struct TypeTest : public TestCase
{
	class ClassA
	{
	};

	class ClassB : public ClassA
	{
	};

	class ClassC
	{
	};

	class ClassD : public ClassA, public ClassC
	{
	};


	struct SA
	{
		virtual ~SA() {}
		long value[200];
	};
	struct SB
	{
		virtual ~SB() {}
		float value[160];
	};
	struct SC : public SA, public SB
	{
		int value;
	};

	void RunTest()
	{
		DKObject<SC> psc = DKObject<SC>::New();
		DKObject<SA> psa = psc.SafeCast<SA>();
		DKObject<SB> psb = psc.SafeCast<SB>();

		DKLog("psc.IsManaged:%d, psa.IsManaged:%d, psb.IsManaged:%d\n", psc.IsManaged(), psa.IsManaged(), psb.IsManaged());
		DKLog("psc.IsPolymorphic:%d, psa.IsPolymorphic:%d, psb.IsPolymorphic:%d\n", psc.IsPolymorphic(), psa.IsPolymorphic(), psb.IsPolymorphic());
		DKLog("[ptr] psc = %x, psa = %x, psb = %x\n", (SC*)psc, (SA*)psa, (SB*)psb);
		DKLog("[base] psc = %x, psa = %x, psb = %x\n", psc.BaseAddress(), psa.BaseAddress(), psb.BaseAddress());



		DKLog("DKTrue::Value = %d\n", DKTrue::Value);
		DKLog("DKFalse::Value = %d\n", DKFalse::Value);

		DKLog("Type Convertible<ClassA, ClassA>: %d\n", DKTypeConversionTest<ClassA, ClassA>());
		DKLog("Type Convertible<ClassA, ClassB>: %d\n", DKTypeConversionTest<ClassA, ClassB>());
		DKLog("Type Convertible<ClassB, ClassA>: %d\n", DKTypeConversionTest<ClassB, ClassA>());
		DKLog("Type Convertible<ClassA, ClassC>: %d\n", DKTypeConversionTest<ClassA, ClassC>());
		DKLog("Type Convertible<ClassB, ClassC>: %d\n", DKTypeConversionTest<ClassB, ClassC>());
		DKLog("Type Convertible<ClassA, ClassD>: %d\n", DKTypeConversionTest<ClassA, ClassD>());
		DKLog("Type Convertible<ClassC, ClassD>: %d\n", DKTypeConversionTest<ClassC, ClassD>());
	}
} typeTest;

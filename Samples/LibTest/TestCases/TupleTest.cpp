#include "../TestCase.h"


struct TupleTest : public TestCase
{
	struct TestA
	{
		int value;
	};
	struct TestB : public TestA
	{
		int value1;
		int value2;
	};



	void RunTest()
	{
		DKLog("DKLIB_TYPES = %ls\n", (const wchar_t*)DKTypeInfo(typeid(DKTypeList<int, int, long, double>)).Name());

		DKTuple<int, int, const int*, const int&, const char*, double> tuple;
	//	DKTupleTypes<int, int, const int*, const int&, const char*, double>::Tuple tuple;

		int x = 88;
		int y = 1234;
		//tuple.Unit<0>().SetValue(1);
		//tuple.Unit<1>().SetValue(2);
		//tuple.Unit<2>().SetValue(&x);
		//tuple.Unit<3>().SetValue(y);

		tuple.SetValue<0>(1, 2, &x, y, "test", 3.141592f);

		DKLog("Tuple length: %d\n", tuple.Length);
		DKLog("Tuple at 0: %d\n", tuple.Unit<0>().Value());
		DKLog("Tuple at 1: %d\n", tuple.Unit<1>().Value());
		DKLog("Tuple at 2: %d\n", *tuple.Unit<2>().Value());
		DKLog("Tuple at 3: %d\n", tuple.Unit<3>().Value());
		DKLog("Tuple at 3ptr: %x, y = %x\n", &tuple.Unit<3>().Value(), &y);
		DKLog("Tuple at 4: %s\n", tuple.Unit<4>().Value());
		DKLog("Tuple at 5: %f\n", tuple.Unit<5>().Value());

		//DKLog("Type: %ls\n", DKTypeInfo(typeid(DKTypeTraits<int&>::UnqualifiedType)).Name());

	}
} TupleTest;

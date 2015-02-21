#include "../TestCase.h"


struct CollectionTest : public TestCase
{
	bool EnumFuncArrayAndSet(const int& a)
	{
		DKLog("[%s] value=%d\n", DKLIB_FUNCTION_NAME, a);
		return true;
	}
	bool EnumFuncPair(const DKMap<int,int>::Pair& p)
	{
		DKLog("[%s] key=%d, value=%d\n", DKLIB_FUNCTION_NAME, p.key, p.value);
		return true;
	}
	void EnumTest(size_t count)
	{
		DKArray<int> arrayItems;
		DKOrderedArray<int> orderedItems; 
		DKMap<int, int> mapItems;
		DKSet<int> setItems;
		DKQueue<int> queueItems;

		arrayItems.Reserve(count);
		orderedItems.Reserve(count);

		DKLog("Generating %d itmes\n", count);
		for (unsigned int i = 0; i < count; i++)
		{
			//int v = DKRandom();
			int v = i;
			arrayItems.Add(v);
			orderedItems.Insert(v);
			mapItems.Insert(v, v);
			setItems.Insert(v);
			queueItems.PushBack(v);
		}

		arrayItems.EnumerateBackward([this](int a){ this->EnumFuncArrayAndSet(a); });
		orderedItems.EnumerateBackward([this](int a){ this->EnumFuncArrayAndSet(a); });
		mapItems.EnumerateBackward([this](DKMap<int, int>::Pair& a){ this->EnumFuncPair(a); });
		setItems.EnumerateBackward([this](int a){ this->EnumFuncArrayAndSet(a); });
		queueItems.EnumerateBackward([this](int a){ this->EnumFuncArrayAndSet(a); });
	}
	void TimeTest(size_t count)
	{
		DKArray<int> items;
		items.Reserve(count);
		DKLog("Generating %d itmes\n", count);
		for (unsigned int i = 0; i < count; i++)
			items.Add(DKRandom());

		DKTimer timer;

		{
			DKLog("Insert %d items in DKArray\n", count);
			DKArray<int> collection;
			timer.Reset();
			for (unsigned int i = 0; i < items.Count(); i++)
			{
				collection.Add(items.Value(i));
				collection.Sort(0, collection.Count(), DKArraySortAscending<int>);
			}
			DKLog("Insert %d items in DKArray = %f\n", collection.Count(), timer.Elapsed());
		}
		{
			DKLog("Insert %d items in DKSortedArray\n", count);
			DKOrderedArray<int> collection;
			timer.Reset();
			for (unsigned int i = 0; i < items.Count(); i++)
				collection.Insert(items.Value(i));
			DKLog("Insert %d items in DKSortedArray = %f\n", collection.Count(), timer.Elapsed());
		}
		{
			DKLog("Insert %d items in DKSet\n", count);
			DKSet<int> collection;
			timer.Reset();
			for (unsigned int i = 0; i < items.Count(); i++)
				collection.Insert(items.Value(i));
			DKLog("Insert %d items in DKSet = %f\n", collection.Count(), timer.Elapsed());
		}
	}
	void RunTest()
	{
		EnumTest(3);
		TimeTest(1000);
	}
} collectionTest;

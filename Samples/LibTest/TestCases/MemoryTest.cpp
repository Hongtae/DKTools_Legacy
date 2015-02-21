#include "../TestCase.h"

/*
namespace MemoryTestPrivate
{
	void MemoryAssert(int exp)
	{
		if (exp == 0)
			throw "error";
	}
	static const unsigned int maximumAllocationSize = 1024;

	template <unsigned int count> struct BitwiseValue
	{
		enum {kLength = count};
		enum {kNumBytes = kLength / 8 + (kLength % 8 ? 1 : 0)};

		unsigned char value[kNumBytes];

		bool GetBit(size_t index) const
		{
			size_t idx = index / 8;
			size_t bits = index % 8;
			return ((value[idx] >> bits) & 0x1) == 0x1;
		}
		void SetBit(size_t index)
		{
			size_t idx = index / 8;
			size_t bits = index % 8;
			value[idx] = value[idx] | (0x1 << bits);
		}
		void ClearBit(size_t index)
		{
			size_t idx = index / 8;
			size_t bits = index % 8;
			value[idx] = value[idx] & ~(0x1 << bits);
		}
		bool IsAllBitsClear(void) const
		{
			for (int i = 0; i < kNumBytes; i++)
			{
				if (value[i] != 0)
					return false;
			}
			return true;
		}

		struct BitmapChunk
		{
			enum {kTotalBytes = maximumAllocationSize * 2};
			enum {kBlockSize = 4};
			enum {kTotalBlocks = kTotalBytes / kBlockSize};

			BitwiseValue<kTotalBlocks> allocatedBits;		// 사용중인 블럭은 true
			BitwiseValue<kTotalBlocks> leaderBits;			// 리더 블록만 true
			unsigned char data[kTotalBytes];
			BitmapChunk* prevChunk;
			BitmapChunk* nextChunk;
		};

		// 포인터를 이용하여 상수시간에 해당 Chunk 를 찾는 기능 필요.
		// 또한 상수시간에 size 의 수용공간이 있는 Chunk 를 찾는 기능도 필요.

		void* Allocate(size_t size)
		{
			return NULL;
		}
		void Deallocate(void* p)
		{
		}
		// p != NULL, s > 0 이면 s 크기로 리사이즈
		// p != NULL, s == 0 이면 해제
		// p == NULL, s > 0 이면 새로 할당
		// p == NULL, s == 0 이면 아무것도 안함.
		void* Reallocate(void* p, size_t s)
		{
			return NULL;
		}
	};
};

using namespace MemoryTestPrivate;

struct MemoryTest : public TestCase
{
	void RunTest()
	{
		DKArray<size_t> sizes;
		DKArray<void*> ptrsInternal;
		DKArray<void*> ptrsExternal;

		for (int i = 0; i < 100; i++)
			sizes.Add(DKRandom() % (maximumAllocationSize+1));

		ptrsInternal.Reserve(sizes.Count());
		ptrsExternal.Reserve(sizes.Count());

		bool useInternal = false;
		bool useExternal = true;


		if (useInternal)
		{

		}
		if (useExternal)
		{
			DKTimer timer;
			size_t totalAllocated = 0;

			timer.Check();

			DKLog("allocating %d objects with malloc.\n", sizes.Count());
			for (int i = 0; i < sizes.Count(); i++)
			{
				ptrsExternal.Add(::malloc(sizes.Value(i)));
				totalAllocated += sizes.Value(i);
			}
			DKLog("%f ms elapsed (total:%d)\n", timer.Elapsed(), totalAllocated);
		}
	}
} memoryTest;

*/


struct MemoryTest : public TestCase
{
	void RunTest()
	{
	}
/*
	typedef void* (*AllocFunc)(size_t);
	typedef void (*DeallocFunc)(void*);
	typedef void* (*ReallocFunc)(void*, size_t);

	enum MemTestSize
	{
		MTSize4 = 0,
		MTSize8,
		MTSize12,
		MTSize16,
		MTSize128,
		MTSize256,
		MTSize1024,
		MTSizeMax,
	};
	struct MemTest
	{
		int index;
		size_t size;
	};
	size_t GetMemTestSize(MemTestSize s)
	{
		switch (s)
		{
		case MTSize4:		return 4;
		case MTSize8:		return 8;
		case MTSize12:		return 12;
		case MTSize16:		return 16;
		case MTSize128:		return 128;
		case MTSize256:		return 256;
		case MTSize1024:	return 1024;
		}
		return DKRandom() % 1025;
	}
	double DoTest(AllocFunc allocFunc, DeallocFunc deallocFunc, ReallocFunc reallocFunc, size_t maxAlloc)
	{
		DKArray<void*> ptrArray((void*)NULL, maxAlloc);
		DKTimer timer;
		timer.Reset();
		for (size_t i = 0; i < this->testTable.Count(); ++i)
		{
			MemTest& mt = this->testTable.Value(i);
			if (ptrArray.Value(mt.index))
			{
				deallocFunc(ptrArray.Value(mt.index));
				ptrArray.Value(mt.index) = NULL;
			}
			else
			{
				ptrArray.Value(mt.index) = allocFunc(mt.size);
			}
		}
		for (size_t i = 0; i < ptrArray.Count(); ++i)
		{
			void* p = ptrArray.Value(i);
			if (p)
				deallocFunc(p);
		}
		return timer.Elapsed();
	}
	void RunTest()
	{
		unsigned int maxTest = 4096000;
		unsigned int maxAlloc = 1024;

		testTable.Reserve(maxTest);

		for (int i = 0; i < maxTest; i++)
		{
			MemTestSize s = (MemTestSize)(DKRandom()% MTSizeMax);
			
			MemTest t = {DKRandom() % maxAlloc, };
			testTable.Add(t);
		}

		double elapsed1 = DoTest(DKMemory::SysDefault::Alloc, DKMemory::SysDefault::Free, DKMemory::SysDefault::Realloc, maxAlloc);
		DKLog("default malloc,free,realloc = %f\n", elapsed1);

		double elapsed2 = DoTest(DKMemory::Managed::Alloc, DKMemory::Managed::Free, DKMemory::Managed::Realloc, maxAlloc);
		DKLog("custom malloc,free,realloc = %f\n", elapsed2);

		DKLog("mem test end.\n");
	}
	DKArray<MemTest> testTable;
*/
} memoryTest;

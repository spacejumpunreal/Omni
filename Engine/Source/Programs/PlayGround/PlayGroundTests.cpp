#include "Programs/PlayGround/PlayGroundTests.h"
#include "Programs/PlayGround/TestAsync.h"
#include "Programs/PlayGround/TestDispatchQueue.h"
#include "Programs/PlayGround/TestMultiThreadAllocation.h"
#include "Runtime/Concurrency/LockfreeContainer.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Memory/MemoryArena.h"
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Test/AssertUtils.h"
#include "Runtime/Test/MultiThreadTest.h"

namespace Omni
{
	void TestLockfreeStackSingleThread()
	{
		LockfreeStack stk;
		constexpr u64 testCount = 1024;
		u64 pushIndex = 0;
		for (; pushIndex < testCount; ++pushIndex)
		{
			LockfreeNode* n = LockfreeNodeCache::Alloc();
			n->Data[0] = (void*)pushIndex;
			stk.Push(n);
		}
		while (true)
		{
			LockfreeNode* poped = stk.Pop();
			if (poped == nullptr)
				break;
			CheckAlways(((u64)(poped->Data[0])) == --pushIndex);
			LockfreeNodeCache::Free(poped);
		}
	}
	void TestLockfreeQueueSingleThread()
	{
		constexpr int TestCount = 1024;
		LockfreeQueue<1> queue;
		for (u64 i = 0; i < TestCount; ++i)
		{
			LockfreeNode* n = LockfreeNodeCache::Alloc();
			n->Data[0] = (void*)i;
			queue.Enqueue(n);
		}
		for (u64 i = 0; i < TestCount; ++i)
		{
			LockfreeNode* n = queue.Dequeue();
			CheckAlways(n->Data[0] == (void*)i);
			LockfreeNodeCache::Free(n);
		}
		for (u64 i = 0; i < TestCount; ++i)
		{
			LockfreeNode* n0 = LockfreeNodeCache::Alloc();
			n0->Data[0] = (void*)(2 * i);
			queue.Enqueue(n0);
			LockfreeNode* n1 = LockfreeNodeCache::Alloc();
			n1->Data[0] = (void*)(2 * i + 1);
			queue.Enqueue(n1);

			LockfreeNode* d0 = queue.Dequeue();
			LockfreeNode* d1 = queue.Dequeue();
			CheckAlways(d0->Data[0] == (void*)(2 * i));
			CheckAlways(d1->Data[0] == (void*)(2 * i + 1));
			LockfreeNodeCache::Free(d0);
			LockfreeNodeCache::Free(d1);
		}
	}
	void TestPMRAllocate()
	{
		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(Omni::MemoryKind::UserDefault);
		size_t testSize = 1024;
		std::byte* p = alloc.allocate(testSize);
		memset(p, 0, testSize);
		alloc.deallocate(p, testSize);
	}
	void TestScratchStack()
	{
		ScratchStack& stack = MemoryModule::Get().GetThreadScratchStack();
		stack.Push();
		{
			MemoryArenaScope s0 = stack.PushScope();
			void* f = nullptr;
			{
				MemoryArenaScope s1 = stack.PushScope();
				f = stack.Allocate(3);
				constexpr u32 allocSizes[] = { 1, 2, 4, 8, 12, 17, 19, 354 };
				for (u32 sz : allocSizes)
				{
					stack.Allocate(sz);
				}
				void* m = nullptr;
				{
					MemoryArenaScope s2 = stack.PushScope();
					m = stack.Allocate(1);
					for (u32 sz : allocSizes)
					{
						stack.Allocate(sz);
					}
				}
				void* m1 = stack.Allocate(2);
				CheckAlways(m1 == m);
			}
			void* f1 = stack.Allocate(1);
			CheckAlways(f1 == f);
		}
		stack.Pop();
	}
	void TestSpinLock()
	{
		SpinLock sl;

		sl.Lock();
		std::thread x = std::thread([&]
			{
				//std::this_thread::sleep_for(std::chrono::seconds(5));
				sl.Unlock();
			});
		sl.Lock();
		bool succeeed = sl.TryLock();
		CheckAlways(!succeeed);
		sl.Unlock();
		x.join();
	}

	void TestMultiThreadAllocation()
	{
		//TestMultiThreadAllocation
		RunOnEveryWorker<struct TestMultiThreadAllocation> obj;
	}
	void TestDispatchQueue()
	{
		RunOnEveryWorker<struct TestDispatchQueue> obj(1);
	}
	void TestAsync()
	{
		RunOnEveryWorker<struct TestAsync> obj(1);
	}
}
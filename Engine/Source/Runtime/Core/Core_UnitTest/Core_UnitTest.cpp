#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Base/MultiThread/LockfreeContainer.h"
#include "Runtime/Base/MultiThread/SpinLock.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Core_UnitTest/TestAsync.h"
#include "Runtime/Core/Core_UnitTest/TestDispatchQueue.h"
#include "Runtime/Core/Core_UnitTest/TestMultiThreadAllocation.h"
#include "Runtime/Core/Concurrency/LockfreeNodeCache.h"
#include "Runtime/Core/Concurrency/RunOnEveryWorker.h"
#include "Runtime/Core/System/System.h"
#include "External/googletest/include/gtest/gtest.h"
#include <array>
#include <random>


namespace Omni
{
	//forward decls

	void TestLockfreeStackSingleThread()
	{//single thread test for LockfreeStack
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
	{//single thread test for LockfreeQueue
		constexpr int TestCount = 1024;
		LockfreeQueue<LockfreeNodeCache> queue(1);
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

	void TestLockfreeQueueMultiThread()
	{//multi-thread test for LockfreeQueue
		constexpr int Repeats = 256;
		for (int iRepeat = 0; iRepeat < Repeats; ++iRepeat)
		{
			constexpr int TestSize = 1024 * 16;
			constexpr int QueueLength = 8;
			constexpr size_t LocalMaxKeep = 4;
			LockfreeQueue<LockfreeNodeCache> queue(1);
			for (int i = 0; i < QueueLength; ++i)
			{
				LockfreeNode* node = LockfreeNodeCache::Alloc();
				node->Data[0] = (void*)(u64)i;
				queue.Enqueue(node);
			}
			struct Tester
			{
				static void DoTest(u64 idx, LockfreeQueue<LockfreeNodeCache>* queue)
				{
					std::random_device rd;  //Will be used to obtain a seed for the random number engine
					std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
					gen.seed((unsigned int)idx);
					std::uniform_int_distribution<> dis(0, 9);
					PMRAllocator defaultAllocator = MemoryModule::Get().GetPMRAllocator(MemoryKind::UserDefault);
					PMRVector<LockfreeNode*> localKeep(defaultAllocator);
					for (int i = 0; i < TestSize; ++i)
					{
						bool doPop = true;
						if (localKeep.size() == 0)
							doPop = true;
						else if (localKeep.size() == LocalMaxKeep)
							doPop = false;
						else
							doPop = dis(gen) > 4;
						if (doPop)
						{
							LockfreeNode* n = queue->Dequeue();
							if (n)
								localKeep.push_back(n);
						}
						else if (localKeep.size() > 0)
						{
							LockfreeNode* n = localKeep.back();
							CheckAlways(n != nullptr);
							queue->Enqueue(n);
							localKeep.pop_back();
						}
					}
					for (LockfreeNode* n : localKeep)
					{
						queue->Enqueue(n);
					}
				}
			};
			PMRAllocator defaultAllocator = MemoryModule::Get().GetPMRAllocator(MemoryKind::UserDefault);
			PMRVector<std::thread> threads(defaultAllocator);
			u64 nThreads = std::thread::hardware_concurrency();
			threads.resize(nThreads);
			std::array<bool, QueueLength> allKeys{};
			for (size_t i = 0; i < allKeys.size(); ++i)
			{
				allKeys[i] = false;
			}
			for (u64 iThread = 0; iThread < nThreads; ++iThread)
			{
				threads[iThread] = std::thread(Tester::DoTest, iThread, &queue);
			}
			for (u64 iThread = 0; iThread < nThreads; ++iThread)
			{
				threads[iThread].join();
			}
			PMRVector<int> collectedThings(defaultAllocator);
			while (true)
			{
				LockfreeNode* n = queue.Dequeue();
				if (!n)
					break;
				int i = (int)(u64)(n->Data[0]);
				collectedThings.push_back(i);
				LockfreeNodeCache::Free(n);
				CheckAlways(i < QueueLength);
				CheckAlways(!allKeys[i]);
				allKeys[i] = true;
			}
		}
	}

	void TestLockfreeStackMultiThread()
	{//multi-thread test for LockfreeStack
		constexpr int Repeats = 256;
		for (int iRepeat = 0; iRepeat < Repeats; ++iRepeat)
		{
			constexpr int TestSize = 1024 * 16;
			constexpr int QueueLength = 8;
			constexpr size_t LocalMaxKeep = 4;
			LockfreeStack stk;
			for (int i = 0; i < QueueLength; ++i)
			{
				LockfreeNode* node = LockfreeNodeCache::Alloc();
				node->Data[0] = (void*)(u64)i;
				stk.Push(node);
			}
			struct Tester
			{
				static void DoTest(u64 idx, LockfreeStack* stk)
				{
					std::random_device rd;  //Will be used to obtain a seed for the random number engine
					std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
					gen.seed((unsigned int)idx);
					std::uniform_int_distribution<> dis(0, 9);
					PMRAllocator defaultAllocator = MemoryModule::Get().GetPMRAllocator(MemoryKind::UserDefault);
					PMRVector<LockfreeNode*> localKeep(defaultAllocator);
					for (int i = 0; i < TestSize; ++i)
					{
						bool doPop = true;
						if (localKeep.size() == 0)
							doPop = true;
						else if (localKeep.size() == LocalMaxKeep)
							doPop = false;
						else
							doPop = dis(gen) > 4;
						if (doPop)
						{
							LockfreeNode* n = stk->Pop();
							if (n)
								localKeep.push_back(n);
						}
						else if (localKeep.size() > 0)
						{
							LockfreeNode* n = localKeep.back();
							CheckAlways(n != nullptr);
							stk->Push(n);
							localKeep.pop_back();
						}
					}
					for (LockfreeNode* n : localKeep)
					{
						stk->Push(n);
					}
				}
			};
			PMRAllocator defaultAllocator = MemoryModule::Get().GetPMRAllocator(MemoryKind::UserDefault);
			PMRVector<std::thread> threads(defaultAllocator);
			u64 nThreads = std::thread::hardware_concurrency();
			threads.resize(nThreads);
			std::array<bool, QueueLength> allKeys{};
			for (size_t i = 0; i < allKeys.size(); ++i)
			{
				allKeys[i] = false;
			}
			for (u64 iThread = 0; iThread < nThreads; ++iThread)
			{
				threads[iThread] = std::thread(Tester::DoTest, iThread, &stk);
			}
			for (u64 iThread = 0; iThread < nThreads; ++iThread)
			{
				threads[iThread].join();
			}
			PMRVector<int> collectedThings(defaultAllocator);
			while (true)
			{
				LockfreeNode* n = stk.Pop();
				if (!n)
					break;
				int i = (int)(u64)(n->Data[0]);
				collectedThings.push_back(i);
				LockfreeNodeCache::Free(n);
				CheckAlways(i < QueueLength);
				CheckAlways(!allKeys[i]);
				allKeys[i] = true;
			}
		}
	}

	void TestPMRAllocate()
	{//very easy test of using PMRAllocator
		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(Omni::MemoryKind::UserDefault);
		size_t testSize = 1024;
		std::byte* p = alloc.allocate(testSize);
		memset(p, 0, testSize);
		alloc.deallocate(p, testSize);
	}

	void TestScratchStack()
	{//simple test for ScratchStack
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
	{//simple test for SpinLock
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
		RunOnAllWorker<struct TestMultiThreadAllocation> obj;
	}

	void TestDispatchQueue()
	{
		RunOnAllWorker<struct TestDispatchQueue> obj(1);
	}

	void TestAsync()
	{
		RunOnAllWorker<struct TestAsync> obj(1);
	}

	FORCEINLINE void TestAll()
	{
		
#if 1
		//Lockfree container related tests
		TestLockfreeStackSingleThread();
		TestLockfreeQueueSingleThread();
		TestLockfreeStackMultiThread();
		TestLockfreeQueueMultiThread();
#endif

#if 1
		//Base primitive related tests
		TestPMRAllocate();
		TestScratchStack();
		TestSpinLock();
		TestMultiThreadAllocation();
#endif

#if 1
		//Concurrency related testes
		TestDispatchQueue();
		TestAsync();
#endif
	}

	void CoreUnitTestCode()
	{
		TestAll();
		System::GetSystem().TriggerFinalization(true);
	}
}

int main(int argc, char** argv) {
    printf("Running main() from %s\n", __FILE__);


	Omni::System::CreateSystem();
	Omni::System& system = Omni::System::GetSystem();
	system.InitializeAndJoin(0, nullptr, Omni::CoreUnitTestCode, nullptr);
	system.DestroySystem();


    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

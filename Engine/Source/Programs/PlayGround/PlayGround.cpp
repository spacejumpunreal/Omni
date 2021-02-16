#include "Runtime/System/System.h"
#include "Runtime/Concurrency/ConcurrencyModule.h"
#include "Runtime/Concurrency/JobPrimitives.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Memory/MemoryArena.h"
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Misc/ArrayUtils.h"
#include "Runtime/Misc/AssertUtils.h"
#include "Runtime/Platform/PlatformAPIs.h"
#include <chrono>
#include <functional>
#include <random>

#define TRAP_MALLOCATIONS 0
#if TRAP_MALLOCATIONS
void* operator new(std::size_t sz)
{
	std::printf("global op new called, size = %zu\n", sz);
	void* ptr = std::malloc(sz);
	return ptr;

}
void operator delete(void* ptr)
{
	std::puts("global op delete called");
	std::free(ptr);
}

void* operator new(std::size_t sz, std::align_val_t)
{
	std::printf("global op new called, size = %zu\n", sz);
	void* ptr = std::malloc(sz);
	return ptr;

}
void operator delete(void* ptr, std::align_val_t)
{
	std::puts("global op delete called");
	std::free(ptr);
}
#endif


namespace Omni
{
	struct AllocJobData
	{
		size_t				IThread;
		DispatchGroup*		Group;
	};
	void AllocJobFunc0(AllocJobData* jobData)
	{
		constexpr size_t Size64K = 64 * 1024;
		constexpr size_t Amount = 1024 * 1024 * 512;
		constexpr size_t History = 8;

		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::CacheLine);
		std::mt19937 gen;
		gen.seed((unsigned int)jobData->IThread);
		std::uniform_int_distribution<> dis((int)(Size64K / 3), (int)(Size64K * 3 / 4));
		u8* slots[History];
		size_t sizes[History];
		memset(slots, 0, sizeof(slots));
		memset(sizes, 0, sizeof(sizes));
		size_t allocated = 0;
		size_t slotIndex = 0;
		int runIndex = 0;
		while (allocated < Amount)
		{
			u8* u8Ptr = slots[slotIndex];
			size_t sz = sizes[slotIndex];
			u8 w = (u8)sz;
			if (sz != 0)
			{
				bool bad = false;
				for (size_t iBytes = 0; iBytes < sz; ++iBytes)
				{
					if (u8Ptr[iBytes] != w)
					{
						bad = true;
						break;
					}
				}
				CheckAlways(!bad);
				alloc.resource()->deallocate(slots[slotIndex], 0);
			}
			//size_t size = (size_t)dis(gen);
			size_t size = Size64K - 128;
			std::byte* p = alloc.allocate(size);
			memset(p, (int)size, size);
			slots[slotIndex] = (u8*)p;
			sizes[slotIndex] = size;
			slotIndex = (slotIndex + 1) % History;
			allocated += size;
			++runIndex;
		}
		for (size_t iSlot = 0; iSlot < History; ++iSlot)
		{
			if (slots[iSlot])
			{
				alloc.resource()->deallocate(slots[iSlot], 0);
			}
		}
		jobData->Group->Leave();
	}
	void AllocJobFunc1()
	{
		System::GetSystem().TriggerFinalization();
	}
	void MainThreadTest()
	{
#if false
		{
			PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(Omni::MemoryKind::UserDefault);
			size_t testSize = 1024;
			std::byte* p = alloc.allocate(testSize);
			memset(p, 0, testSize);
			alloc.deallocate(p, testSize);
		}
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
#endif
#if true
		{
			constexpr size_t NThreads = 8;
			DispatchGroup& group = DispatchGroup::Create(0);
			AllocJobData jd;
			jd.Group = &group;
			DispatchWorkItem* lastJob = nullptr;
			for (size_t iThread = 0; iThread < NThreads; ++iThread)
			{
				jd.IThread = iThread;
				DispatchWorkItem& item = DispatchWorkItem::Create(AllocJobFunc0, &jd);
				item.Next = lastJob;
				lastJob = &item;
				group.Enter();
			}
			DispatchWorkItem& item = DispatchWorkItem::Create(AllocJobFunc1);
			group.Notify(item, nullptr);
			ConcurrencyModule::Get().Async(*lastJob);
		}
#endif
	}
}

int main(int, const char** )
{
	Omni::System::CreateSystem();
	Omni::System& system = Omni::System::GetSystem();
	const char* engineArgv[] =
	{
		"",
	};
	system.InitializeAndJoin(ARRAY_LENGTH(engineArgv), engineArgv, Omni::MainThreadTest);
	system.DestroySystem();
 
	return 0;
}

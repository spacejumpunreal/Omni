#include "Runtime/System/System.h"
#include "Runtime/Concurrency/JobPrimitives.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Memory/MemoryArena.h"
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Misc/ArrayUtils.h"
#include "Runtime/Misc/AssertUtils.h"
#include <chrono>
#include <functional>

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
	struct SuperStruct
	{
		int Data[64];
	};
	void AvoidFunction()
	{
		printf("AvoidFunction()\n");
	}
	void Func0(int* x)
	{
		printf("Func0(%x)\n", *x);
	}
	void Func1(SuperStruct* s)
	{
		printf("Func1(s->Data[63]:%d)\n", s->Data[63]);
	}
	void MainThreadTest()
	{
		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(Omni::MemoryKind::UserDefault);
		size_t testSize = 1024;
		std::byte* p = alloc.allocate(testSize);
		memset(p, 0, testSize);
		alloc.deallocate(p, testSize);

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
		if constexpr (false)
		{
			int x = 98;
			DispatchWorkItem& item = DispatchWorkItem::Create(Func0, &x);
			x = 97;
			item.Perform();
			item.Release();
		}
		{
			MemoryArenaScope s = stack.PushScope();
			SuperStruct* ss = (SuperStruct*)stack.Allocate(sizeof(SuperStruct));
			ss->Data[63] = 0x8866;

			DispatchWorkItem& item = DispatchWorkItem::Create(Func1, ss);
			ss->Data[63] = 0;
			item.Perform();
			item.Release();
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
	system.InitializeAndJoin(ARRAY_LENGTH(engineArgv), engineArgv);

	Omni::MainThreadTest();

	system.TriggerFinalization();
	system.Finalize();
	system.DestroySystem();
 
	return 0;
}

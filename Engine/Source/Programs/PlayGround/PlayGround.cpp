#include "Runtime/System/System.h"
#include "Runtime/Misc/ArrayUtils.h"
#include "Runtime/Misc/AssertUtils.h"
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Memory/MemoryArena.h"


namespace Omni
{
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

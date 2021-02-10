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

		MemoryArena& arena = MemoryModule::Get().GetThreadArena();
		arena.Push();
		{
			MemoryArenaScope s0 = arena.PushScope();
			void* f = nullptr;
			{
				MemoryArenaScope s1 = arena.PushScope();
				f = arena.Allocate(3);
				constexpr u32 allocSizes[] = { 1, 2, 4, 8, 12, 17, 19, 354 };
				for (u32 sz : allocSizes)
				{
					arena.Allocate(sz);
				}
				void* m = nullptr;
				{
					MemoryArenaScope s2 = arena.PushScope();
					m = arena.Allocate(1);
					for (u32 sz : allocSizes)
					{
						arena.Allocate(sz);
					}
				}
				void* m1 = arena.Allocate(2);
				CheckAlways(m1 == m);
			}
			void* f1 = arena.Allocate(1);
			CheckAlways(f1 == f);
		}
		arena.Pop();
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

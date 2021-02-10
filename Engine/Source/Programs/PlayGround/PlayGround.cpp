#include "Runtime/System/System.h"
#include "Runtime/Misc/ArrayUtils.h"
#include "Runtime/Memory/MemoryModule.h"

int main(int, const char** )
{
	Omni::System::CreateSystem();
	Omni::System& system = Omni::System::GetSystem();
	const char* engineArgv[] =
	{
		"",
	};
	system.InitializeAndJoin(ARRAY_LENGTH(engineArgv), engineArgv);

	Omni::PMRAllocator alloc = Omni::MemoryModule::Get().GetPMRAllocator(Omni::MemoryKind::UserDefault);
	size_t testSize = 1024;
	std::byte* p = alloc.allocate(testSize);
	memset(p, 0, testSize);
	alloc.deallocate(p, testSize);

	system.TriggerFinalization();
	system.Finalize();
	system.DestroySystem();
 
	return 0;
}

#include "Runtime/System/System.h"
#include "Programs/PlayGround/PlayGroundTests.h"
#include "Runtime/Concurrency/ConcurrencyModule.h"
#include "Runtime/Concurrency/JobPrimitives.h"
#include "Runtime/Concurrency/LockfreeContainer.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Concurrency/ThreadUtils.h"
#include "Runtime/Memory/MemoryArena.h"
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Misc/ArrayUtils.h"
#include "Runtime/Platform/PlatformAPIs.h"
#include "Runtime/Test/AssertUtils.h"
#include "Runtime/Test/PerfUtils.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>

#include <thread>
#include <unordered_set>


namespace Omni
{
	void MainThreadTest()
	{
		TestLockfreeStackSingleThread();
		TestLockfreeQueueSingleThread();
		TestPMRAllocate();
		TestScratchStack();
		TestSpinLock();
		TestMultiThreadAllocation();
		TestDispatchQueue();
		TestAsync();
		System::GetSystem().TriggerFinalization(true);
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

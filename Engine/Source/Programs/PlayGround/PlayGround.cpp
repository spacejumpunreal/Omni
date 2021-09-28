#include "PlayGroundPCH.h"
#include "PlayGroundExperiment.h"
#include "PlayGroundTests.h"
#include "Concurrency/ConcurrencyModule.h"
#include "Concurrency/JobPrimitives.h"
#include "MultiThread/LockfreeContainer.h"
#include "MultiThread/SpinLock.h"
#include "Concurrency/ThreadUtils.h"
#include "Memory/MemoryArena.h"
#include "Allocator/MemoryModule.h"
#include "Misc/ArrayUtils.h"
#include "Misc/PlatformAPIs.h"
#include "Platform/InputDefs.h"
#include "Platform/InputModule.h"
#include "Platform/KeyMap.h"
#include "System/System.h"
#include "Misc/AssertUtils.h"
#include "Misc/PerfUtils.h"

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
		struct KeyStateLogger : public KeyStateListener
		{
			void OnKeyEvent(KeyCode key, bool pressed) override
			{
				printf("Key[%x] is %s\n", key, pressed ? "down" : "up");
				count++;
				if (count > 40)
				{
					InputModule::Get().UnRegisterlistener(key, this);
					delete this;
				}
			}
			int count = 0;
		};

		if constexpr (false)
		{
			auto pleft = new KeyStateLogger();
			auto pright = new KeyStateLogger();
			InputModule::Get().RegisterListener(KeyMap::MouseLeft, pleft);
			InputModule::Get().RegisterListener(KeyMap::MouseRight, pright);
		}
		
		//TestAll();
		//ExperimentAll();
		//System::GetSystem().TriggerFinalization(true);
	}
}

int main(int, const char** )
{
	Omni::System::CreateSystem();
	Omni::System& system = Omni::System::GetSystem();
	const char* engineArgv[] =
	{
		"LoadModule=Window",
		"--window-size=1920x1080",
	};
	system.InitializeAndJoin(ARRAY_LENGTH(engineArgv), engineArgv, Omni::MainThreadTest);
	system.DestroySystem();
 
	return 0;
}

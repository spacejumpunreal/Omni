#include "Programs/PlayGround/PlayGroundPCH.h"
#include "Programs/PlayGround/PlayGroundExperiment.h"
#include "Programs/PlayGround/PlayGroundTests.h"
#include "Runtime/Core/Concurrency/ConcurrencyModule.h"
#include "Runtime/Core/Concurrency/JobPrimitives.h"
#include "Runtime/Base/MultiThread/LockfreeContainer.h"
#include "Runtime/Base/MultiThread/SpinLock.h"
#include "Runtime/Core/Concurrency/ThreadUtils.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Misc/PlatformAPIs.h"
#include "Runtime/Core/Platform/InputDefs.h"
#include "Runtime/Core/Platform/InputModule.h"
#include "Runtime/Core/Platform/KeyMap.h"
#include "Runtime/Core/System/System.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PerfUtils.h"

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

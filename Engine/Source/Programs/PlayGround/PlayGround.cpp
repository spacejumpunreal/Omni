#include "Programs/PlayGround/PlayGroundPCH.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PerfUtils.h"
#include "Runtime/Base/Misc/PlatformAPIs.h"
#include "Runtime/Base/MultiThread/LockfreeContainer.h"
#include "Runtime/Base/MultiThread/SpinLock.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Concurrency/ConcurrencyModule.h"
#include "Runtime/Core/Concurrency/JobPrimitives.h"
#include "Runtime/Core/Concurrency/ThreadUtils.h"
#include "Runtime/Core/Platform/InputDefs.h"
#include "Runtime/Core/Platform/InputModule.h"
#include "Runtime/Core/Platform/KeyMap.h"
#include "Runtime/Core/System/System.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_set>


namespace Omni
{
	void PlayGroundCode()
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
	}
}

int main(int, const char** )
{
	Omni::System::CreateSystem();
	Omni::System& system = Omni::System::GetSystem();
	const char* engineArgv[] =
	{
		"LoadModule=Window",
		"LoadModule=GfxApi",
		"LoadModule=DemoRenderer",
		"--window-size=1920x1080",
	};
	system.InitializeAndJoin(ARRAY_LENGTH(engineArgv), engineArgv, Omni::PlayGroundCode);
	system.DestroySystem();
 
	return 0;
}

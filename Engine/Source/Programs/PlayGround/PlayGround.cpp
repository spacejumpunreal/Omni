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
#include "Runtime/Engine/Engine.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_set>

namespace Omni
{
struct KeyStateLogger;
KeyStateLogger* pleft;

struct KeyStateLogger : public KeyStateListener
{
    void OnKeyEvent(KeyCode key, bool pressed) override
    {
        (void)pressed;
        count++;
        if (count > 10)
        {
            InputModule::Get().UnRegisterlistener(key, this);
            delete this;
            pleft = nullptr;
            System::GetSystem().TriggerFinalization(true);
        }
    }
    int count = 0;
};

void PlayGroundReady()
{
    pleft = new KeyStateLogger();
    InputModule::Get().RegisterListener(KeyMap::MouseLeft, pleft);
}
void PlayGroundWillQuit()
{
    if (pleft != nullptr)
    {
        InputModule::Get().UnRegisterlistener(KeyMap::MouseLeft, pleft);
        delete pleft;
        pleft = nullptr;
    }
}

void PlayGroundCodeEmpty()
{
    System::GetSystem().TriggerFinalization(true);
}
} // namespace Omni

int main(int, const char**)
{
    Omni::CreateEngineSystem();
    Omni::System& system = Omni::System::GetSystem();
#if true
    const char* engineArgv[] = {
        "LoadModule=Window",
        "LoadModule=DX12",
        "LoadModule=DemoRenderer",
        "--window-size=800x600",
        "--file-project-root=C:/checkout/Omni/TestProject",
    };
    system.InitializeAndJoin(ARRAY_LENGTH(engineArgv), engineArgv, Omni::PlayGroundReady, Omni::PlayGroundWillQuit);
#else
    const char* engineArgv[] = {
        "LoadModule=Window",
        //"LoadModule=GfxApi",
        //"LoadModule=DemoRenderer",
        "--window-size=1920x1080",
    };
    system.InitializeAndJoin(ARRAY_LENGTH(engineArgv), engineArgv, Omni::PlayGroundCodeEmpty, nullptr);
#endif
    system.DestroySystem();

    return 0;
}

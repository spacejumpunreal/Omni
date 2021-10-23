#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/System/System.h"
#include <functional>

namespace Omni
{
    class ThreadData;

    using ThreadId = u32; //this id is used to mark ThreadData before thread is created(thread::id not known yet)
    using TThreadBody = std::function<void()>;

    static constexpr ThreadId InvalidThreadId = (ThreadId)-1;
    static constexpr ThreadId MainThreadId = 0;
    static constexpr ThreadId RenderThreadId = 1;
    static constexpr ThreadId WindowThreadId = 2;
    static constexpr ThreadId WorkerThreadBaseId = 3;
    static constexpr ThreadId DynamicThreadBaseId = 1024;

    class CORE_API ThreadData
    {
    public:
        //for Engine
        static ThreadData& Create(ThreadId id);
        void InitAsMainOnMain();
        void RunAndFinalizeAsMain(SystemInitializedCallback cb);
        void LauchAsWorkerOnMain(const TThreadBody& body);
        void JoinAndDestroyOnMain();
        //for module/user
        static ThreadData& GetThisThreadData();
        bool IsAskedToQuit();
        static void MarkQuitWork();
        bool IsSelfThread();
        ThreadId GetThreadId();

    protected:
        ThreadData() = default;
    };

    CORE_API bool IsOnMainThread();
    CORE_API void RegisterMainThread();
    CORE_API void UnregisterMainThread();
}

/* sequence
* @MainThread
*   - init modules
*       - init in concurrent module
*           - alloc ThreadData, everything default initialized
*           - stored the pointer
*           - launch as worker
*   - after all modules initialized
*       - may work in a UI loop
*       - jumped out of loop
* 
*   - finalize other modules
*   - finalize concurrency module
*       - mark all worker threads destroy
*       - finalize main(wont this be a problem if main or other threads still use some thread utils when main is already finalized?)
*       - join and delete other threads
*   - done
* 
* @WorkerThread
*   - std::thread created
*   - thread running
*   - ThreadInitialize
*
*   - work
* 
*   - ThreadFinalize
*   - return
*   - MainThread call std::~thread
* 
* quit sequence
* - some thread called System::TriggerFinalization
* - FrameTrigger logic stop, free main thread from ui loop
* - main thread continue with finalization
*/


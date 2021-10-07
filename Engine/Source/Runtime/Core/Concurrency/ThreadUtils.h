#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/System/System.h"

namespace Omni
{
    using ThreadIndex = i16;

    static constexpr ThreadIndex MainThreadIndex = 0;
    static constexpr ThreadIndex InvalidThreadIndex = -1;

    class CORE_API ThreadData
    {
    public:
        //for Engine
        static ThreadData& Create();
        void InitAsMainOnMain();
        void RunAndFinalizeOnMain(SystemInitializedCallback cb);
        void LauchAsWorkerOnMain();
        void JoinAndDestroyOnMain();
        //for module/user
        static ThreadData& GetThisThreadData();
        bool IsAskedToQuit();
        static void MarkQuitWork();
        bool IsSelfThread();
        ThreadIndex GetThreadIndex();

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


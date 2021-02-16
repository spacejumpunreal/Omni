#pragma once
#include "Runtime/Omni.h"
#include "Runtime/System/System.h"

namespace Omni
{
    using ThreadIndex = i16;

    static constexpr ThreadIndex MainThreadIndex = 0;
    static constexpr ThreadIndex InvalidThreadIndex = -1;

    class ThreadData
    {
    public:
        //for ConcurrencyModule
        static ThreadData& Create();
        
        
        void InitAsMainOnMain();
        void RunAndFinalizeOnMain(SystemInitializedCallback cb);
        void LauchAsWorkerOnMain();
        void JoinAndDestroyOnMain();
        //for thread
        static ThreadData& GetThisThreadData();
        bool IsAskedToQuit();
        static void MarkQuitWork();
        bool IsOnSelfThread();
        ThreadIndex GetThreadIndex();

    protected:
        ThreadData() = default;
    };

    bool IsOnMainThread();
    void RegisterMainThread();
    void UnregisterMainThread();
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
*       - finalize destroy main
*       - join and delete other threads
*   - done
* 
* @WorkerThread
*   - take id
*   - ThreadInitialize
*
*   - work
* 
*   - ThreadFinalize
*   - return
* 
* 
* quit sequence
* - some thread called System::TriggerFinalization
* - FrameTrigger logic stop, free main thread from ui loop
* - main thread continue with finalization
*/
#include "Programs/PlayGround/PlayGroundPCH.h"
#include "Programs/PlayGround/PlayGroundExperiment.h"
#include "Runtime/Base/MultiThread/LockfreeContainer.h"
#include "Runtime/Core/Concurrency/LockfreeNodeCache.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Misc/PlatformAPIs.h"


#include <atomic>
#include <thread>
#include <chrono>

namespace Omni
{
    void ExperimentAll()
    {
        //Exp2ThreadsPingpong();
        ExpLockfreeVSSingle();
    }

    struct CacheAlignedCounter
    {
        static constexpr int CounterSize = sizeof(std::atomic<size_t>);
        
        CacheAlignedCounter()
            : Count(0)
        {}

        std::atomic<size_t> Count;
        char Padding[256 - CounterSize];
    };

    template<size_t TestCount>
    struct Exp2ThreadsPassToken
    {
    public:
        Exp2ThreadsPassToken(size_t threadCount)
            : mToken(0)
            , mThreadCount(threadCount)
            , mDataPtr(nullptr)
        {
            //mDataPtr = &mData0;
            mDataPtr = new CacheAlignedCounter;
        }
        ~Exp2ThreadsPassToken()
        {
            if (mDataPtr != &mData0)
                delete mDataPtr;
        }
        void Run(u64 threadIndex)
        {
            u64 nextThreadIndex = (threadIndex + 1) % mThreadCount;
            while (true)
            {
                PauseThread();
                if (mToken.load(std::memory_order_acquire) != threadIndex)
                {
                    
                    continue;
                }
                size_t prevValue = mDataPtr->Count.fetch_add(1, std::memory_order_relaxed);
                mToken.store(nextThreadIndex, std::memory_order_release);
                if (prevValue >= TestCount)
                    return;
            }
        }
        static void RunStatic(Exp2ThreadsPassToken* d, u64 i)
        {
            d->Run(i);
        }
    private:
        std::atomic<u64>        mToken;
        size_t                  mThreadCount;
        CacheAlignedCounter     mData0;
        CacheAlignedCounter*    mDataPtr;
    };

    void Exp2ThreadsPingpong()
    {
        constexpr size_t TestCount = 0x7ffffff;
        size_t nThreads = std::thread::hardware_concurrency() / 4;
        nThreads = 8;
        Exp2ThreadsPassToken<TestCount> test(nThreads);
        PMRVector<std::thread> threads(MemoryModule::Get().GetPMRAllocator(MemoryKind::UserDefault));

        auto tBegin = std::chrono::steady_clock::now();
        for (size_t i = 0; i < nThreads; ++i)
        {
            threads.push_back(std::thread(&Exp2ThreadsPassToken<TestCount>::RunStatic, &test, (u64)i));
        }
        for (size_t i = 0; i < nThreads; ++i)
        {
            threads[i].join();
        }
        auto tEnd = std::chrono::steady_clock::now();
        std::chrono::duration<double> durtion = tEnd - tBegin;

        printf("\nExp2ThreadsPingpong(%d threads) duration:%fs\n", (int)nThreads, durtion.count());
    }

    template<size_t TestCount>
    struct ExpLockfreeStack
    {
    public:
        ExpLockfreeStack(size_t)
        {
            LockfreeNode* n = LockfreeNodeCache::Alloc();
            n->Data[0] = 0;
            mStack.Push(n);
        }
        ~ExpLockfreeStack()
        {
            LockfreeNode* n = mStack.Pop();
            LockfreeNodeCache::Free(n);
        }
        void Run(u64)
        {
            while (true)
            {
                auto n = mStack.Pop();
                if (!n)
                {
                    PauseThread();
                    continue;
                }
                size_t& r = *(size_t*)&(n->Data[0]);
                ++r;
                size_t x = r;
                mStack.Push(n);
                if (x >= TestCount)
                    break;
            }
        }
        static void RunStatic(ExpLockfreeStack* d, u64 i)
        {
            d->Run(i);
        }
    private:
        LockfreeStack mStack;
    };

    void ExpLockfreeVSSingle()
    {
        constexpr size_t TestCount = 0x7ffffff;
        size_t nThreads = std::thread::hardware_concurrency() / 4;
        nThreads = 4;
        ExpLockfreeStack<TestCount> test(nThreads);
        PMRVector<std::thread> threads(MemoryModule::Get().GetPMRAllocator(MemoryKind::UserDefault));

        auto tBegin = std::chrono::steady_clock::now();
        for (size_t i = 0; i < nThreads; ++i)
        {
            threads.push_back(std::thread(&ExpLockfreeStack<TestCount>::RunStatic, &test, (u64)i));
        }
        for (size_t i = 0; i < nThreads; ++i)
        {
            threads[i].join();
        }
        auto tEnd = std::chrono::steady_clock::now();
        std::chrono::duration<double> durtion = tEnd - tBegin;

        printf("\nExpLockfreeVSSingle(%d threads) duration:%fs\n", (int)nThreads, durtion.count());
    }
}

#include "Programs/Experiments/MultiThreadEfficiency/MultiThreadEfficiency.h"
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
#include "Runtime/Core/Concurrency/LockfreeNodeCache.h"
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
    struct CacheAlignedCounter
    {
    public:
        static constexpr int CounterSize = sizeof(std::atomic<size_t>);
        std::atomic<size_t> Count;
        char Padding[256 - CounterSize];
    public:
        CacheAlignedCounter()
            : Count(0)
        {}

    };

    template<size_t TestCount>
    struct Exp2ThreadsPassToken
    {
        /*
            test:
                N threads uses a token to control who has access to a global shared counter,
                each thread read the token to know if it has access,
                if has access, increment the counter, and then modify token to pass access to other thread

        */
    private:
        std::atomic<u64>        mToken;
        size_t                  mThreadCount;
        CacheAlignedCounter     mData0;
        CacheAlignedCounter* mDataPtr;
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
    {/*
        test:
           N threads sharing a LockfreeStack which contains a single node,
           the element has a pointer to a global counter,
           each thread try to pop this node,
           if get it, increment the global counter and then push node back,
           quit until counter reached speicific value
        objective:
            see how slow can multithread be compared with single thread
        */
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
    }

	void PlayGroundCode()
	{
        Exp2ThreadsPingpong();
        ExpLockfreeVSSingle();
	}
}

int main(int, const char**)
{
	Omni::System::CreateSystem();
	Omni::System& system = Omni::System::GetSystem();
	system.InitializeAndJoin(0, nullptr, Omni::PlayGroundCode, nullptr);
	system.DestroySystem();

	return 0;
}

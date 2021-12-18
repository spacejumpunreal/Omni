#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Prelude/PlatformDefs.h"
#include "Runtime/Prelude/SuppressWarning.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Container/LinkedList.h"
#include "Runtime/Base/Memory/MemoryDefs.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/Padding.h"
#include "Runtime/Base/Misc/TimeTypes.h"
#include "Runtime/Base/MultiThread/SpinLock.h"
#include <condition_variable>
#include <mutex>
#include <atomic>


namespace Omni
{
    struct SListNode;
    
    OMNI_PUSH_WARNING()
    OMNI_SUPPRESS_WARNING_PADDED_DUE_TO_ALIGNMENT()

    class alignas(CPU_CACHE_LINE_SIZE) LockQueue
    {
    public:
        LockQueue()
            : mHead(nullptr)
            , mTail(nullptr)
            , mTodoCount(0)
            , mWaitCount(0)
            , mAllWakeupLimit(0)
        {}
        LockQueue(const LockQueue&) = delete;

        void Enqueue(SListNode* head)
        {
            SListNode* tail;
            u32 inc = 0;
            {
                SListNode* p = head;
                SListNode* last = nullptr;
                while (p != nullptr)
                {
                    ++inc;
                    last = p;
                    p = p->Next;
                }
                tail = last;
            }

            mLock.lock();
            mTodoCount += inc;
            if (mTail)
            {
                mTail->Next = head;
                mTail = tail;
            }
            else
            {
                CheckDebug(mHead == nullptr);
                mHead = head;
                mTail = tail;
            }
            u32 tc = mTodoCount;
            u32 wc = mWaitCount;
            mLock.unlock();

            u32 ac = tc < wc ? tc : wc;
            if (ac > 0)
            {
                if (ac > mAllWakeupLimit)
                    mCV.notify_all();
                else
                    for (u32 i = 0; i < mAllWakeupLimit; ++i)
                        mCV.notify_one();
            }
        }

        template<typename T>
        T* Dequeue()
        {
            static_assert(std::is_base_of_v<SListNode, T>);
            SListNode* ret;
            {
                std::unique_lock<std::mutex> lk(mLock);
                while (mHead == nullptr)
                {
                    ++mWaitCount;
                    mCV.wait(lk);
                    --mWaitCount;
                }
                --mTodoCount;
                ret = mHead;
                mHead = mHead->Next;
                if (mHead == nullptr)
                    mTail = nullptr;
            }
            return static_cast<T*>(ret);
        }

        template<typename T>
        T* TryDequeue()
        {
            static_assert(std::is_base_of_v<SListNode, T>);
            SListNode* ret;
            {
                std::unique_lock<std::mutex> lk(mLock);
                if (mHead == nullptr)
                    return nullptr;
                --mTodoCount;
                ret = mHead;
                mHead = mHead->Next;
                if (mHead == nullptr)
                    mTail = nullptr;
            }
            return static_cast<T*>(ret);
        }
        
        template<typename T>
        T* TryDequeueWithTimeout(TimePoint deadline)
        {
            static_assert(std::is_base_of_v<SListNode, T>);
            SListNode* ret;
            {
                std::unique_lock<std::mutex> lk(mLock);
                bool timeout = false;
                while (mHead == nullptr && !timeout)
                {
                    ++mWaitCount;
                    std::cv_status state = mCV.wait_until(lk, deadline);
                    timeout = std::cv_status::timeout == state;
                    --mWaitCount;
                }
                if (mHead == nullptr)
                    return nullptr;
                --mTodoCount;
                ret = mHead;
                mHead = mHead->Next;
                ret->Next = nullptr;
                if (mHead == nullptr)
                    mTail = nullptr;
            }
            return static_cast<T*>(ret);
        }


        u32 Size()
        {
            bool ret;
            mLock.lock();
            ret = mTodoCount;
            mLock.unlock();
            return ret;
        }

        void SetAllWakeupLimit(u32 limit) 
        {
            mAllWakeupLimit = limit;
        }

    private:
        std::mutex                  mLock;
        std::condition_variable     mCV;
        SListNode*                  mHead;
        SListNode*                  mTail;
        u32                         mTodoCount;
        u32                         mWaitCount;
        u32                         mAllWakeupLimit;
    };

    OMNI_POP_WARNING()
}

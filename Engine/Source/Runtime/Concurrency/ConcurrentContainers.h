#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Misc/Padding.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Misc/LinkedListUtils.h"
#include "Runtime/Test/AssertUtils.h"
#include <condition_variable>
#include <mutex>

namespace Omni
{
    struct SListNode;
    //can't wrap sync around Queue, Dequeue need to block on empty
    class ConcurrentQueue
    {
    public:
        ConcurrentQueue()
            : mHead(nullptr)
            , mTail(nullptr)
            , mTodoCount(0)
            , mWaitCount(0)
            , mAllWakeupLimit(0)
        {}
        void Enqueue(SListNode* head, SListNode* tail)
        {
            u32 inc = 0;
            {
                SListNode* p = head;
                while (p != nullptr)
                {
                    ++inc;
                    p = p->Next;
                }
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
                CheckAlways(mHead == nullptr);
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
}

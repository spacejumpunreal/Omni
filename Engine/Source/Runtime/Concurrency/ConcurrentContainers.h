#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Misc/Padding.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Misc/LinkedListUtils.h"
#include "Runtime/Misc/AssertUtils.h"
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
            : mWaitCount(0)
            , mHead(nullptr)
            , mTail(nullptr)
        {}
        void Enqueue(SListNode* head, SListNode* tail)
        {
            CheckAlways(tail->Next == nullptr);
            mLock.lock();
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
            size_t waitCount = mWaitCount;
            mLock.unlock();
            if (waitCount == 1)
                mCV.notify_one();
            else if (waitCount > 0)
                mCV.notify_all();
        }
        template<typename T>
        T* Dequeue()
        {
            static_assert(std::is_base_of_v<SListNode, T>);
            SListNode* ret;
            {
                std::unique_lock<std::mutex> lk(mLock);
                ++mWaitCount;
                while (mHead == nullptr)
                    mCV.wait(lk);
                --mWaitCount;
                ret = mHead;
                mHead = mHead->Next;
                if (mHead == nullptr)
                    mTail = nullptr;
            }
            return static_cast<T*>(ret);
        }
    private:
        std::mutex                  mLock;
        std::condition_variable     mCV;
        size_t                      mWaitCount;
        SListNode*                  mHead;
        SListNode*                  mTail;
    };
}

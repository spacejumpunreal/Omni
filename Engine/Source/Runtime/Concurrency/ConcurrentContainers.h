#pragma once
#if false
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
    class ConcurrentQueue
    {
    public:
        ConcurrentQueue()
            : mCount(0)
            , mHead(nullptr)
            , mTail(nullptr)
        {}
        void Enqueue(SListNode* head, SListNode* tail)
        {
            n->Next = nullptr;
            int nCount;
            mLock.Data.Lock();
            if (mTail)
            {
                SListNode* o = mTail;
                mTail = n;
            }
            else
            {
                CheckAlways(mHead == nullptr);
                mHead = mTail = n;
            }
            ++mCount;
            nCount = mCount;
            mLock.Data.Unlock();
            if (nCount > 0)
                ???
        }
        bool TryDequeue(SListNode*& outNode)
        {
            bool ret = false;
            mLock.Data.Lock();
            --mCount;
            if (mHead)
            {
                outNode = mHead;
                mHead = mHead->Next;
                if (mHead == nullptr)
                    mTail = nullptr;
            }
            mLock.Data.Unlock();
            return ret;
        }
    private:
        CacheAlign<SpinLock>        mLock;
        std::mutex                  mSlowLock;
        std::condition_variable     mCV;
        std::atomic<size_t>         mPendingThreads;
        size_t                      mCount;
        SListNode*                  mHead;
        SListNode*                  mTail;
    };
}
#endif
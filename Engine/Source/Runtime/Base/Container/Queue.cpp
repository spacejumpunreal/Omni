#include "Queue.h"
#include "LinkedList.h"

namespace Omni
{
    Queue::Queue()
        : mHead(nullptr)
        , mTail(nullptr)
    {
    }
    void Queue::Enqueue(SListNode* head, SListNode* tail)
    {
        tail->Next = nullptr;
        if (mTail)
        {
            mTail->Next = head;
            mTail = tail;
        }
        else
        {
            mHead = head;
            mTail = tail;
        }
    }
    SListNode* Queue::Dequeue()
    {
        SListNode* ret = mHead;
        mHead = mHead->Next;
        if (mHead == nullptr)
            mTail = nullptr;
        return ret;
    }
    SListNode* Queue::DequeueAll()
    {
        SListNode* ret = mHead;
        mHead = mTail = nullptr;
        return ret;
    }
    bool Queue::IsEmpty()
    {
        return mHead == nullptr;
    }
}


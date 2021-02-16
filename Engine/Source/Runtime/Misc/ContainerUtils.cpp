#include "Runtime/Misc/ContainerUtils.h"
#include "Runtime/Misc/LinkedListUtils.h"

namespace Omni
{
    Queue::Queue()
        : mSize(0)
        , mHead(nullptr)
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
    size_t Queue::GetSize()
    {
        return mSize;
    }
}
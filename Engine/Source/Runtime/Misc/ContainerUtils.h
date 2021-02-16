#pragma once
#include "Runtime/Omni.h"

namespace Omni
{
    struct SListNode;

    class Queue
    {
    public:
        Queue();
        void Enqueue(SListNode* head, SListNode* tail);
        SListNode* Dequeue();
        FORCEINLINE size_t GetSize();
    private:
        size_t      mSize;
        SListNode*  mHead;
        SListNode*  mTail;
    };
}
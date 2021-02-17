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
        SListNode* DequeueAll();
        bool IsEmpty();
    private:
        SListNode*  mHead;
        SListNode*  mTail;
    };
}
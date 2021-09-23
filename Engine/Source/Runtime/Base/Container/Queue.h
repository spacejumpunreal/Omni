#pragma once
#include "BaseAPI.h"

namespace Omni
{
    struct SListNode;

    class BASE_API Queue
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


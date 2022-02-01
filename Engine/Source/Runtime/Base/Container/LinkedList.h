#pragma once
#include "Runtime/Base/BaseAPI.h"

namespace Omni
{
    struct SListNode
    {
    public:
        SListNode(SListNode* next = nullptr)
            : Next(next)
        {}
    public:
        SListNode*      Next;
    };
}


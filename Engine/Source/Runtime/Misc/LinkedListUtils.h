#pragma once
#include "Runtime/Omni.h"

namespace Omni
{
    struct SListNode
    {
    public:
        SListNode(SListNode* next)
            : Next(next)
        {}
    public:
        SListNode*      Next;
    };
}


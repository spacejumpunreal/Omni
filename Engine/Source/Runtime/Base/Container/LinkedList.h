#pragma once

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


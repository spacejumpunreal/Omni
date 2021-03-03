#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Platform/PlatformDefs.h"
#include <atomic>

namespace Omni
{
    struct LockfreeNode
    {
        static constexpr u32    MaxDataSlots = 7;

        LockfreeNode*   Next;
        void*           Data[MaxDataSlots];
    };
    static_assert(sizeof(LockfreeNode) <= CPU_CACHE_LINE_SIZE);
    

    struct LockfreeNodeCache
    {
    public:
        static void GlobalInitialize();
        static void GlobalFinalize();
        static void ThreadInitialize();
        static void ThreadFinalize();
        static LockfreeNode* Alloc();
        static void Free(LockfreeNode* node);
    };

    struct alignas(sizeof(u64) * 2) TaggedPointer
    {
        u64                 Tag;
        LockfreeNode*       Ptr;
    };

    class LockfreeStack
    {
    public:
        LockfreeStack();
        ~LockfreeStack();
        void Push(LockfreeNode* node);
        LockfreeNode* Pop();
    private:
        TaggedPointer       mHead;
    };

    template<u32 NodeDataCount>
    class LockfreeQueue
    {
    public:
        LockfreeQueue();
        ~LockfreeQueue();
        void Enqueue(LockfreeNode* first, LockfreeNode* last);
        void Enqueue(LockfreeNode* node) { Enqueue(node, node); }
        LockfreeNode* Dequeue();
    private:
        TaggedPointer      mHead;
        LockfreeNode*      mTail;
    };
}
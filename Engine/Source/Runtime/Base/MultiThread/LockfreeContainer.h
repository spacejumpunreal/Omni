#pragma once
#include "Omni.h"
#include "PlatformDefs.h"
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

    struct alignas(sizeof(u64) * 2) TaggedPointer
    {
#if OMNI_WINDOWS
        u64                 Tag;
#endif
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

    class LockfreeQueueBase
    {
    public:
        void Enqueue(LockfreeNode* first, LockfreeNode* last);
        void Enqueue(LockfreeNode* node) { Enqueue(node, node); }
        LockfreeNode* Dequeue();
    protected:
        void CheckOnDestroy();
    protected:
        TaggedPointer       mHead;
        LockfreeNode*       mTail;
        u32                 mNodeDataCount;
    };

    template<typename TNodeCache>
    class LockfreeQueue :LockfreeQueueBase
    {
    public:
        LockfreeQueue()
        {
#if OMNI_WINDOWS
            mHead.Tag = 0;
#endif
            mTail = mHead.Ptr = TNodeCache::Alloc();
            mHead.Ptr->Next = nullptr;
        }
        ~LockfreeQueue()
        {
            CheckOnDestroy();
            TNodeCache::Free(mHead.Ptr);
        }
    };
}

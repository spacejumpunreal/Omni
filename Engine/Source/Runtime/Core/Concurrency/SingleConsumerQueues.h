#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Prelude/PlatformDefs.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include <atomic>

namespace Omni
{
    template<typename T, bool MultiProducer>
    struct SCQueue
    {
    public:
        struct QueueNode;
        SCQueue()
            : mAllocator(MemoryModule::Get().GetPMRAllocatorT<QueueNode>())
        {
            mHead = mTail = mAllocator.allocate(1);
        }
        ~SCQueue()
        {
            QueueNode* p = mHead;
            while (p != nullptr)
            {
                QueueNode* np = p->Next;
                mAllocator.deallocate(p, 1);
                p = np;
            }
        }
        void Push(const T& data)
        {//push next to tail
            QueueNode* newNode = mAllocator.allocate(1);
            newNode->Data = data;
            newNode->Next.store(nullptr, std::memory_order_relaxed);
            
            if constexpr (MultiProducer)
            {
                QueueNode* prevHead = mHead.exchange(newNode, std::memory_order_acquire); //acquire guarantee: write to prevHead->Next as nullptr will be seen here
                prevHead->Next.store(newNode, std::memory_order_release); //release guarantee:conumser(who called pop) will see newNode->Data when it gets pointer newNode
            }
            else
            {
                QueueNode* prevHead = mHead.load(std::memory_order_relaxed);
                prevHead->Next.store(newNode, std::memory_order_release);
                mHead.store(newNode, std::memory_order_relaxed);
            }
        }
        bool TryPop(T& data)
        {//pop from head
            QueueNode* next = mTail.load(std::memory_order_relaxed)->Next.load(std::memory_order_relaxed);
            if (next == nullptr)
                return false;
            mTail = next;
            data = next->Data;
        }

    private:
        using AtomicLink = std::atomic<QueueNode*>;
        struct alignas(CPU_CACHE_LINE_SIZE) QueueNode
        {
            AtomicLink  Next;
            T           Data;
        };

        AtomicLink                  mHead;
        AtomicLink                  mTail;
        PMRAllocatorT<QueueNode>    mAllocator;
    };
}
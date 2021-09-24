#include "BasePCH.h"
#include "MultiThread/LockfreeContainer.h"
#include "Math/CompileTime.h"
#include "Misc/AssertUtils.h"

namespace Omni
{

    LockfreeStack::LockfreeStack()
    {
#if OMNI_WINDOWS
        mHead.Tag = 0;
#endif
        mHead.Ptr = nullptr;
    }
    LockfreeStack::~LockfreeStack()
    {
        CheckDebug(mHead.Ptr == nullptr);
    }

    void LockfreeStack::Push(LockfreeNode* node)
    {
#if OMNI_WINDOWS
        TaggedPointer oldHead;
        do
        {
            oldHead.Tag = mHead.Tag;
            oldHead.Ptr = mHead.Ptr;
            node->Next = oldHead.Ptr;
        } while (_InterlockedCompareExchange128((volatile long long*)&mHead, (long long)node, (long long)oldHead.Tag + 1, (long long*)&oldHead) == 0);
#elif OMNI_IOS
        long ok;
        LockfreeNode* oldHead;
        __asm__ __volatile__
        (
         "0:\n\t"
         "ldxr %[oldHead], %[Head]\n\t"
         "str %[oldHead], [%[node]]\n\t"
         "stlxr %w[ok], %[node], %[Head]\n\t"
         "cbnz %[ok], 0b\n\t"
         : [Head]"+m"(mHead), [oldHead]"=&r"(oldHead), [ok]"=&r"(ok)
         : [node]"r"(node)
         : "cc", "memory"
        );
#endif
    }
    LockfreeNode* LockfreeStack::Pop()
    {
#if OMNI_WINDOWS
        TaggedPointer oldHead;
        do
        {
            oldHead.Tag = mHead.Tag;
            oldHead.Ptr = mHead.Ptr;
            if (oldHead.Ptr == nullptr)
                return nullptr;
        } while (_InterlockedCompareExchange128((volatile long long*)&mHead, (long long)(oldHead.Ptr->Next), (long long)oldHead.Tag + 1, (long long*)&oldHead) == 0);
        oldHead.Ptr->Next = nullptr;
        return oldHead.Ptr;
#elif OMNI_IOS
        LockfreeNode* ret;
        LockfreeNode* next;
        long ok;
        __asm__ __volatile__
        (
        "0:\n\t"
        "ldaxr %[ret], %[Head]\n\t"
        "cbz %[ret], 1f\n\t"
        "ldr %[next], [%[ret]]\n\t"
        "stxr %w[ok], %[next], %[Head]\n\t"
        "cbnz %[ok], 0b\n\t"
        "1:\n\t"
        : [ret]"=&r"(ret), [next]"=&r"(next), [ok]"=&r"(ok),[Head]"+m"(mHead)
        :
        : "cc", "memory"
        );
        if (ret)
            ret->Next = nullptr;
        return ret;
#endif
    }
    void LockfreeQueueBase::Enqueue(LockfreeNode* first, LockfreeNode* last)
    {
        last->Next = nullptr;
#if OMNI_WINDOWS
        LockfreeNode* oldTail = std::atomic_exchange_explicit((std::atomic<LockfreeNode*>*)&mTail, last, std::memory_order_release);
        std::atomic_store_explicit((std::atomic<LockfreeNode*>*)&oldTail->Next, first, std::memory_order_release);
#elif OMNI_IOS
        long ok;
        LockfreeNode* oldTail;
        __asm__ __volatile__
        (
         "0:\n\t"
         "ldxr %[oldTail], %[Tail]\n\t"
         "stlxr %w[ok], %[t], %[Tail]\n\t"
         "cbnz %[ok], 0b\n\t"
         "stlr %[h], [%[oldTail]]\n\t"
         : [oldTail]"=&r"(oldTail), [Tail]"+m"(mTail), [ok]"=&r"(ok)
         : [t]"r"(last), [h]"r"(first)
         : "cc", "memory"
        );
#endif
    }
    LockfreeNode* LockfreeQueueBase::Dequeue()
    {
        void* tData[LockfreeNode::MaxDataSlots];
#if OMNI_WINDOWS
        TaggedPointer oldHead;
        LockfreeNode* next;
        
        oldHead.Tag = mHead.Tag;
        oldHead.Ptr = mHead.Ptr;
        do
        {
            next = std::atomic_load_explicit((std::atomic<LockfreeNode*>*)&(oldHead.Ptr->Next), std::memory_order_acquire);
            if (!next)
                return nullptr;
            for (u32 i = 0; i < mNodeDataCount; ++i)
                tData[i] = next->Data[i];
        } while (_InterlockedCompareExchange128((volatile long long*)&mHead, (long long)next, (long long)oldHead.Tag + 1, (long long*)&oldHead) == 0);
        for (u32 i = 0; i < mNodeDataCount; ++i)
            oldHead.Ptr->Data[i] = tData[i];
        oldHead.Ptr->Next = nullptr;
        return oldHead.Ptr;
#elif OMNI_IOS
        LockfreeNode* next;
        LockfreeNode* ret = nullptr;
        long cCount;
        LockfreeNode* psrc;
        LockfreeNode* pdst;
        void* td;
        
        __asm__ __volatile__
        (
         "0:\n\t"
         "ldaxr %[ret], %[Head]\n\t"
         "ldar %[next], [%[ret]]\n\t"
         "cbz %[next], 2f\n\t"
         "mov %[psrc], %[next]\n\t"
         "mov %[pdst], %[tData]\n\t"
         "mov %[cCount], %[mNodeDataCount]\n\t"
         "1:\n\t"
         "ldr %[td], [%[psrc], #8]!\n\t"
         "str %[td], [%[pdst]], #8\n\t"
         "sub %[cCount], %[cCount], #1\n\t"
         "cbnz %[cCount], 1b\n\t"
         "stxr %w[td], %[next], %[Head]\n\t"
         "cbnz %[td], 0b\n\t"
         "2:\n\t"
         : [next]"=&r"(next), [ret]"=&r"(ret), [cCount]"=&r"(cCount), [psrc]"=&r"(psrc), [pdst]"=&r"(pdst), [td]"=&r"(td), [Head]"+m"(mHead)
         : [mNodeDataCount]"i"(mNodeDataCount), [tData]"r"(tData)
         : "cc", "memory"
         );
        if (next)
        {
            ret->Next = nullptr;
            for (size_t i = 0; i < mNodeDataCount; ++i)
                ret->Data[i] = tData[i];
            return ret;
        }
        else
            return nullptr;
#endif		
    }

    void LockfreeQueueBase::CheckOnDestroy()
    {
        CheckAlways(mHead.Ptr->Next == nullptr);
    }
}

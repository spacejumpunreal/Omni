#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include <atomic>
#include <type_traits>
#include <cstdio>

namespace Omni
{
    class SharedObject
    {
    public:
        SharedObject() : mRefCount(1) {}
        virtual ~SharedObject() {}

        FORCEINLINE void Acquire()
        {
            mRefCount.fetch_add(1, std::memory_order_relaxed);
        }

        FORCEINLINE void Release()
        {
            u64 oldValue = mRefCount.fetch_sub(1, std::memory_order_acq_rel);
            if (oldValue == 1)
            {
                Destroy();
            }
        }

        /**
        * won't provide a basic implementation, 
        * because it's not just about calling dtor but also about freeing memory
        */
        virtual void Destroy() = 0;
    private:
        std::atomic<u64>    mRefCount;
    };

    template<typename T>
    class SharedPtr
    {
    public:
        FORCEINLINE SharedPtr()
            : mObject(nullptr)
        {
            //printf("%p.SharedPtr.ctor()\n", this);
        }
        template<typename U>
        FORCEINLINE SharedPtr(const SharedPtr<U>& other)
            : mObject(static_cast<T*>(other.GetRaw()))
        {//copy, ref+1
            mObject->Acquire(); 
            //printf("%p.SharedPtr<U>.ctor(%p)\n", (void*)this, (void*)other.GetRaw());
        }
        
        FORCEINLINE SharedPtr(T*& other)
            : mObject(other)
        {//transfer ownership, ref not changed
            //printf("%p.SharedPtr.ctor(%p)\n", (void*)this, (void*)other);
            other = nullptr;
        }

        FORCEINLINE ~SharedPtr()
        {
            //printf("%p.SharedPtr.dtor()\n", (void*)this);
            Clear();
        }
        FORCEINLINE SharedPtr& operator=(T*& other)
        {//new ref + 1, release old ownership
            other->Acquire();
            Clear();
            mObject = other;
            //printf("%p.SharedPtr::operator=(%p)\n", (void*)this, (void*)other.GetRaw());
            other = nullptr;
            return *this;
        }
        FORCEINLINE SharedPtr& operator=(SharedPtr&& other)
        {//release old ownership, transfer ownership
            Clear();
            mObject = other.mObject;
            other.mObject = nullptr;
            return *this;
        }
        FORCEINLINE T* operator->()
        {
            return mObject;
        }
        FORCEINLINE T* GetRaw() const 
        { 
            return mObject; 
        }
        FORCEINLINE void Clear()
        {//release ownership, ref-1
            if (mObject)
            {
                mObject->Release();
                mObject = nullptr;
            }
        }
    private:
        T* mObject;
    };
}

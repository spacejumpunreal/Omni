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
    private:
        T* mObject;
    public:
        static_assert(std::is_base_of_v<SharedObject, T>, "T must be derived from IShared");
        FORCEINLINE SharedPtr()
            : mObject(nullptr)
        {
        }
        template<typename U>
        FORCEINLINE SharedPtr(const SharedPtr<U>& other)
            : mObject(static_cast<T*>(other.GetRaw()))
        {//copy, ref+1
            mObject->Acquire(); 
        }
        FORCEINLINE SharedPtr(const SharedPtr& other)
            : mObject(other.GetRaw())
        {//copy, ref+1
            mObject->Acquire();
        }
        template<typename U>
        FORCEINLINE SharedPtr(U*&& other)
            : mObject(static_cast<T*>(std::forward<U*>(other)))
        {//transfer ownership, ref not changed
            other = nullptr;
        }
        FORCEINLINE ~SharedPtr()
        {
            Clear();
        }
        FORCEINLINE SharedPtr& operator=(T*& other)
        {//new ref + 1, release old ownership
            other->Acquire();
            Clear();
            mObject = other;
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
        FORCEINLINE T** operator&()
        {
            return &mObject;
        }
        FORCEINLINE void Clear()
        {//release ownership, ref-1
            if (mObject)
            {
                mObject->Release();
                mObject = nullptr;
            }
        }
    };

}

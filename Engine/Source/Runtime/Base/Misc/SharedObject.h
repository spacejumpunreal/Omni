#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include <atomic>
#include <type_traits>

namespace Omni
{
    class SharedObject
    {
    public:
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

        FORCEINLINE virtual void Destroy()
        {
            this->~SharedObject();
        }
    private:
        std::atomic<u64>    mRefCount;
    };

    template<typename T>
    class SharedPtr
    {
    public:
        FORCEINLINE T* GetRaw() const { return mObject; }

        template<typename U>
        FORCEINLINE SharedPtr(const SharedPtr<U>& other)
            : mObject(static_cast<T*>(other.GetRaw()))
        { 
            mObject->Acquire(); 
        }

        FORCEINLINE SharedPtr(T* other = nullptr)
            : mObject(other)
        {
            if (mObject)
                mObject->Acquire();
        }

        FORCEINLINE ~SharedPtr()
        {
            if (mObject)
            {
                mObject->Release();
                mObject = nullptr;
            }
        }
        FORCEINLINE T* operator->()
        {
            return mObject;
        }
        
    private:
        T* mObject;
    };
}

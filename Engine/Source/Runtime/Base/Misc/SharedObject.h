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
        BASE_API virtual ~SharedObject() {}

        BASE_API void Acquire()
        {
            mRefCount.fetch_add(1, std::memory_order_relaxed);
        }

        BASE_API void Release()
        {
            u64 oldValue = mRefCount.fetch_sub(1, std::memory_order_acq_rel);
            if (oldValue == 1)
            {
                Destroy();
            }
        }

        BASE_API virtual void Destroy()
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
        SharedPtr() : mObject(nullptr) {}
        T* GetRaw() { return mObject; }

        template<typename U>
        SharedPtr(const SharedPtr<U>& other)
            : mObject(static_cast<T>(other.mObject))
        { 
            mObject->Acquire(); 
        }

        ~SharedPtr() 
        {
            if (mObject)
            {
                mObject->Release();
                mObject = nullptr;
            }
        }
        
    private:
        SharedObject* mObject;
    };
}
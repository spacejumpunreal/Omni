#pragma once
#include "Omni.h"
#include "BaseAPI.h"
#include "Misc/PrivateData.h"
#include <atomic>

namespace Omni
{
    class SpinLock
    {
    public:
        BASE_API SpinLock();
        void BASE_API Lock();
        bool BASE_API TryLock();
        void BASE_API Unlock();
    private:
        std::atomic_bool mData;
    };

    class LockGuard
    {
    public:
        FORCEINLINE LockGuard(SpinLock& lock)
            : mLock(lock)
        {
            lock.Lock();
        }
        FORCEINLINE ~LockGuard()
        {
            mLock.Unlock();
        }
    private:
        SpinLock& mLock;
    };
}


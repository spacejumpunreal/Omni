#pragma once
#include "Runtime/Omni.h"
#include <atomic>

namespace Omni
{
    class SpinLock
    {
    public:
        SpinLock();
        void Lock();
        bool TryLock();
        void Unlock();
    private:
        std::atomic<bool> mFlag;
    };
}


#pragma once
#include "CoreAPI.h"

namespace Omni
{
    struct LockfreeNode;

    struct CORE_API LockfreeNodeCache
    {
    public:
        static void GlobalInitialize();
        static void GlobalFinalize();
        static void ThreadInitialize();
        static void ThreadFinalize();
        static LockfreeNode* Alloc();
        static void Free(LockfreeNode* node);
    };
}



#pragma once

namespace Omni
{
    struct LockfreeNode;

    struct LockfreeNodeCache
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



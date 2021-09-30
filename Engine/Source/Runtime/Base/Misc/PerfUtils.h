#pragma once
#include "Runtime/Prelude/Omni.h"

namespace Omni
{
    namespace TimeConsumingFunctions
    {
        inline u64 Fab(u64 i)
        {
            if (i <= 1)
                return 1;
            return Fab(i - 2) + Fab(i - 1);
        }
    }
    
}

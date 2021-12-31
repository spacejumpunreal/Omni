#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS

namespace Omni
{
    //typedefs
    using DX12Descriptor = u64;
    constexpr DX12Descriptor NullDX12Descriptor = 0;
}

#endif//OMNI_WINDOWS

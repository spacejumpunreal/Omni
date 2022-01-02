#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS

struct ID3D12GraphicsCommandList4;

namespace Omni
{
    ID3D12GraphicsCommandList4* SetupDirectCommandList();

}

#endif//OMNI_WINDOWS

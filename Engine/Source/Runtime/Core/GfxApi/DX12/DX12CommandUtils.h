#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS

struct ID3D12GraphicsCommandList4;

namespace Omni
{
ID3D12GraphicsCommandList4* SetupDirectCommandList();
ID3D12GraphicsCommandList4* SetupCopyCommandList();

} // namespace Omni

#endif // OMNI_WINDOWS

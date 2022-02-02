#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Core/GfxApi/GfxApiGraphicState.h"

namespace Omni
{
class DX12PSOManager
{
public:
    static DX12PSOManager* Create();
    void                   Destroy();
};

}
#endif // OMNI_WINDOWS

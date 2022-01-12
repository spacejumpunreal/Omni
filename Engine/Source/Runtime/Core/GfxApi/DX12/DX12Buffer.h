#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiConstants.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"

struct ID3D12Resource;

namespace Omni
{
    class DX12Buffer
    {
    public:
        DX12Buffer(const GfxApiBufferDesc& desc);
        ~DX12Buffer();
        const GfxApiBufferDesc GetDesc();
    private:
        GfxApiBufferDesc            mDesc;
        ID3D12Resource*             mDX12Buffer;
        //descriptor related things later
    };
}


#endif//OMNI_WINDOWS

#pragma once
#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"

namespace Omni
{

D3D12_COMMAND_LIST_TYPE QueueTypeToCmdType(GfxApiQueueType queueType)
{
    switch (queueType)
    {
    case GfxApiQueueType::GraphicsQueue:
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case GfxApiQueueType::ComputeQueue:
        return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    case GfxApiQueueType::CopyQueue:
        return D3D12_COMMAND_LIST_TYPE_COPY;
    default:
        NotImplemented();
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    }
}

DXGI_FORMAT ToDXGIFormat(GfxApiFormat format)
{
    switch (format)
    {
    case GfxApiFormat::R8G8B8A8_UNORM:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case GfxApiFormat::R11G11B10_FLOAT:
        return DXGI_FORMAT_R11G11B10_FLOAT;
    case GfxApiFormat::R16G16B16A16_FLOAT:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case GfxApiFormat::D24_UNORM_S8_UINT:
        return DXGI_FORMAT_D24_UNORM_S8_UINT;
    default:
        NotImplemented();
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    }
}

}

#endif//OMNI_WINDOWS

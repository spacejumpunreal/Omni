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

}

#endif//OMNI_WINDOWS

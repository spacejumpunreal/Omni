#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Memory/HandleObjectPool.h"
#include "Runtime/Base/Memory/ObjectCache.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Core/Platform/OSUtils_Windows.h"
#include <d3d12.h>
#include <dxgi1_6.h>

namespace Omni
{
// forward decls
class DX12Texture;
class DX12SwapChain;
class DX12Buffer;
class DX12Shader;
class DX12GpuEvent;
class DXCWrapper;

struct DX12Singletons
{
public:
    IDXGIFactory7*      DXGIFactory;
    IDXGIAdapter1*      DXGIAdaptor;
    ID3D12Device*       D3DDevice;
    ID3D12CommandQueue* D3DQueues[(u32)GfxApiQueueType::Count];

public:
    DX12Singletons();
    void Finalize();
};

struct DX12GlobalState
{
public:
    DX12GlobalState();
    void Initialize();
    void Finalize();
    void WaitGPUIdle();

public: // DX12 global objects
    DX12Singletons Singletons;

public: // DX12 object pools
    ObjectCache<ID3D12GraphicsCommandList4> CommandListCache[(u32)GfxApiQueueType::Count];
    ObjectCache<ID3D12CommandAllocator>     CommandAllocatorCache[(u32)GfxApiQueueType::Count];
    // we can have a ID3D12Fence cache here
public: // object pools
    RawPtrObjectPool<DX12SwapChain> DX12SwapChainPool;
    RawPtrObjectPool<DX12Buffer>    DX12BufferPool;
    RawPtrObjectPool<DX12Texture>   DX12TexturePool;
    RawPtrObjectPool<DX12Shader>    DX12ShaderPool;
    RawPtrObjectPool<DX12GpuEvent>  DX12GpuEventPool;

public: // state flags
    bool Initialized;

public: // managers
    class DX12TimelineManager* TimelineManager;
    class DX12DeleteManager*   DeleteManager;
    class DX12BufferManager*   BufferManager;
    class DXCWrapper*          DXCInstance;
};

extern DX12GlobalState gDX12GlobalState;
} // namespace Omni

#endif // OMNI_WINDOWS

#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/GfxApiObjectCacheFactory.h"


#include <d3d12.h>

namespace Omni
{
    //cache object factories
    struct ID3D12CommandListCacheFactory final 
        : public GfxApiObjectCacheFactory<ID3D12CommandList, ID3D12CommandListCacheFactory>
    {
        ID3D12CommandListCacheFactory(ID3D12Device* device)
            : mDevice(device)
        {
            mDevice->AddRef();
        }
        void Destroy() override
        {
            SafeRelease(mDevice);
            GfxApiObjectCacheFactory<ID3D12CommandList, ID3D12CommandListCacheFactory>::Destroy();
        }
        void* CreateObject() override
        {
            ID3D12CommandList* p;
            CheckGfxApi(mDevice->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                nullptr, nullptr,
                IID_PPV_ARGS(&p)));
            return p;
        }
        void DestroyObject(void* obj) override
        {
            ID3D12CommandList* p = (ID3D12CommandList*)obj;
            p->Release();
        }
        void RecycleCleanup(void* obj) override
        {
            (void)obj;
            //check if close perhaps?
        }

    private:
        ID3D12Device* mDevice;
    };

    struct ID3D12CommandAllocatorCacheFactory final 
        : public GfxApiObjectCacheFactory<ID3D12CommandAllocator, ID3D12CommandAllocatorCacheFactory>
    {
        ID3D12CommandAllocatorCacheFactory(ID3D12Device* device)
            : mDevice(device)
        {
            mDevice->AddRef();
        }
        void Destroy() override
        {
            SafeRelease(mDevice);
            GfxApiObjectCacheFactory<ID3D12CommandAllocator, ID3D12CommandAllocatorCacheFactory>::Destroy();
        }
        void* CreateObject() override
        {
            ID3D12CommandAllocator* p;
            mDevice->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&p));
            return p;
        }
        void DestroyObject(void* obj) override
        {
            ID3D12CommandAllocator* p = (ID3D12CommandAllocator*)obj;
            p->Release();
        }
        void RecycleCleanup(void* obj) override
        {
            (void)obj;
            //check if is reseted?
        }

    private:
        ID3D12Device* mDevice;
    };

}


#endif//OMNI_WINDOWS

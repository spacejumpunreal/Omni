#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Memory/ObjectCache.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"


#include <d3d12.h>

namespace Omni
{
    /**
    * DX12 object cache
    */
    struct ID3D12GraphicsCommandList4CacheFactory final : public IObjectCacheFactory
    {
        DEFINE_GFX_API_OBJECT_NEW_DELETE();

        ID3D12GraphicsCommandList4CacheFactory(ID3D12Device* device)
            : mDevice(device)
        {
            mDevice->AddRef();
            CheckDX12(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mDummyAllocator)));
        }
        void Destroy() override
        {
            SafeRelease(mDummyAllocator);
            SafeRelease(mDevice);
            delete this;
        }
        void* CreateObject() override
        {
            ID3D12GraphicsCommandList4* p;
            auto ret = mDevice->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                mDummyAllocator, nullptr,
                IID_PPV_ARGS(&p));
            CheckDX12(ret);
            CheckDX12(p->Close());
            return p;
        }
        void DestroyObject(void* obj) override
        {
            ID3D12GraphicsCommandList4* p = (ID3D12GraphicsCommandList4*)obj;
            p->Release();
        }
        void RecycleCleanup(void* obj) override
        {
            (void)obj;
            //check if close perhaps?
        }

    private:
        ID3D12Device*               mDevice;
        ID3D12CommandAllocator*     mDummyAllocator;
    };

    struct ID3D12CommandAllocatorCacheFactory final : public IObjectCacheFactory
    {
        DEFINE_GFX_API_OBJECT_NEW_DELETE();

        ID3D12CommandAllocatorCacheFactory(ID3D12Device* device)
            : mDevice(device)
        {
            mDevice->AddRef();
        }
        void Destroy() override
        {
            SafeRelease(mDevice);
            delete this;
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

    /**
    * object cache
    */
}


#endif//OMNI_WINDOWS

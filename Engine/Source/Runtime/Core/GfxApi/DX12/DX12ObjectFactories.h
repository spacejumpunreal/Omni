#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Memory/ObjectCache.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"


#include <d3d12.h>

namespace Omni
{
    //cache object factories
    struct ID3D12CommandListCacheFactory : public IObjectCacheFactory
    {
        void* operator new(size_t size)
        {
            return MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi).allocate_bytes(size, alignof(ID3D12CommandListCacheFactory));
        }
        ID3D12CommandListCacheFactory(ID3D12Device* device)
            : mDevice(device)
        {
            mDevice->AddRef();
        }
        void Destroy()
        {
            SafeRelease(mDevice);
            MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi).delete_object(this);
        }
        void* CreateObject()
        {
            ID3D12CommandList* p;
            CheckGfxApi(mDevice->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
                nullptr, nullptr,
                IID_PPV_ARGS(&p)));
            return p;
        }
        void DestroyObject(void* obj)
        {
            ID3D12CommandList* p = (ID3D12CommandList*)obj;
            p->Release();
        }
        void RecycleCleanup(void* obj)
        {
            (void)obj;
            //check if close perhaps?
        }

    private:
        ID3D12Device* mDevice;
    };
}


#endif//OMNI_WINDOWS

#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Base/Misc/SharedObject.h"

//forward decl
struct ID3D12CommandAllocator;
struct ID3D12CommandQueue;
struct IUnknown;

namespace Omni
{
    

    //declarations
    struct TimelineHelpers
    {
        static void RecycleDirectCommandAllocator(ID3D12CommandAllocator* commandAllocator);
        static void ReleaseD3DObject(IUnknown* obj);
        static void ReleaseSharedObject(SharedObject* obj);

        template<typename TObject>
        static DX12TimelineManager::DX12RecycleCB CreateRecycleCB(
            void (*func)(TObject*), 
            TObject* obj)
        {
            return DX12TimelineManager::DX12RecycleCB((void(*)(void*))func, (void*)obj);
        }
    };
    

}

#endif//OMNI_WINDOWS

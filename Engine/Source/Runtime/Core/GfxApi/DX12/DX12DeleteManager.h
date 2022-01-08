#pragma once
#include "Runtime/Prelude/Omni.h"

#if OMNI_WINDOWS
#include "Runtime/Base/Misc/PrivateData.h"
#include "Runtime/Base/Misc/FunctorUtils.h"
#include "Runtime/Base/Memory/ObjectHandle.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"


struct ID3D12CommandList;
struct ID3D12Device;

namespace Omni
{
    //forward decl

    //constants

    //declaration
    class DX12DeleteManager
    {
    public:
        using DX12DeleteCB = Action1<void, void*>;;
    public:
        DX12DeleteManager();
        ~DX12DeleteManager();

        template<typename TObject>
        void AddForDelete(TObject* obj, GfxApiQueueMask queueMask);
        
        void AddForHandleFree(void(*func)(void*), IndexHandle handle, GfxApiQueueMask queueMask);

        void Flush();
        void OnBatchDelete();

    private:
        void AddDeleteCB(DX12DeleteCB cb, GfxApiQueueMask queueMask);
    private:
        PrivateData<1024, 16> mData;
    };
    
    template<typename TObject>
    struct DX12DeleteCBHelper
    {
        static void DoDelete(TObject* obj)
        {
            delete obj;
        }
    };

    template<typename TObject>
    struct DX12FreeHandleCBHelper
    {
        static void DoFree(void* handle)
        {
        }
    };

    //method definition
    template<typename TObject>
    void DX12DeleteManager::AddForDelete(TObject* obj, GfxApiQueueMask queueMask)
    {
        DX12DeleteManager::DX12DeleteCB cb(
            (void(*)(void*))DX12DeleteCBHelper<TObject>::DoDelete, 
            (void*)obj);
        AddDeleteCB(cb, queueMask);
    }
}


#endif//OMNI_WINDOWS

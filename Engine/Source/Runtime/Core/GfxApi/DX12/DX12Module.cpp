#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/WindowModule.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"
#include "Runtime/Core/GfxApi/GfxApiModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12SwapChain.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12Command.h"
#include "Runtime/Core/GfxApi/DX12/DX12DeleteManager.h"

#include <d3d12.h>
#include <dxgidebug.h>

#define DEBUG_DX_OBJECT_LEAK_ON_QUIT 1


EXTERN_C const GUID DECLSPEC_SELECTANY DXGI_DEBUG_ALL =  { 0xe48ae283, 0xda80, 0x490b, 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8  };


namespace Omni
{
    //forward decls
    // 
    //declarations
    class DX12Module final : public GfxApiModule
    {
    public:
        DX12Module(const EngineInitArgMap& args);
        void Destroy() override;
        void Initialize(const EngineInitArgMap&) override;
        void StopThreads() override;
        void Finalize() override;

#define GfxApiMethod(Definition) Definition override;
#include "Runtime/Core/GfxApi/GfxApiMethodList.inl"
#undef GfxApiMethod
    private:
#if DEBUG_DX_OBJECT_LEAK_ON_QUIT
        void ReportAllLivingObjects();
#endif
    };


    /*
    * DX12Module implementations
    */

    DX12Module::DX12Module(const EngineInitArgMap& args)
    {
        (void)args;
    }

    void DX12Module::Initialize(const EngineInitArgMap& args)
    {
        MemoryModule& mm = MemoryModule::Get();
        mm.Retain();

        CheckDebug(!gDX12GlobalState.Initialized);
        gDX12GlobalState.Initialize();

        Module::Initialize(args);
        CheckAlways(gGfxApiModule == nullptr);
        gGfxApiModule = this;
    }

    void DX12Module::StopThreads()
    {}

    void DX12Module::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        CheckAlways(gGfxApiModule != nullptr);
        gGfxApiModule = nullptr;

        gDX12GlobalState.Finalize();

        MemoryModule& mm = MemoryModule::Get();
        Module::Finalize();
        mm.Release();

#if DEBUG_DX_OBJECT_LEAK_ON_QUIT
        ReportAllLivingObjects();
#endif
    }


    /*
    * GfxApiMethod implementations
    */


    //Buffer
    GfxApiBufferRef DX12Module::CreateBuffer(const GfxApiBufferDesc& desc)
    {
        (void)desc;
        NotImplemented();
        return {};
    }

    void DX12Module::DestroyBuffer(GfxApiBufferRef buffer)
    {
        (void)buffer;
        NotImplemented();
    }


    //Texture
    GfxApiTextureRef DX12Module::CreateTexture(const GfxApiTextureDesc& desc)
    {
        (void)desc;
        NotImplemented();
        return {};
    }

    void DX12Module::DestroyTexture(GfxApiTextureRef texture)
    {
        (void)texture;
        NotImplemented();
    }


    //SwapChain
    GfxApiSwapChainRef DX12Module::CreateSwapChain(const GfxApiSwapChainDesc& desc)
    {
        return new DX12SwapChain(desc);
    }

    void DX12Module::UpdateSwapChain(GfxApiSwapChainRef swapChain, const GfxApiSwapChainDesc& desc)
    {
        DX12SwapChain* dx12SwapChain = static_cast<DX12SwapChain*>(swapChain);
        dx12SwapChain->Update(desc);
    }

    void DX12Module::DestroySwapChain(GfxApiSwapChainRef swapChain)
    {
        DX12SwapChain* dx12SwapChain = static_cast<DX12SwapChain*>(swapChain);
        gDX12GlobalState.DeleteManager->AddForDelete(dx12SwapChain, AllQueueMask);
    }

    void DX12Module::GetBackbufferTextures(GfxApiSwapChainRef swapChain, GfxApiTextureRef backbuffers[], u32 count)
    {
        DX12SwapChain* dx12SwapChain = static_cast<DX12SwapChain*>(swapChain);
        dx12SwapChain->GetBackbufferTextures(backbuffers, count);
    }
    
    u32 DX12Module::GetCurrentBackbufferIndex(GfxApiSwapChainRef swapChain)
    {
        DX12SwapChain* dx12SwapChain = static_cast<DX12SwapChain*>(swapChain);
        return dx12SwapChain->GetCurrentBackbufferIndex();
    }


    //GpuEvent
    bool DX12Module::IsEventTriggered(GfxApiGpuEventRef gpuEvent)
    {
        (void)gpuEvent;
        NotImplemented();
        return false;
    }

    void DX12Module::WaitEvent(GfxApiGpuEventRef gpuEvent)
    {
        (void)gpuEvent;
        NotImplemented();
    }

    void DX12Module::DestroyEvent(GfxApiGpuEventRef gpuEvent)
    {
        (void)gpuEvent;
        NotImplemented();
    }

    //AsyncActions
    void DX12Module::DrawRenderPass(GfxApiRenderPass* renderPass, GfxApiGpuEventRef* doneEvent)
    {
        DX12DrawRenderPass(renderPass, doneEvent);
    }

    void DX12Module::DispatchComputePass(GfxApiComputePass* computePass, GfxApiGpuEventRef* doneEvent)
    {
        (void)computePass;
        (void)doneEvent;
        NotImplemented();
    }

    void DX12Module::Present(GfxApiSwapChainRef swapChain, bool waitVSync, GfxApiGpuEventRef* doneEvent)
    {
        DX12SwapChain* dx12SwapChain = static_cast<DX12SwapChain*>(swapChain);
        dx12SwapChain->Present(waitVSync);
        if (doneEvent != nullptr)
            NotImplemented("doneEvent");
    }

    void DX12Module::ScheduleGpuEvent(GfxApiQueueType queueType, GfxApiGpuEventRef* doneEvent)
    {
        (void)doneEvent;
        ID3D12CommandQueue* queue = nullptr;
        switch (queueType)
        {
        case GfxApiQueueType::GraphicsQueue:
            queue = gDX12GlobalState.D3DGraphicsCommandQueue;
            break;
        default:
            NotImplemented("ScheduleGpuEvent only implemented for GraphicsQueue");
            break;
        }
        u64 batchId;
        gDX12GlobalState.TimelineManager->CloseBatchAndSignalOnGPU(queueType, queue, batchId, true);
        if (doneEvent != nullptr)
        {
            NotImplemented("doneEvent != nullptr for DX12Module::ScheduleGpuEvent");
        }
    }
    //Maintain operations
    void DX12Module::CloseBatchDelete()
    {
        gDX12GlobalState.DeleteManager->Flush();
    }

    void DX12Module::CheckGpuEvents(GfxApiQueueMask queueMask)
    {
        gDX12GlobalState.TimelineManager->PollBatch(queueMask);
    }

    //Private functions

#if DEBUG_DX_OBJECT_LEAK_ON_QUIT
    void DX12Module::ReportAllLivingObjects()
    {
        IDXGIDebug* dxgiDebug;
        CheckSucceeded(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)));
        CheckSucceeded(dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL));
        //OmniDebugBreak();
        dxgiDebug->Release();
    }
#endif
     
    /*
    * DX12Module ctors
    */
    static Module* DX12ModuleCtor(const EngineInitArgMap& args)
    {
        return InitMemFactory<DX12Module>::New(args);
    }

    void DX12Module::Destroy()
    {
        InitMemFactory<DX12Module>::Delete((DX12Module*)this);
    }

    EXPORT_INTERNAL_MODULE(DX12Module, ModuleExportInfo(DX12ModuleCtor, false, "DX12"));
}

#endif//OMNI_WINDOWS

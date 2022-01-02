/**
* GfxApiObjects
*/

//Buffer
GfxApiMethod(GfxApiBufferRef CreateBuffer(const GfxApiBufferDesc& desc))
GfxApiMethod(void DestroyBuffer(GfxApiBufferRef buffer))
//Texture
GfxApiMethod(GfxApiTextureRef CreateTexture(const GfxApiTextureDesc& desc))
GfxApiMethod(void DestroyTexture(GfxApiTextureRef))
//SwapChain
GfxApiMethod(GfxApiSwapChainRef CreateSwapChain(const GfxApiSwapChainDesc& desc))
GfxApiMethod(void UpdateSwapChain(GfxApiSwapChainRef swapChain, const GfxApiSwapChainDesc& desc))
GfxApiMethod(void DestroySwapChain(GfxApiSwapChainRef))
GfxApiMethod(void GetBackbufferTextures(GfxApiSwapChainRef swapChain, GfxApiTextureRef backbuffers[], u32 count))
GfxApiMethod(u32 GetCurrentBackbufferIndex(GfxApiSwapChainRef swapChain))
//GpuEvent
GfxApiMethod(bool IsEventTriggered(GfxApiGpuEventRef gpuEvent))
GfxApiMethod(void WaitEvent(GfxApiGpuEventRef gpuEvent))
GfxApiMethod(void DestroyEvent(GfxApiGpuEventRef gpuEvent))


/**
* AsyncActions, One-way(RenderModule to GfxApiModule)
*/
GfxApiMethod(void DrawRenderPass(GfxApiRenderPass* renderPass, GfxApiGpuEventRef* doneEvent))
GfxApiMethod(void DispatchComputePass(GfxApiComputePass* computePass, GfxApiGpuEventRef* doneEvent))
GfxApiMethod(void Present(GfxApiSwapChainRef swapChain, bool waitVSync, GfxApiGpuEventRef* doneEvent))
GfxApiMethod(void ScheduleGpuEvent(GfxApiQueueType queueType, GfxApiGpuEventRef* doneEvent))



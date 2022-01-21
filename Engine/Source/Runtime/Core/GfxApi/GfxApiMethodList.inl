/**
* GfxApiObjects
*/

//Buffer
GfxApiMethod(GfxApiBufferRef CreateBuffer(const GfxApiBufferDesc& desc))
GfxApiMethod(void* MapBuffer(GfxApiBufferRef buffer, u32 offset, u32 size))
GfxApiMethod(void UnmapBuffer(GfxApiBufferRef buffer, u32 offset, u32 size))
GfxApiMethod(void DestroyBuffer(GfxApiBufferRef buffer))
//Texture
GfxApiMethod(GfxApiTextureRef CreateTexture(const GfxApiTextureDesc& desc))
GfxApiMethod(void DestroyTexture(GfxApiTextureRef))
//SwapChain
GfxApiMethod(GfxApiSwapChainRef CreateSwapChain(const GfxApiSwapChainDesc& desc))
GfxApiMethod(void UpdateSwapChain(GfxApiSwapChainRef swapChain, const GfxApiSwapChainDesc& desc)) //NOTE: this will WaitGPUIdle
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
GfxApiMethod(void DrawRenderPass(GfxApiRenderPass* renderPass))
GfxApiMethod(void DispatchComputePass(GfxApiComputePass* computePass))
GfxApiMethod(void CopyBlitPass(GfxApiBlitPass* blitPass))
GfxApiMethod(void Present(GfxApiSwapChainRef swapChain, bool waitVSync))
GfxApiMethod(void ScheduleGpuEvent(GfxApiQueueType queueType, GfxApiGpuEventRef* gpuEvent))

/**
* Maintain operations
*/
GfxApiMethod(void CloseBatchDelete())
GfxApiMethod(void CheckGpuEvents(GfxApiQueueMask queueMask))


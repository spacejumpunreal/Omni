GfxApiMethod(SharedPtr<SharedObject> CreateGfxApiObject(const GfxApiObjectDesc& desc))

//swapchain
GfxApiMethod(void UpdateSwapChain(GfxApiSwapChain& swapChain))

//renderpass
GfxApiMethod(GfxApiRenderPass* BeginRenderPass(const GfxApiRenderPassDesc& desc))
GfxApiMethod(void EndRenderPass(const GfxApiRenderPass* desc))

//command context
GfxApiMethod(GfxApiContext* BeginContext(const GfxApiContextDesc& desc)) //for threaded recording
GfxApiMethod(void EndContext(GfxApiContext* context))

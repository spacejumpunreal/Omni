//Buffer
GfxApiMethod(GfxApiBufferRef CreateBuffer(const GfxApiBufferDesc& desc))

//Texture
GfxApiMethod(GfxApiTextureRef CreateTexture(const GfxApiTextureDesc& desc))

//SwapChain
GfxApiMethod(GfxApiSwapChainRef CreateSwapChain(const GfxApiSwapChainDesc& desc))

//Renderpass
GfxApiMethod(GfxApiRenderPass* BeginRenderPass(const GfxApiRenderPassDesc& desc))
GfxApiMethod(void EndRenderPass(const GfxApiRenderPass* desc))

//Command Context
GfxApiMethod(GfxApiCommandContext* BeginContext(const GfxApiCommandContextDesc& desc)) //for threaded recording
GfxApiMethod(void EndContext(GfxApiCommandContext* context))

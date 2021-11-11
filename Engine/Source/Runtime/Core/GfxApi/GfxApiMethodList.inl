GfxApiMethod(SharedPtr<SharedObject> CreateResource(const GfxApiResourceDesc& desc))

GfxApiMethod(GfxApiRenderPass* BeginRenderPass(const GfxApiRenderPassDesc& desc))
GfxApiMethod(void EndRenderPass(const GfxApiRenderPass* desc))

GfxApiMethod(GfxApiContext* BeginContext(const GfxApiContextDesc& desc)) //for threaded recording
GfxApiMethod(void EndContext(GfxApiContext* context))

GfxApiMethod(void ResizeBackbuffer(u32 width, u32 height, u32 bufferCount))
GfxApiMethod(GfxApiTexture* GetBackBuffer(u32 index))
GfxApiMethod(u32 GetBackBufferCount())
GfxApiMethod(u32 GetCurrentFrameIndex())
GfxApiMethod(void Present())
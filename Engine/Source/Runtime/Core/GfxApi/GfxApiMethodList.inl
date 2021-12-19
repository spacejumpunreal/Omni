GfxApiMethod(SharedPtr<SharedObject> CreateGfxApiObject(const GfxApiObjectDesc& desc))

//renderpass
GfxApiMethod(GfxApiRenderPass* BeginRenderPass(const GfxApiRenderPassDesc& desc))
GfxApiMethod(void EndRenderPass(const GfxApiRenderPass* desc))

//command context
GfxApiMethod(GfxApiCommandContext* BeginContext(const GfxApiCommandContextDesc& desc)) //for threaded recording
GfxApiMethod(void EndContext(GfxApiCommandContext* context))

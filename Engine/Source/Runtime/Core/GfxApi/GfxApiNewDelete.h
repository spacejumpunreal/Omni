#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Memory/MemoryDefs.h"
#include "Runtime/Core/Allocator/MemoryModule.h"


#define DEFINE_GFX_API_TEMP_NEW_DELETE() \
    CORE_API FORCEINLINE void* operator new(size_t size, std::align_val_t align)\
    {\
        PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApiTmp);\
        return alloc.allocate_bytes(size, (size_t)align);\
    }\
    CORE_API FORCEINLINE void operator delete(void* ptr, size_t size, std::align_val_t align)\
    {\
        PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApiTmp);\
        alloc.deallocate_bytes(ptr, size, (size_t)align);\
    }\


#define DEFINE_GFX_API_OBJECT_NEW_DELETE() \
    CORE_API FORCEINLINE void* operator new(size_t size, std::align_val_t align)\
    {\
        PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi);\
        return alloc.allocate_bytes(size, (size_t)align);\
    }\
    CORE_API FORCEINLINE void operator delete(void* ptr, size_t size, std::align_val_t align)\
    {\
        PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi);\
        alloc.deallocate_bytes(ptr, size, (size_t)align);\
    }\





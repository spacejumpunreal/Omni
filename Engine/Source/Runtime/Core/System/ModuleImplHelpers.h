#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Base/Memory/MonotonicMemoryResource.h"
#include "Runtime/Core/System/Module.h"

namespace Omni
{
    template<typename ModuleT>
    struct InitMemFactory
    {
        template<typename... Args>
        static ModuleT* New(Args&&... args)
        {
            MonotonicMemoryResource& initMem = System::GetSystem().GetInitMemResource();
            PMRAllocatorT<ModuleT> alloc(&initMem);
            ModuleT* ret = alloc.allocate(1);
            alloc.construct(ret, std::forward<Args>(args)...);
            return ret;
        }

        static void Delete(ModuleT* _module)
        {
            MonotonicMemoryResource& initMem = System::GetSystem().GetInitMemResource();
            PMRAllocatorT<ModuleT> alloc(&initMem);
            std::allocator_traits<PMRAllocatorT<ModuleT>>::destroy(alloc, _module);
            //allocation size doesn't matter for this allocator, it only check counts, checking size is doable but would add more trouble
            alloc.deallocate(_module, 0);
        }
    };
}

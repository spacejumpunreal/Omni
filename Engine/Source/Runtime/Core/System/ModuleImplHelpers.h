#pragma once
#include "Runtime/Omni.h"
#include "Runtime/System/System.h"
#include "Runtime/Memory/MonotonicMemoryResource.h"

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
            alloc.deallocate(_module, 1);//the size here is wrong, size doesn't matter for this allocator, allocator will figure it out
        }
    };
}
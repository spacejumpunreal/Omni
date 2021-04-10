#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Concurrency/IThreadLocal.h"
#include "Runtime/Concurrency/LockfreeContainer.h"
#include "Runtime/Concurrency/ThreadUtils.h"
#include "Runtime/Memory/CacheLineAllocator.h"
#include "Runtime/Memory/MemoryArena.h"
#include "Runtime/Memory/SNAllocator.h"
#include "Runtime/Memory/WrapperAllocator.h"
#include "Runtime/Misc/PImplUtils.h"
#include "Runtime/Misc/ThreadLocalData.h"
#include "Runtime/Platform/PlatformAPIs.h"
#include "Runtime/System/Module.h"
#include "Runtime/System/ModuleExport.h"
#include "Runtime/System/ModuleImplHelpers.h"
#include "Runtime/Test/AssertUtils.h"

#if OMNI_WINDOWS
#include <Windows.h>
#include <memoryapi.h>
#include <atomic>
#elif OMNI_IOS
#include <sys/mman.h>
#endif

namespace Omni
{
    //flags
    static constexpr bool StatsMemoryKinds = false;

    //forard decalrations
    class IAllocator;

    //definitions
    struct MemoryModulePrivateData
    {
        static constexpr size_t			DefaultThreadArenaSize = 1024 * 1024;
    public:
        PMRResource*					mKind2PMRResources[(size_t)MemoryKind::Max];
        IAllocator*						mAllocators[(size_t)MemoryKind::Max];
        u32								mThreadArenaSize;
        CacheLineAllocator*				mCacheLineAllocator;
        std::atomic<size_t>				mMmappedSize;
    public:
        MemoryModulePrivateData();
    };
    using MemoryModuleImpl = PImplCombine<MemoryModule, MemoryModulePrivateData>;
    //globals
    MemoryModuleImpl*						gMemoryModule;
    OMNI_DECLARE_THREAD_LOCAL(ScratchStack, gThreadArena);
    //methods

    MemoryModulePrivateData::MemoryModulePrivateData()
        : mThreadArenaSize(DefaultThreadArenaSize)
        , mCacheLineAllocator(nullptr)
    {
#if OMNI_WINDOWS
        std::pmr::set_default_resource(std::pmr::null_memory_resource());
#endif

        memset(mKind2PMRResources, 0, sizeof(mKind2PMRResources));
        memset(mAllocators, 0, sizeof(mAllocators));

        CheckAlways(gMemoryModule == nullptr, "singleton rule violated");
        MemoryModuleImpl* self = MemoryModuleImpl::GetCombinePtr(this);
        gMemoryModule = self;
        
        CheckAlways(IsOnMainThread());
        LockfreeNodeCache::GlobalInitialize();
        LockfreeNodeCache::ThreadInitialize(); //cacheline allocator will use this, so init early for main thread
        size_t usedAllocators = 0;
        IAllocator* primary = new SNAllocator();
        self->mAllocators[usedAllocators++] = primary;
        self->mKind2PMRResources[(u32)MemoryKind::SystemInit] = primary->GetResource();
        
        IAllocator* cacheline = self->mCacheLineAllocator = new CacheLineAllocator();
        self->mAllocators[usedAllocators++] = cacheline;
        self->mKind2PMRResources[(u32)MemoryKind::CacheLine] = cacheline->GetResource();

        if constexpr (StatsMemoryKinds)
        {
            const char* MemoryKindNames[] =
            {
#define MEMORY_KIND(X) #X,
#include "Runtime/Memory/MemoryKind.inl"
#undef MEMORY_KIND
            };
            for (u32 iKind = 0; iKind < (u32)MemoryKind::Max; ++iKind)
            {
                PMRResource*& res = self->mKind2PMRResources[iKind];
                if (res == nullptr)
                {
                    IAllocator* alloc = self->mAllocators[usedAllocators++] = new WrapperAllocator(*primary->GetResource(), MemoryKindNames[iKind]);
                    res = alloc->GetResource();
                }
            }
        }
        else
        {
            for (u32 iKind = 0; iKind < (u32)MemoryKind::Max; ++iKind)
            {
                PMRResource*& res = self->mKind2PMRResources[iKind];
                if (res == nullptr)
                {
                    res = primary->GetResource();
                }
            }
        }
    }
    void MemoryModule::Initialize(const EngineInitArgMap& args)
    {
        Module::Initialize(args);
    }
    void MemoryModule::Finalize()
    {
        MemoryModulePrivateData* self = MemoryModuleImpl::GetCombinePtr(this);
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        for (i32 iAllocator = (i32)MemoryKind::Max - 1; iAllocator >= 0; --iAllocator)
        {
            IAllocator* alloc = self->mAllocators[iAllocator];
            if (alloc == nullptr)
                continue;
            alloc->Shrink();
            MemoryStats ms = alloc->GetStats();
            CheckAlways(ms.Used == 0);
            delete alloc;
        }
        LockfreeNodeCache::ThreadFinalize();
        LockfreeNodeCache::GlobalFinalize();
        CheckAlways(self->mMmappedSize == 0);
        Module::Finalize();
        gMemoryModule = nullptr;
    }
    MemoryModule& MemoryModule::Get()
    {
        return *gMemoryModule;
    }
    PMRAllocator MemoryModule::GetPMRAllocator(MemoryKind kind)
    {
        MemoryModuleImpl* self = MemoryModuleImpl::GetCombinePtr(this);
        return self->mKind2PMRResources[(u32)kind];
    }
    void* MemoryModule::Mmap(size_t size, size_t alignment)
    {
        CheckAlways(IsPow2((u64)alignment));
        CheckAlways(IsAligned((void*)size, alignment));
        MemoryModuleImpl* self = MemoryModuleImpl::GetCombinePtr(this);
        self->mMmappedSize.fetch_add(size, std::memory_order_relaxed);
#if OMNI_WINDOWS
        MEM_EXTENDED_PARAMETER param = { 0 };
        MEM_ADDRESS_REQUIREMENTS addrReq = { 0 };
        addrReq.Alignment = alignment;
        param.Type = MEM_EXTENDED_PARAMETER_TYPE::MemExtendedParameterAddressRequirements;
        param.Pointer = &addrReq;
        void* p = VirtualAlloc2(GetCurrentProcess(), nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE, &param, 1);
        if (!p)
        {
            DWORD err = GetLastError();
            const char* messageBuffer = nullptr;

            //Ask Win32 to give us the string version of that message ID.
            //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
            CheckAlways(p, messageBuffer);
        }
        
        return p;
#elif OMNI_IOS
        u8* addr = (u8*)mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
        CheckAlways(addr != MAP_FAILED);
        u8* alignedAddr = (u8*)AlignUpPtr(addr, alignment);
        size_t unalignedSize = ((u8*)alignedAddr) - addr;
        if (!unalignedSize)
            return addr;
        munmap(addr, unalignedSize);
        void* next = mmap(addr + size, unalignedSize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
        if (next != MAP_FAILED)
            return alignedAddr;
        munmap(alignedAddr, size - unalignedSize);
        //things are tough, use another strategy: map twice larger range and keep aligned range
        u8* begin = (u8*)mmap(nullptr, size + alignment, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
        CheckAlways(next != MAP_FAILED);
        u8* end = size + alignment + (u8*)begin;
        alignedAddr = (u8*)AlignUpPtr(begin, alignment);
        size_t holeHead = alignedAddr - begin;
        if (holeHead > 0)
            munmap(begin, holeHead);
        size_t holeTail = end - (alignedAddr + size);
        if (holeTail > 0)
            munmap(alignedAddr + size, holeTail);
        return alignedAddr;
#else
        static_assert(false, "not implemented");
        return nullptr;
#endif
    }
    void MemoryModule::Munmap(void* mem, size_t size)
    {
        MemoryModuleImpl* self = MemoryModuleImpl::GetCombinePtr(this);
        self->mMmappedSize.fetch_sub(size, std::memory_order_relaxed);
#if OMNI_WINDOWS
        (void)(size);
        VirtualFreeEx(GetCurrentProcess(), mem, 0, MEM_RELEASE);
#else
        munmap(mem, size);
#endif
    }
    ScratchStack& MemoryModule::GetThreadScratchStack()
    {
        return gThreadArena.GetRaw();
    }
    void MemoryModule::ThreadInitialize()
    {
        CheckAlways(gThreadArena->GetPtr() == nullptr);
        MemoryModuleImpl* self = gMemoryModule;
        void* p = self ->GetPMRAllocator(MemoryKind::ThreadScratchStack).resource()->allocate(self->mThreadArenaSize, OMNI_DEFAULT_ALIGNMENT);
        gThreadArena->Reset((u8*)p, self->mThreadArenaSize);
        if (!IsOnMainThread())
            LockfreeNodeCache::ThreadInitialize();
        if (self->mCacheLineAllocator)
            self->mCacheLineAllocator->ThreadInitialize();
    }
    void MemoryModule::ThreadFinalize()
    {
        MemoryModuleImpl* self = gMemoryModule;
        if (self->mCacheLineAllocator)
            self->mCacheLineAllocator->ThreadFinalize();
        if (!IsOnMainThread())
            LockfreeNodeCache::ThreadFinalize();
        void* p = gThreadArena->Cleanup();
        CheckAlways(p != nullptr);
        self->GetPMRAllocator(MemoryKind::ThreadScratchStack).resource()->deallocate(p, self->mThreadArenaSize, OMNI_DEFAULT_ALIGNMENT);
    }
    void MemoryModule::GetStats(STD_PMR_NS::vector<MemoryStats>& ret)
    {
        MemoryModuleImpl* self = MemoryModuleImpl::GetCombinePtr(this);
        for (IAllocator* a : self->mAllocators)
        {
            if (a)
                ret.push_back(a->GetStats());
        }
    }
    void MemoryModule::Shrink()
    {
        MemoryModuleImpl* self = MemoryModuleImpl::GetCombinePtr(this);
        for (IAllocator* a : self->mAllocators)
        {
            if (a)
                a->Shrink();
        }
    }
    static Module* MemoryModuleCtor(const EngineInitArgMap&)
    {
        return ModuleFactory<MemoryModuleImpl>::Construct();
    }
    void MemoryModule::Destroy()
    {
        return ModuleFactory<MemoryModuleImpl>::Destroy((MemoryModuleImpl*)this);
    }
    ExportInternalModule(Memory, ModuleExportInfo(MemoryModuleCtor, true));
}

void operator delete(void*, Omni::MemoryKind)
{
    Omni::CheckAlways(false);
}


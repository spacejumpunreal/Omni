#include "Runtime/Platform/PlatformAPIs.h"
#if OMNI_WINDOWS
#include <Windows.h>
#include <memoryapi.h>
#elif OMNI_IOS
#include <sys/mman.h>
#endif

namespace Omni
{

	void* AllocPages(size_t size)
	{
#if OMNI_WINDOWS
		return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#elif OMNI_IOS
        return mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_ANON, 0, 0);
#else
		static_assert(false, "not implemented");
		return nullptr;
#endif
	}
	void FreePages(void* mem, size_t size)
	{
#if OMNI_WINDOWS
		(void)(size);
		VirtualFree(mem, 0, MEM_RELEASE);
#else
        munmap(mem, size);
#endif
	}

	void PauseThread()
	{
#if OMNI_WINDOWS
		YieldProcessor();
#else
        __asm__ __volatile__("yield");
#endif
	}

}

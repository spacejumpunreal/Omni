#include "Runtime/Platform/PlatformAPIs.h"
#if OMNI_WINDOWS
#include <Windows.h>
#include <memoryapi.h>
#endif

namespace Omni
{

	void* AllocPages(size_t size)
	{
#if OMNI_WINDOWS
		return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
		static_assert(false, "not implemented");
		return nullptr;
#endif
	}
	void FreePages(void* mem, size_t)
	{
#if OMNI_WINDOWS
		VirtualFree(mem, 0, MEM_RELEASE);
#else
		static_assert(false, "not implemented");
#endif
	}

}
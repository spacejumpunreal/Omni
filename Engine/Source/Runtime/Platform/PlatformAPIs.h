#pragma once
#include "Runtime/Omni.h"

namespace Omni
{
	void* AllocPages(size_t size);
	void FreePages(void* mem, size_t size);

	void Pause();
}
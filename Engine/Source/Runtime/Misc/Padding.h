#pragma once
#include "Runtime/Omni.h"

namespace Omni
{
	template<typename T, size_t PadToSize>
	struct Padded
	{
	public:
		static constexpr size_t PaddingBytes = PadToSize - sizeof(T);
	public:
		T		Data;
		char	Padding[PaddingBytes];
	};
}
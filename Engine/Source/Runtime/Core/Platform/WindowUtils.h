#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"

#if OMNI_WINDOWS
#include <Windows.h>
#endif

namespace Omni
{
	class WindowHandle
	{
	public:
		WindowHandle(const WindowHandle& other)
			: mPtr(other.mPtr)
		{}
#if OMNI_WINDOWS
		WindowHandle(HWND hwnd = nullptr)
			: mPtr(hwnd)
		{}
		HWND ToNativeHandle() const
		{
			return (HWND)mPtr;
		}
#else
#endif
	private:
		void*		mPtr;
	};
}


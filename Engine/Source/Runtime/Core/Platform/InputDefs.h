#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"

namespace Omni
{
	using KeyCode = u32;
	struct MousePos
	{//origin is at bottom left, x go right, y go up, can be negative if cursor is outside client area
		i16 X;
		i16 Y;
	};
	class KeyStateListener
	{
	public:
		virtual void OnKeyEvent(KeyCode key, bool pressed) = 0;
	};
}

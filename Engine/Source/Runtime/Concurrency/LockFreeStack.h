#pragma once
#include "Runtime/Omni.h"
#include <atomic>

namespace Omni
{
	template<typename T>
	class LockFreeStack
	{
	public:
		void Push(T*);
		bool TryPop(T*& pdata);
		void* Pop();
	private:
	};
}
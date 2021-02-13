#pragma once
#include "Runtime/Omni.h"
#include <atomic>

namespace Omni
{
	class IShared
	{
	public:
		IShared() : mRefCount(1) 
		{}
		void Retain() 
		{
			mRefCount.fetch_add(1, std::memory_order::relaxed);
		}
		void Release()
		{
			u32 o = mRefCount.fetch_sub(1, std::memory_order::acq_rel);
			if (o == 1)
				Destroy();
		}
	protected:
		virtual void Destroy()
		{
			delete this;
		};
	private:
		std::atomic<u32>	mRefCount;
	};
}
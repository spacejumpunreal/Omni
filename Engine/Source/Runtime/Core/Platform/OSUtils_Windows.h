#pragma once
#include "Runtime/Prelude/Omni.h"

#define RELEASE_COM(comPtr) { comPtr->Release(); comPtr = nullptr; }

namespace Omni
{
	template<typename ComT>
	class ComPtr
	{
	public:
		FORCEINLINE ComPtr(ComT* ptr = nullptr) 
			: mPtr(ptr)
		{
			if (mPtr)
				mPtr->AddRef();
		}
		FORCEINLINE ComPtr(const ComPtr& ptr)
			: mPtr(ptr.mPtr)
		{
			if (mPtr)
				mPtr->AddRef();
		}
		FORCEINLINE ~ComPtr()
		{
			if (mPtr)
				mPtr->Release();
			mPtr = nullptr;
		}
		FORCEINLINE ComT* operator=(const ComT& ptr)
		{
			if (mPtr)
				mPtr->Release();
			mPtr = ptr;
			if (mPtr)
				mPtr->AddRef();
		}
		FORCEINLINE ComT* operator->() { return mPtr; }
		FORCEINLINE ComT** operator&() { return &mPtr; }
		FORCEINLINE operator ComT* () { return mPtr; }
	private:
		ComT* mPtr;
	};
}
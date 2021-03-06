#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"

namespace Omni
{
	class ConcurrentNode
	{
	public:
		ConcurrentNode()
			: Data(nullptr)
			, Next(nullptr)
			, ABA(0)
		{}
		//FORCEINLINE void InsertBefore(ConcurrentNode* n);
		//FORCEINLINE void InsertAfter(ConcurrentNode* n);
	public:
		void*				Data;
	private:
		ConcurrentNode*		Next;
		u64					ABA;
	};
}

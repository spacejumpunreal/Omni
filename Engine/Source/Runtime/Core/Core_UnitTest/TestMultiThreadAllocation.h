#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Concurrency/ConcurrencyModule.h"
#include "Runtime/Core/Concurrency/JobPrimitives.h"
#include <random>

namespace Omni
{
	struct TestMultiThreadAllocation
	{
		struct JobData
		{};
		TestMultiThreadAllocation(u32)
		{}
		JobData Prepare(u32)
		{
			return JobData{};
		}
		static void Run(JobData*)
		{//algorith: keep a history ring buffer, use next entry for each iteration, check and free old entry allocation, do a new one, quit when total amount reached
			constexpr size_t Size64K = 64 * 1024;
			constexpr size_t Amount = 1024 * 1024 * 1024;
			constexpr size_t History = 8;

			PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::CacheLine);
			std::mt19937 gen;
			ThreadId seed = ThreadData::GetThisThreadData().GetThreadId();
			gen.seed((unsigned int)seed);
			std::uniform_int_distribution<> dis((int)(Size64K / 3), (int)(Size64K * 3 / 4));
			u8* slots[History];
			size_t sizes[History];
			memset(slots, 0, sizeof(slots));
			memset(sizes, 0, sizeof(sizes));
			size_t allocated = 0;
			size_t slotIndex = 0;
			int runIndex = 0;
			while (allocated < Amount)
			{
				u8* u8Ptr = slots[slotIndex];
				size_t sz = sizes[slotIndex];
				u8 w = (u8)sz;
				if (sz != 0)
				{
					bool bad = false;
					for (size_t iBytes = 0; iBytes < sz; ++iBytes)
					{
						if (u8Ptr[iBytes] != w)
						{
							bad = true;
							break;
						}
					}
					CheckAlways(!bad);
					alloc.resource()->deallocate(slots[slotIndex], 0);
				}
				size_t size = (size_t)dis(gen);
				std::byte* p = alloc.allocate(size);
				memset(p, (int)size, size);
				slots[slotIndex] = (u8*)p;
				sizes[slotIndex] = size;
				slotIndex = (slotIndex + 1) % History;
				allocated += size;
				++runIndex;
			}
			for (size_t iSlot = 0; iSlot < History; ++iSlot)
			{
				if (slots[iSlot])
				{
					alloc.resource()->deallocate(slots[iSlot], 0);
				}
			}
		}
		void Check()
		{
		}
	};
}

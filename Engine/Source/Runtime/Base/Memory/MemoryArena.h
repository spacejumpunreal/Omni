#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Prelude/PlatformDefs.h"
#include <cstring>

namespace Omni
{
class ScratchStack;

class MemoryArenaScope
{
private:
    FORCEINLINE MemoryArenaScope(ScratchStack& arena, u32 depth);

public:
    FORCEINLINE ~MemoryArenaScope();

private:
    ScratchStack& mArena;
    u32           mDepth;
    friend class ScratchStack;
};
class BASE_API ScratchStack
{
public:
    static constexpr u32 MaxDepth = 16;
    static constexpr u32 Alignment = OMNI_DEFAULT_ALIGNMENT;

public:
    ScratchStack();
    void Reset(u8* ptr, u32 size);
    u8*  Cleanup();
    bool IsClean();

    template<typename T>
    T* AllocArray(u32 count);

    FORCEINLINE u8* Allocate(u32 size);

    template<typename T>
    FORCEINLINE T*   AllocateAndInitWith(u32 count, const T* src);
    FORCEINLINE void Push();
    FORCEINLINE void Pop();

    [[nodiscard]] FORCEINLINE MemoryArenaScope PushScope();

    FORCEINLINE u32 GetUsedBytes()
    {
        return mUsedBytes;
    }
    FORCEINLINE u8* GetPtr()
    {
        return mPtr;
    }

protected:
    u8* mPtr;
    u32 mUsedBytes;
    u32 mTotalBytes;
    u32 mDepth;
    u32 mOffsets[MaxDepth];
    friend class MemoryArenaScope;
};

MemoryArenaScope::MemoryArenaScope(ScratchStack& arena, u32 depth) : mArena(arena), mDepth(depth)
{
}
MemoryArenaScope::~MemoryArenaScope()
{
    mArena.mDepth = mDepth;
    mArena.mUsedBytes = mArena.mOffsets[mDepth];
}
template<typename T>
T* ScratchStack::AllocArray(u32 count)
{
    return (T*)Allocate(count * sizeof(T));
}
u8* ScratchStack::Allocate(u32 size)
{
    CheckDebug(mDepth > 0);
    u8* ret = mPtr + mUsedBytes;
    mUsedBytes += AlignUpSize(size, Alignment);
    CheckDebug(mUsedBytes <= mTotalBytes);
    return ret;
}
template<typename T>
T* ScratchStack::AllocateAndInitWith(u32 count, const T* src)
{
    CheckDebug(mDepth > 0);
    auto ptr = (T*)Allocate(count * sizeof(T));
    std::memcpy(ptr, src, count * sizeof(T));
    return ptr;
}
void ScratchStack::Push()
{
    CheckDebug(mDepth < MaxDepth);
    mOffsets[mDepth++] = mUsedBytes;
}
void ScratchStack::Pop()
{
    CheckDebug(mDepth > 0);
    mUsedBytes = mOffsets[--mDepth];
}
MemoryArenaScope ScratchStack::PushScope()
{
    Push();
    return MemoryArenaScope(*this, mDepth - 1);
}

} // namespace Omni

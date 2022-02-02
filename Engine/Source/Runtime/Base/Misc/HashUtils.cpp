#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/Misc/HashUtils.h"


#include "External/xxHash/xxhash.c"


namespace Omni
{
u64 ComputeHash(u8* data, size_t bytes)
{
    return XXH3_64bits(data, bytes);
}
} // namespace Omni

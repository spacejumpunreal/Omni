#pragma once
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Container/PMRContainers.h"

namespace Omni
{

BASE_API void SetupUTF8Locale();
BASE_API size_t FromUTF8ToUTF16(const char* srcUtf8, size_t srcLength, char16_t* dstUtf16, size_t dstLength);
BASE_API size_t FromUTF16ToUTF8(const char16_t* srcUtf16, size_t srcLEngth, char* dstUtf8, size_t dstLength);

} // namespace Omni

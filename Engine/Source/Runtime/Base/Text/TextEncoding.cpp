#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Text/TextEncoding.h"
#include "Runtime/Base/Memory/MemoryDefs.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Prelude/SuppressWarning.h"


#include <clocale>
#include <cuchar>
#include <string>

namespace Omni
{

void SetupUTF8Locale()
{
    std::setlocale(LC_ALL, "en_US.UTF-8");
}

size_t FromUTF8ToUTF16(const char* srcUtf8, size_t srcLength, char16_t* dstUtf16, size_t dstLength)
{
    if (dstLength == 0)
        return 0;
    --dstLength;//for terminal null char
    std::mbstate_t state{}; // zero-initialized to initial state
    const char*    limit = srcUtf8 + srcLength;
    const char*    ptr = (const char*)srcUtf8;
    char16_t*      wptr = dstUtf16;

    size_t rc;
    while (dstLength > 0)
    {
        rc = std::mbrtoc16(wptr, ptr, limit - ptr, &state);
        if (rc == 0)
            break;
        --dstLength;
        if (rc == (size_t)-3)
        {
        }
        else if ((rc == (size_t)-2) || (rc == (size_t)-1))
        {
            CheckDebug(false, "incomplete string");
            break;
        }
        else
        {
            ptr += rc;
            ++wptr;
        }
    }
    *wptr = 0;
    return (wptr - dstUtf16);
}

size_t FromUTF16ToUTF8(const char16_t* srcUtf16, size_t srcLength, char* dstUtf8, size_t dstLength)
{
    if (dstLength == 0)
        return 0;
    --dstLength;
    std::mbstate_t state{};
    size_t         rc;
    char*          wptr = dstUtf8;
    char*          wlimit = wptr + dstLength;
    char           tmpStr[16];
    size_t         rcnt = 0;
    while (dstLength > 0 && rcnt < srcLength)
    {
        rc = std::c16rtomb(tmpStr, *(srcUtf16 + rcnt), &state);
        if (rc == (size_t)-1)
        {
            CheckDebug(false, "invalid string");
            break;
        }
        if (wptr + rc <= wlimit)
            memcpy(wptr, tmpStr, rc);
        else
            break;
        wptr += rc;
    }
    *wptr = 0;
    return wptr - dstUtf8;
}


} // namespace Omni

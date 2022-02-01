#pragma once


#define DECLARE_GFXAPI_REF_TYPE(RefTypeName, BaseType)                                                                 \
    struct RefTypeName : public BaseType                                                                               \
    {                                                                                                                  \
        using UnderlyingHandle = BaseType;                                                                             \
    }
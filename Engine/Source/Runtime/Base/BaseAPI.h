#pragma once
#include "Runtime/Prelude/APIDefine.h"

#if BASE_IS_DYNAMIC_LIBRARY
#if EXPORT_BASE
#define BASE_API EXPORT_API
#endif

#if IMPORT_BASE
#define BASE_API IMPORT_API
#endif

#else
#define BASE_API
#endif

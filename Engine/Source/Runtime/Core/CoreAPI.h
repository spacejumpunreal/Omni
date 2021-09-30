#pragma once
#include "Runtime/Prelude/APIDefine.h"

#if CORE_IS_DYNAMIC_LIBRARY
#if EXPORT_CORE
#define CORE_API EXPORT_API
#endif

#if IMPORT_CORE
#define CORE_API IMPORT_API
#endif

#else
#define CORE_API
#endif

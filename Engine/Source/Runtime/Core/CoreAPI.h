#pragma once
#include "Runtime/Prelude/APIDefine.h"

#if CORE_IS_DYNAMIC_LIBRARY
#define EXPORT_CORE_API OMNI_DLL_EXPORT
#define IMPORT_CORE_API OMNI_DLL_IMPORT
#else
#define EXPORT_CORE_API
#define IMPORT_CORE_API
#endif


#if EXPORT_CORE
#define CORE_API EXPORT_CORE_API
#elif IMPORT_CORE
#define CORE_API IMPORT_CORE_API
#else
#error("using header of module that is not imported")
#endif


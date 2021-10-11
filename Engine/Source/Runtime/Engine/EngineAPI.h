#pragma once
#include "Runtime/Prelude/APIDefine.h"

#if ENGINE_IS_DYNAMIC_LIBRARY
#define EXPORT_ENGINE_API OMNI_DLL_EXPORT
#define IMPORT_ENGINE_API OMNI_DLL_IMPORT
#else
#define EXPORT_ENGINE_API
#define IMPORT_ENGINE_API
#endif


#if EXPORT_ENGINE
#define ENGINE_API EXPORT_ENGINE_API
#elif IMPORT_ENGINE
#define ENGINE_API IMPORT_ENGINE_API
#else
#error("using header of module that is not imported")
#endif


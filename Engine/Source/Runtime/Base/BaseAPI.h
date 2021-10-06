#pragma once
#include "Runtime/Prelude/APIDefine.h"

#if BASE_IS_DYNAMIC_LIBRARY
#define EXPORT_BASE_API OMNI_DLL_EXPORT
#define IMPORT_BASE_API OMNI_DLL_IMPORT
#else
#define EXPORT_BASE_API
#define IMPORT_BASE_API
#endif


#if EXPORT_BASE
#define BASE_API EXPORT_BASE_API
#elif IMPORT_BASE
#define BASE_API IMPORT_BASE_API
#else
#error("using header of module that is not imported")
#endif


//the deal
/*
* # user
*   - define import/export in a file, but there's a problem, what about inferred dependency, so dependency resolved by build system is still needed
* # implementation
* # build tool
*/
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
#error("using header from a module that is not imported")
#endif


//the plan
/*
* # user
*   - declare import in build files
* # implementation
*   - declare export in build files
*   - include ModuleAPI in all .h for checking
* # build tool
*   - generate EXPORT_MODULE/IMPORT_MODULE defs
*/
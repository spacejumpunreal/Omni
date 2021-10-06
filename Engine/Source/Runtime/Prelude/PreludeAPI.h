#pragma once
#include "Runtime/Prelude/APIDefine.h"

#if PRELUDE_IS_DYNAMIC_LIBRARY
#define EXPORT_PRELUDE_API OMNI_DLL_EXPORT
#define IMPORT_PRELUDE_API OMNI_DLL_IMPORT
#else
#define EXPORT_PRELUDE_API
#define IMPORT_PRELUDE_API
#endif


#if EXPORT_PRELUDE
#define PRELUDE_API EXPORT_PRELUDE_API
#elif IMPORT_PRELUDE
#define PRELUDE_API IMPORT_PRELUDE_API
#else
#error("using header of module that is not imported")
#endif

#pragma once
#include "APIDefine.h"

#if PRELUDE_IS_DYNAMIC_LIBRARY
#if EXPORT_PRELUDE
#define PRELUDE_API EXPORT_API
#endif

#if IMPORT_PRELUDE
#define PRELUDE_API IMPORT_API
#endif

#else
#define PRELUDE_API
#endif
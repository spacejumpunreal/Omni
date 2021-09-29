#pragma once
#include "Omni.h"
#include "MacroUtils.h"
#if OMNI_CLANG
#include <experimental/memory_resource>
#else
#include <memory_resource>
#endif


#if OMNI_CLANG
#define StdPmr std::experimental::pmr
#else
#define StdPmr std::pmr
#endif


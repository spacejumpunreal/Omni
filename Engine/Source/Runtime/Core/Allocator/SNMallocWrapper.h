#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Prelude/SuppressWarning.h"

OMNI_PUSH_WARNING()
OMNI_SUPPRESS_WARNING_PADDED_DUE_TO_ALIGNMENT()
OMNI_SUPPRESS_WARNING_COND_EXPR_IS_CONSTANT()
#include "External/snmalloc/src/snmalloc.h"
OMNI_POP_WARNING()


#pragma once

#if __clang__

#define OMNI_PUSH_WARNING() _Pragma("clang diagnostic push")
#define OMNI_POP_WARNING() _Pragma("clang diagnostic pop")
#define OMNI_SUPPRESS_WARNING_INTEGER_CONVERSION() \
	_Pragma("clang diagnostic ignored \"-Wimplicit-int-conversion\"") \
	_Pragma("clang diagnostic ignored \"-Wshorten-64-to-32\"")

#define OMNI_SUPPRESS_WARNING_NAME_HIDDEN()
#define OMNI_SUPPRESS_WARNING_UNUSED_PARAMTER()
#define OMNI_SUPPRESS_WARNING_CONSTANT_VALUE_TRUNCATED()
#define OMNI_SUPPRESS_WARNING_MISSING_OVERRIDE() _Pragma("clang diagnostic ignored \"-Winconsistent-missing-override\"")
#define OMNI_SUPPRESS_WARNING_INLINE_NEW_DELETE() _Pragma("clang diagnostic ignored \"-Winline-new-delete\"")
#define OMNI_SUPPRESS_WARNING_EXCEPTION_SPEC_MISMATCH() _Pragma("clang diagnostic ignored \"-Wimplicit-exception-spec-mismatch\"")
#define OMNI_SUPPRESS_WARNING_NEW_RETURN_NULL() _Pragma("clang diagnostic ignored \"-Wnew-returns-null\"")
#define OMNI_SUPPRESS_WARNING_PADDED_DUE_TO_ALIGNMENT()
#define OMNI_SUPPRESS_WARNING_COND_EXPR_IS_CONSTANT()
#define OMNI_SUPPRESS_WARNING_NAMELESS_UNION_STRUCT()

#elif _MSC_VER
#define OMNI_PUSH_WARNING() __pragma(warning(push))
#define OMNI_POP_WARNING() __pragma(warning(pop))
#define OMNI_SUPPRESS_WARNING_INTEGER_CONVERSION() __pragma(warning(disable: 4244))
#define OMNI_SUPPRESS_WARNING_NAME_HIDDEN() __pragma(warning(disable: 4456 4458))
#define OMNI_SUPPRESS_WARNING_UNUSED_PARAMTER() __pragma(warning(disable: 4100))
#define OMNI_SUPPRESS_WARNING_CONSTANT_VALUE_TRUNCATED() __pragma(warning(disable: 4309))
#define OMNI_SUPPRESS_WARNING_MISSING_OVERRIDE() 
#define OMNI_SUPPRESS_WARNING_INLINE_NEW_DELETE() __pragma(warning(disable: 4595))
#define OMNI_SUPPRESS_WARNING_EXCEPTION_SPEC_MISMATCH()
#define OMNI_SUPPRESS_WARNING_NEW_RETURN_NULL()
#define OMNI_SUPPRESS_WARNING_PADDED_DUE_TO_ALIGNMENT() __pragma(warning(disable: 4324))
#define OMNI_SUPPRESS_WARNING_COND_EXPR_IS_CONSTANT() __pragma(warning(disable: 4127))
#define OMNI_SUPPRESS_WARNING_NAMELESS_UNION_STRUCT() __pragma(warning(disable : 4201))

#endif


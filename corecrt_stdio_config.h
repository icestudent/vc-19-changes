//
// corecrt_stdio_config.h
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Per-module <stdio.h> configuration.
//
#pragma once

#include <corecrt.h>

#if !defined __midl && !defined RC_INVOKED

_CRT_BEGIN_C_HEADER



#ifndef _CRT_STDIO_INLINE
    #define _CRT_STDIO_INLINE __inline
#endif

#if defined _M_IX86
    #define _CRT_INTERNAL_STDIO_SYMBOL_PREFIX "_"
#elif defined _M_AMD64 || defined _M_ARM
    #define _CRT_INTERNAL_STDIO_SYMBOL_PREFIX ""
#else
    #error Unsupported architecture
#endif



// Predefine _CRT_STDIO_LEGACY_WIDE_SPECIFIERS to use the legacy behavior for
// the wide string printf and scanf functions (%s, %c, and %[] specifiers).
//
// Predefine _CRT_STDIO_ARBITRARY_WIDE_SPECIFIERS when building code that does
// not use these format specifiers without a length modifier and thus can be
// used with either the legacy or the conforming (default) mode.  (This option
// is intended for use by static libraries).
#if defined _CRT_STDIO_LEGACY_WIDE_SPECIFIERS
    #if !defined _M_CEE_PURE
        #pragma comment(lib, "legacy_stdio_wide_specifiers")
        #pragma comment(linker, "/include:" _CRT_INTERNAL_STDIO_SYMBOL_PREFIX "__scrt_stdio_legacy_wide_specifiers")
    #endif
#endif

#if defined __cplusplus && !defined _CRT_STDIO_ARBITRARY_WIDE_SPECIFIERS
    #ifdef _CRT_STDIO_LEGACY_WIDE_SPECIFIERS
        #pragma detect_mismatch("_CRT_STDIO_LEGACY_WIDE_SPECIFIERS", "1")
    #else
        #pragma detect_mismatch("_CRT_STDIO_LEGACY_WIDE_SPECIFIERS", "0")
    #endif
#endif

#if defined _CRT_INTERNAL_STDIO_LEGACY_MSVCRT_COMPATIBILITY
    #ifndef BUILD_WINDOWS
        #error This option supported for interal use only
    #endif
    #if !defined _M_CEE_PURE
        #pragma comment(lib, "legacy_stdio_msvcrt_compatibility")
        #pragma comment(linker, "/include:" _CRT_INTERNAL_STDIO_SYMBOL_PREFIX "__scrt_stdio_legacy_msvcrt_compatibility")
    #endif
#endif



_Check_return_ _Ret_notnull_
__inline unsigned __int64* __CRTDECL __local_stdio_printf_options(void)
{
    static unsigned __int64 _OptionsStorage;
    return &_OptionsStorage;
}

_Check_return_ _Ret_notnull_
__inline unsigned __int64* __CRTDECL __local_stdio_scanf_options(void)
{
    static unsigned __int64 _OptionsStorage;
    return &_OptionsStorage;
}

#define _CRT_INTERNAL_LOCAL_PRINTF_OPTIONS (*__local_stdio_printf_options())
#define _CRT_INTERNAL_LOCAL_SCANF_OPTIONS  (*__local_stdio_scanf_options ())



#define _CRT_INTERNAL_PRINTF_LEGACY_VSPRINTF_NULL_TERMINATION (1ULL << 0)
#define _CRT_INTERNAL_PRINTF_STANDARD_SNPRINTF_BEHAVIOR       (1ULL << 1)
#define _CRT_INTERNAL_PRINTF_LEGACY_WIDE_SPECIFIERS           (1ULL << 2)
#define _CRT_INTERNAL_PRINTF_LEGACY_FN_LENGTH_MODIFIERS       (1ULL << 3)
#define _CRT_INTERNAL_PRINTF_LEGACY_THREE_DIGIT_EXPONENTS     (1ULL << 4)
#define _CRT_INTERNAL_PRINTF_LEGACY_INF_NAN_FORMATTING        (1ULL << 5)


#define _CRT_INTERNAL_SCANF_SECURECRT              (1ULL << 0)
#define _CRT_INTERNAL_SCANF_LEGACY_WIDE_SPECIFIERS (1ULL << 1)



_CRT_END_C_HEADER

#endif // !defined __midl && !defined RC_INVOKED

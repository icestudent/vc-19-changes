//
// varargs.h
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// XENIX-style macros for accessing arguments of a function which takes a
// variable number of arguments.  Use the C Standard <stdarg.h> instead.
//
#pragma once
#define _INC_VARARGS

#include <vcruntime.h>

_CRT_BEGIN_C_HEADER



#if __STDC__
    #error varargs.h incompatible with ANSI (use stdarg.h)
#endif

#ifndef va_arg
    #if defined _M_CEE

        #error varargs.h not supported when targetting _M_CEE (use stdarg.h)

    #elif defined _M_IX86

        // Define a macro to compute the size of a type, variable or expression,
        // rounded up to the nearest multiple of sizeof(int). This number is its
        // size as function argument (Intel architecture). Note that the macro
        // depends on sizeof(int) being a power of 2!
        #define _INTSIZEOF(n)   ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))

        #define va_dcl          va_list va_alist;
        #define va_start(ap)    ap = (va_list)&va_alist
        #define va_arg(ap, t)   (*(t*)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
        #define va_end(ap)      ap = (va_list)0

    #elif defined _M_ARM

        #define _VA_ALIGN       4
        #define _APALIGN(t,ap)  (((va_list)0 - (ap)) & (__alignof(t) - 1))
        #define va_dcl          va_list va_alist;

        #define _SLOTSIZEOF(t)  ((sizeof(t) + _VA_ALIGN - 1) & ~(_VA_ALIGN - 1))
        #define va_start(ap)    (ap = (va_list)&va_alist)
        #define va_arg(ap,t)    (*(t*)((ap += _SLOTSIZEOF(t) + _APALIGN(t,ap)) - _SLOTSIZEOF(t)))
        #define va_end(ap)      (ap = (va_list)0)

    #elif defined _M_X64

        void __cdecl __va_start(va_list *, ...);
        #define va_dcl          va_list va_alist;
        #define va_start(ap)    (__va_start(&ap, 0))
        #define va_arg(ap, t)                                                    \
            ((sizeof(t) > sizeof(__int64) || (sizeof(t) & (sizeof(t) - 1)) != 0) \
                ? **(t**)((ap += sizeof(__int64)) - sizeof(__int64))             \
                :  *(t* )((ap += sizeof(__int64)) - sizeof(__int64)))
        #define va_end(ap)      (ap = (va_list)0)

    #endif
#endif // va_arg

_CRT_END_C_HEADER

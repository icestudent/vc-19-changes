//
// vadefs.h
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Definitions of macro helpers used by <stdarg.h>.  This is the topmost header
// in the CRT header lattice, and is always the first CRT header to be included,
// explicitly or implicitly.  Therefore, this header also has several definitions
// that are used throughout the CRT.
//
#pragma once
#define _INC_VADEFS

#define _CRT_PACKING 8
#pragma pack(push, _CRT_PACKING)

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _W64
    #if !defined __midl && (defined _X86_ || defined _M_IX86)
        #define _W64 __w64
    #else
        #define _W64
    #endif
#endif

#ifndef _UINTPTR_T_DEFINED
    #define _UINTPTR_T_DEFINED
    #ifdef _WIN64
        typedef unsigned __int64  uintptr_t;
    #else
        typedef _W64 unsigned int uintptr_t;
    #endif
#endif

#ifndef _VA_LIST_DEFINED
    #define _VA_LIST_DEFINED
    #ifdef _M_CEE_PURE
        typedef System::ArgIterator va_list;
    #else
        typedef char *  va_list;
    #endif
#endif

#ifdef __cplusplus
    #define _ADDRESSOF(v) (&reinterpret_cast<const char &>(v))
#else
    #define _ADDRESSOF(v) (&(v))
#endif

#if defined _M_ARM && !defined _M_CEE_PURE
    #define _VA_ALIGN       4
    #define _SLOTSIZEOF(t)  ((sizeof(t) + _VA_ALIGN - 1) & ~(_VA_ALIGN - 1))
    #define _APALIGN(t,ap)  (((va_list)0 - (ap)) & (__alignof(t) - 1))
#else
    #define _SLOTSIZEOF(t)  (sizeof(t))
    #define _APALIGN(t,ap)  (__alignof(t))
#endif

#if defined _M_CEE_PURE || (defined _M_CEE && !defined _M_ARM)

    void  __cdecl __va_start(va_list*, ...);
    void* __cdecl __va_arg(va_list*, ...);
    void  __cdecl __va_end(va_list*);

    #define _crt_va_start(ap, v) (__va_start(&ap, _ADDRESSOF(v), _SLOTSIZEOF(v), __alignof(v), _ADDRESSOF(v)))
    #define _crt_va_arg(ap, t)   (*(t *)__va_arg(&ap, _SLOTSIZEOF(t), _APALIGN(t,ap), (t*)0))
    #define _crt_va_end(ap)      (__va_end(&ap))

#elif defined _M_IX86

    #define _INTSIZEOF(n)         ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))

    #define _crt_va_start(ap, v)  (ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v))
    #define _crt_va_arg(ap, t)    (*(t*)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
    #define _crt_va_end(ap)       (ap = (va_list)0)

#elif defined _M_ARM

    #ifdef __cplusplus
        extern void __cdecl __va_start(va_list*, ...);
        #define _crt_va_start(ap, v) (__va_start(&ap, _ADDRESSOF(v), _SLOTSIZEOF(v), _ADDRESSOF(v)))
    #else
        #define _crt_va_start(ap, v) (ap = (va_list)_ADDRESSOF(v) + _SLOTSIZEOF(v))
    #endif

    #define _crt_va_arg(ap, t) (*(t*)((ap += _SLOTSIZEOF(t) + _APALIGN(t,ap)) - _SLOTSIZEOF(t)))

    #define _crt_va_end(ap)    (ap = (va_list)0)

#elif defined _M_X64

    extern void __cdecl __va_start(va_list* , ...);

    #define _crt_va_start(ap, x) (__va_start(&ap, x))
    #define _crt_va_arg(ap, t)                                               \
        ((sizeof(t) > sizeof(__int64) || (sizeof(t) & (sizeof(t) - 1)) != 0) \
            ? **(t**)((ap += sizeof(__int64)) - sizeof(__int64))             \
            :  *(t* )((ap += sizeof(__int64)) - sizeof(__int64)))
    #define _crt_va_end(ap)      (ap = (va_list)0)

#endif

#ifdef __cplusplus
} // extern "C++"
#endif

#pragma pack(pop)

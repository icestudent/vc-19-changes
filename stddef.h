//
// stddef.h
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// The C <stddef.h> Standard Library header.
//
#pragma once
#define _INC_STDDEF

#include <corecrt.h>

_CRT_BEGIN_C_HEADER



#ifdef __cplusplus
    namespace std
    {
        typedef decltype(__nullptr) nullptr_t;
    }

    using ::std::nullptr_t;
#endif



// Declare errno functions and macros
_ACRTIMP int* __cdecl _errno(void);
#define errno (*_errno())

_ACRTIMP errno_t __cdecl _set_errno(_In_ int _Value);
_ACRTIMP errno_t __cdecl _get_errno(_Out_ int* _Value);



// Define offsetof macro
#ifdef __cplusplus

    #ifdef _WIN64
        #define offsetof(s,m) (size_t)( (ptrdiff_t)&reinterpret_cast<const volatile char&>((((s *)0)->m)) )
    #else
        #define offsetof(s,m) (size_t)&reinterpret_cast<const volatile char&>((((s *)0)->m))
    #endif

#else

    #ifdef _WIN64
        #define offsetof(s,m)   (size_t)( (ptrdiff_t)&(((s *)0)->m) )
    #else
        #define offsetof(s,m)   (size_t)&(((s *)0)->m)
    #endif

#endif

_ACRTIMP extern unsigned long  __cdecl __threadid(void);
#define _threadid (__threadid())
_ACRTIMP extern uintptr_t __cdecl __threadhandle(void);



_CRT_END_C_HEADER

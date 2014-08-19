//
// eh.h
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// User-includable hedaer for exception handling.
//
#pragma once
#define _INC_EH

#include <vcruntime.h>

#ifndef RC_INVOKED

#pragma pack(push, _CRT_PACKING)

#ifndef __cplusplus
    #error "eh.h is only for C++!"
#endif

// terminate_handler is the standard name; terminate_function is supported for
// historical reasons
typedef void (__CRTDECL* terminate_function )();
typedef void (__CRTDECL* terminate_handler  )();
typedef void (__CRTDECL* unexpected_function)();
typedef void (__CRTDECL* unexpected_handler )();

#ifdef _M_CEE
    typedef void (__clrcall* __terminate_function_m)();
    typedef void (__clrcall* __terminate_handler_m)();
    typedef void (__clrcall* __unexpected_function_m)();
    typedef void (__clrcall* __unexpected_handler_m)();
#endif

struct _EXCEPTION_POINTERS;
#ifndef _M_CEE_PURE
    typedef void (__cdecl* _se_translator_function)(unsigned int, struct _EXCEPTION_POINTERS*);
#endif

_VCRTIMP __declspec(noreturn) void __cdecl terminate(void);
_VCRTIMP __declspec(noreturn) void __cdecl unexpected(void);

_VCRTIMP int __cdecl _is_exception_typeof(_In_ const type_info &_Type, _In_ struct _EXCEPTION_POINTERS * _ExceptionPtr);

#ifndef _M_CEE_PURE 
    extern "C" _VCRTIMP terminate_function __cdecl set_terminate(_In_opt_ terminate_function _NewPtFunc);
    extern "C" _VCRTIMP terminate_function __cdecl _get_terminate(void);

    extern "C" _VCRTIMP unexpected_function __cdecl set_unexpected(_In_opt_ unexpected_function _NewPtFunc);
    extern "C" _VCRTIMP unexpected_function __cdecl _get_unexpected(void);

    extern "C" _VCRTIMP _se_translator_function __cdecl _set_se_translator(_In_opt_ _se_translator_function _NewPtFunc);
#endif



_VCRTIMP bool __cdecl __uncaught_exception();

// These overload helps in resolving NULL
#ifdef _M_CEE
    _VCRTIMP terminate_function __cdecl set_terminate(_In_ int _Zero);
    _VCRTIMP unexpected_function __cdecl set_unexpected(_In_ int _Zero);
#endif

#pragma pack(pop)
#endif // RC_INVOKED

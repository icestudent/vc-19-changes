//
// stdarg.h
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// The C Standard Library <stdarg.h> header.
//
#pragma once
#define _INC_STDARG

#include <vcruntime.h>

_CRT_BEGIN_C_HEADER



#define va_start __crt_va_start
#define va_arg   __crt_va_arg
#define va_end   __crt_va_end

_VCRTIMP void __cdecl _vacopy(_Out_ va_list*, _In_ va_list);
#define va_copy(apd, aps) _vacopy(&(apd), aps)



_CRT_END_C_HEADER

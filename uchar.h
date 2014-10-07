//
// uchar.h
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//

#pragma once
#define _UCHAR

#include <corecrt.h>

_CRT_BEGIN_C_HEADER

#define __STDC_UTF_16__
#define __STDC_UTF_32__

typedef unsigned short _Char16_t;
typedef unsigned int _Char32_t;

#ifndef __cplusplus
    typedef unsigned short char16_t;
    typedef unsigned int char32_t;
#endif // __cplusplus

_Check_return_ _ACRTIMP size_t __cdecl mbrtoc16(_Out_opt_ _Char16_t *_Pc16, _In_reads_or_z_opt_(_N) const char *_S, _In_ size_t _N, _Inout_opt_ mbstate_t *_Ps);
_Check_return_ _ACRTIMP size_t __cdecl c16rtomb(_Out_writes_opt_(6) char *_S, _In_ _Char16_t _C16, _Inout_opt_ mbstate_t *_Ps);

_Check_return_ _ACRTIMP size_t __cdecl mbrtoc32(_Out_opt_ _Char32_t *_Pc32, _In_reads_or_z_opt_(_N) const char *_S, _In_ size_t _N, _Inout_opt_ mbstate_t *_Ps);
_Check_return_ _ACRTIMP size_t __cdecl c32rtomb(_Out_writes_opt_(6) char *_S, _In_ _Char32_t _C32, _Inout_opt_ mbstate_t *_Ps);

_CRT_END_C_HEADER

/*
 * Copyright (c) 1992-2013 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 V6.40:0009 */

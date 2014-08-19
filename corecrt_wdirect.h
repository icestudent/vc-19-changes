//
// corecrt_wdirect.h
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// This file declares the wide character (wchar_t) directory functionality, shared
// by <direct.h> and <wchar.h>.
//
#pragma once

#include <corecrt.h>

_CRT_BEGIN_C_HEADER



#ifdef _CRT_USE_WINAPI_FAMILY_DESKTOP_APP

    #pragma push_macro("_wgetcwd")
    #pragma push_macro("_wgetdcwd")
    #undef _wgetcwd
    #undef _wgetdcwd

    _Check_return_ _Ret_maybenull_z_
    _DCRTIMP _CRTALLOCATOR wchar_t* __cdecl _wgetcwd(
        _Out_writes_opt_(_SizeInWords) wchar_t* _DstBuf,
        _In_                           int      _SizeInWords
        );

    _Check_return_ _Ret_maybenull_z_
    _DCRTIMP _CRTALLOCATOR wchar_t* __cdecl _wgetdcwd(
        _In_                           int      _Drive,
        _Out_writes_opt_(_SizeInWords) wchar_t* _DstBuf,
        _In_                           int      _SizeInWords
        );

    #define _wgetdcwd_nolock  _wgetdcwd

    #pragma pop_macro("_wgetcwd")
    #pragma pop_macro("_wgetdcwd")

    _Check_return_
    _DCRTIMP int __cdecl _wchdir(
        _In_z_ wchar_t const* _Path
        );

#endif // _CRT_USE_WINAPI_FAMILY_DESKTOP_APP



_Check_return_
_ACRTIMP int __cdecl _wmkdir(
    _In_z_ wchar_t const* _Path
    );

_Check_return_
_ACRTIMP int __cdecl _wrmdir(
    _In_z_ wchar_t const* _Path
    );



_CRT_END_C_HEADER

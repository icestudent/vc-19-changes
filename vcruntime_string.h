//
// vcruntime_string.h
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// <string.h> functionality that is implemented in the VCRuntime.
//
#pragma once

#include <vcruntime.h>

#ifndef __midl

_CRT_BEGIN_C_HEADER



_VCRTIMP void* __cdecl _memccpy(
    _Out_writes_bytes_opt_(_MaxCount) void*       _Dst,
    _In_                              void const* _Src,
    _In_                              int         _Val,
    _In_                              size_t      _MaxCount
    );

_Check_return_
_VCRTIMP void _CONST_RETURN* __cdecl memchr(
    _In_reads_bytes_opt_(_MaxCount) void const* _Buf,
    _In_                            int         _Val,
    _In_                            size_t      _MaxCount
    );

_Check_return_
int __cdecl memcmp(
    _In_reads_bytes_(_Size) void const* _Buf1,
    _In_reads_bytes_(_Size) void const* _Buf2,
    _In_                    size_t      _Size
    );

_CRT_INSECURE_DEPRECATE_MEMORY(memcpy_s)
_Post_equal_to_(_Dst)
_At_buffer_(
    (unsigned char*)_Dst,
    _Iter_,
    _Size,
    _Post_satisfies_(((unsigned char*)_Dst)[_Iter_] == ((unsigned char*)_Src)[_Iter_])
)
void* __cdecl memcpy(
    _Out_writes_bytes_all_(_Size) void* _Dst,
    _In_reads_bytes_(_Size)       void const* _Src,
    _In_                          size_t      _Size
    );

_CRT_INSECURE_DEPRECATE_MEMORY(memmove_s)
_VCRTIMP void* __cdecl memmove(
    _Out_writes_bytes_all_opt_(_Size) void*       _Dst,
    _In_reads_bytes_opt_(_Size)       void const* _Src,
    _In_                              size_t      _Size
    );

_Post_equal_to_(_Dst)
_At_buffer_(
    (unsigned char*)_Dst,
    _Iter_,
    _Size,
    _Post_satisfies_(((unsigned char*)_Dst)[_Iter_] == _Val)
)
void* __cdecl memset(
    _Out_writes_bytes_all_(_Size) void*  _Dst,
    _In_                          int    _Val,
    _In_                          size_t _Size
    );

_Check_return_
_VCRTIMP char _CONST_RETURN* __cdecl strchr(
    _In_z_ char const* _Str,
    _In_   int         _Val
    );

_Check_return_
int __cdecl strcmp(
    _In_z_ char const* _Str1,
    _In_z_ char const* _Str2
    );

char* __cdecl strcpy(
    _Out_writes_z_(_String_length_(_Source) + 1) char*       _Dest,
    _In_z_                                       char const* _Source
    );

_Check_return_
_VCRTIMP size_t __cdecl strcspn(
    _In_z_ char const* _Str,
    _In_z_ char const* _Control
    );

_Check_return_
size_t __cdecl strlen(
    _In_z_ char const* _Str
    );

_VCRTIMP char* __cdecl strncat(
    _Inout_updates_z_(_Count) char*       _Dest,
    _In_reads_or_z_(_Count)   char const* _Source,
    _In_                      size_t      _Count
    );

_Check_return_
_VCRTIMP int __cdecl strncmp(
    _In_reads_or_z_(_MaxCount) char const* _Str1,
    _In_reads_or_z_(_MaxCount) char const* _Str2,
    _In_                       size_t      _MaxCount
    );

_VCRTIMP char* __cdecl _strnset(
    _Inout_updates_z_(_Count) char*  _Dest,
    _In_                      int    _Val,
    _In_                      size_t _Count
    );

_Check_return_
_VCRTIMP char _CONST_RETURN* __cdecl strpbrk(
    _In_z_ char const* _Str,
    _In_z_ char const* _Control
    );

_Check_return_
_VCRTIMP char _CONST_RETURN* __cdecl strrchr(
    _In_z_ char const* _Str,
    _In_   int         _Ch
    );

_VCRTIMP char* __cdecl _strrev(
    _Inout_z_ char* _Str
    );

char* __cdecl _strset(
    _Inout_z_ char* _Dest,
    _In_      int   _Value
    );

_Check_return_
_VCRTIMP size_t __cdecl strspn(
    _In_z_ char const* _Str,
    _In_z_ char const* _Control
    );

_Check_return_ _Ret_maybenull_
_VCRTIMP char _CONST_RETURN* __cdecl strstr(
    _In_z_ char const* _Str,
    _In_z_ char const* _SubStr
    );

_Check_return_
_When_(return != NULL, _Ret_range_(_Str, _Str+_String_length_(_Str)-1))
_VCRTIMP wchar_t _CONST_RETURN* __cdecl wcschr(
    _In_z_ wchar_t const* _Str,
    _In_   wchar_t        _Ch
    );

_Check_return_
_VCRTIMP wchar_t _CONST_RETURN* __cdecl wcsrchr(
    _In_z_ wchar_t const* _Str,
    _In_   wchar_t        _Ch
    );

_Check_return_ _Ret_maybenull_
_When_(return != NULL, _Ret_range_(_Str, _Str+_String_length_(_Str)-1))
_VCRTIMP wchar_t _CONST_RETURN* __cdecl wcsstr(
    _In_z_ wchar_t const* _Str,
    _In_z_ wchar_t const* _SubStr
    );



_CRT_END_C_HEADER

#endif // __midl

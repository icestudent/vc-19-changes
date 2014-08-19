//
// vcruntime_new.h
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Declarations and definitions of memory management functions in the VCRuntime.
//
#pragma once

#ifdef __cplusplus

#include <vcruntime.h>

#pragma pack(push, _CRT_PACKING)

#pragma warning(push)
#pragma warning(disable: 4985) // attributes not present on previous declaration

#pragma push_macro("new")
#undef new

#ifndef __NOTHROW_T_DEFINED
#define __NOTHROW_T_DEFINED
    namespace std
    {
        struct nothrow_t { };

        extern nothrow_t const nothrow;
    }
#endif

_Ret_notnull_ _Post_writable_byte_size_(_Size)
_VCRT_ALLOCATOR void* __CRTDECL operator new(
    size_t _Size
    );

_Ret_maybenull_ _Success_(return != NULL) _Post_writable_byte_size_(_Size)
_VCRT_ALLOCATOR void* __CRTDECL operator new(
    size_t                _Size,
    std::nothrow_t const&
    ) throw();

_Ret_notnull_ _Post_writable_byte_size_(_Size)
_VCRT_ALLOCATOR void* __CRTDECL operator new[](
    size_t _Size
    );

_Ret_maybenull_ _Success_(return != NULL) _Post_writable_byte_size_(_Size)
_VCRT_ALLOCATOR void* __CRTDECL operator new[](
    size_t                _Size,
    std::nothrow_t const&
    ) throw();

void __CRTDECL operator delete(
    void* _Block
    );

void __CRTDECL operator delete(
    void* _Block,
    std::nothrow_t const&
    ) throw();

void __CRTDECL operator delete[](
    void* _Block
    );

void __CRTDECL operator delete[](
    void* _Block,
    std::nothrow_t const&
    ) throw();

#ifndef _MFC_OVERRIDES_NEW

    _Check_return_ _Ret_notnull_ _Post_writable_byte_size_(_Size)
    _VCRT_ALLOCATOR void* __CRTDECL operator new(
        _In_   size_t      _Size,
        _In_   int         _BlockUse,
        _In_z_ char const* _FileName,
        _In_   int         _LineNumber
        );

    _Check_return_ _Ret_notnull_ _Post_writable_byte_size_(_Size)
    _VCRT_ALLOCATOR void* __CRTDECL operator new[](
        _In_   size_t      _Size,
        _In_   int         _BlockUse,
        _In_z_ char const* _FileName,
        _In_   int         _LineNumber
        );

    void __CRTDECL operator delete(
        void*       _Block,
        int         _BlockUse,
        char const* _FileName,
        int         _LineNumber
        );

    void __CRTDECL operator delete[](
        void*       _Block,
        int         _BlockUse,
        char const* _FileName,
        int         _LineNumber
        );

#endif

#pragma pop_macro("new")

#pragma warning(pop)
#pragma pack(pop)

#endif // __cplusplus

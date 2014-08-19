//
// corecrt_startup.h
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Declarations for the CoreCRT startup functionality, used while initializing
// the CRT and during app startup and termination.
//
// CRT_REFACTOR TODO Clean up this header
//
#pragma once

#include <corecrt.h>
#include <math.h>
#include <stdbool.h>
#include <vcruntime_startup.h>

_CRT_BEGIN_C_HEADER



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Exception Filters for main() and DllMain()
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
struct _EXCEPTION_POINTERS;

_ACRTIMP int __cdecl _seh_filter_dll(
    _In_ unsigned long               _ExceptionNum,
    _In_ struct _EXCEPTION_POINTERS* _ExceptionPtr
    );

_ACRTIMP int __cdecl _seh_filter_exe(
    _In_ unsigned long               _ExceptionNum,
    _In_ struct _EXCEPTION_POINTERS* _ExceptionPtr
    );



// Prototype, variables and constants which determine how error messages are
// written out.
typedef enum _crt_app_type
{
    _crt_unknown_app,
    _crt_console_app,
    _crt_gui_app
} _crt_app_type;

_ACRTIMP _crt_app_type __cdecl _query_app_type(void);
_ACRTIMP void __cdecl _set_app_type(_In_ _crt_app_type _Type);


int __cdecl _is_c_termination_complete(void); // Not exported; static only


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Arguments API for main() et al.
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
_ACRTIMP errno_t __cdecl _configure_narrow_argv(_In_ _crt_argv_mode mode);
_ACRTIMP errno_t __cdecl _configure_wide_argv  (_In_ _crt_argv_mode mode);

char*    __CRTDECL _get_narrow_winmain_command_line(void);
wchar_t* __CRTDECL _get_wide_winmain_command_line  (void);

_ACRTIMP char**    __cdecl __p__acmdln(void);
_ACRTIMP wchar_t** __cdecl __p__wcmdln(void);

#if defined _ACRT_BUILD || !defined _DLL
    extern char*    _acmdln;
    extern wchar_t* _wcmdln;
#else
    #define _acmdln (*__p__acmdln())
    #define _wcmdln (*__p__wcmdln())
#endif



// These point to the initial environment that was passed to main or wmain.
#ifndef _M_CEE_PURE
    _DCRTIMP extern wchar_t** __dcrt_initial_wide_environment;
    _DCRTIMP extern char**    __dcrt_initial_narrow_environment;
#endif

_DCRTIMP int __cdecl __dcrt_initialize_narrow_environment_nolock(void);
_DCRTIMP int __cdecl __dcrt_initialize_wide_environment_nolock(void);

_DCRTIMP char**    __cdecl __dcrt_get_or_create_narrow_environment_nolock(void);
_DCRTIMP wchar_t** __cdecl __dcrt_get_or_create_wide_environment_nolock(void);


_ACRTIMP void __setusermatherr(int (__cdecl*)(struct _exception*));



typedef void (__cdecl* _PVFV)(void);
typedef int  (__cdecl* _PIFV)(void);
typedef void (__cdecl* _PVFI)(int);

#ifndef _M_CEE
    _ACRTIMP void __cdecl _initterm  (_PVFV* _First, _PVFV* _Last);
    _ACRTIMP int  __cdecl _initterm_e(_PIFV* _First, _PIFV* _Last);
#endif



typedef int (__CRTDECL* _onexit_t)(void);

typedef struct _onexit_table_t
{
    _PVFV* _first;
    _PVFV* _last;
    _PVFV* _end;
} _onexit_table_t;

_ACRTIMP int __cdecl _initialize_onexit_table(_onexit_table_t* table);
_ACRTIMP int __cdecl _register_onexit_function(_onexit_table_t* table, _onexit_t function);
_ACRTIMP int __cdecl _execute_onexit_table(_onexit_table_t* table);




bool __cdecl __acrt_initialize(void);
bool __cdecl __acrt_uninitialize(_In_ bool _Terminating);
bool __cdecl __acrt_uninitialize_critical(void);
bool __cdecl __acrt_thread_attach(void);
bool __cdecl __acrt_thread_detach(void);

bool __cdecl __dcrt_initialize(void);
bool __cdecl __dcrt_uninitialize(_In_ bool _Terminating);
bool __cdecl __dcrt_uninitialize_critical(void);
bool __cdecl __dcrt_thread_attach(void);
bool __cdecl __dcrt_thread_detach(void);


_CRT_END_C_HEADER

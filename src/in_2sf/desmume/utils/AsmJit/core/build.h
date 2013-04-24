// [AsmJit]
// Complete JIT Assembler for C++ Language.
//
// [License]
// Zlib - See COPYING file in this package.

// [Guard]
#ifndef _ASMJIT_CORE_BUILD_H
#define _ASMJIT_CORE_BUILD_H

// [Include]
#include "../Config.h"

#ifdef ASMJIT_EXPORTS
# if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
# endif // _MSC_VER
#endif // ASMJIT_EXPORTS

// Here should be optional include files that's needed fo successfuly
// use macros defined here. Remember, AsmJit uses only AsmJit namespace
// and all macros are used within it.
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <new>

// ============================================================================
// [AsmJit - OS]
// ============================================================================

#if defined(WINDOWS) || defined(_WINDOWS) || defined(__WINDOWS__) || defined(_WIN32) || defined(_WIN64)
# define ASMJIT_WINDOWS
#elif defined(__linux__) || defined(__unix__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__BSD__) || defined(__FREEBSD__) || defined(__APPLE__)
# define ASMJIT_POSIX
#else
# warning "AsmJit - Can't match operating system, using ASMJIT_POSIX"
# define ASMJIT_POSIX
#endif

// ============================================================================
// [AsmJit - Architecture]
// ============================================================================

// define it only if it's not defined. In some systems we can
// use -D command in compiler to bypass this autodetection.
#if !defined(ASMJIT_X86) && !defined(ASMJIT_X64)
# if defined(__x86_64__) || defined(__LP64) || defined(__IA64__) || defined(_M_X64) || defined(_WIN64) 
#  define ASMJIT_X64 // x86-64
# else
// _M_IX86, __INTEL__, __i386__
#  define ASMJIT_X86
# endif
#endif

// ============================================================================
// [AsmJit - API]
// ============================================================================

// Make AsmJit as shared library by default.
#ifndef ASMJIT_API
# ifdef ASMJIT_WINDOWS
#  ifdef __GNUC__
#   ifdef ASMJIT_EXPORTS
#    define ASMJIT_API __attribute__((dllexport))
#   else
#    define ASMJIT_API __attribute__((dllimport))
#   endif // ASMJIT_EXPORTS
#  else
#   ifdef ASMJIT_EXPORTS
#    define ASMJIT_API __declspec(dllexport)
#   else
#    define ASMJIT_API __declspec(dllimport)
#   endif // ASMJIT_EXPORTS
#  endif // __GNUC__
# else
#  ifdef (__GNUC__
#   if __GNUC__ >= 4
#    define ASMJIT_API __attribute__((visibility("default")))
#    define ASMJIT_VAR extern ASMJIT_API
#   endif // __GNUC__ >= 4
#  endif // __GNUC__
# endif
#endif // ASMJIT_API

#ifndef ASMJIT_VAR
# ifdef ASMJIT_API
#  define ASMJIT_VAR extern ASMJIT_API
# else
#  define ASMJIT_VAR
# endif // ASMJIT_API
#endif // !ASMJIT_VAR

// [AsmJit - Memory Management]
#ifndef ASMJIT_MALLOC
# define ASMJIT_MALLOC ::malloc
#endif // ASMJIT_MALLOC

#ifndef ASMJIT_REALLOC
# define ASMJIT_REALLOC ::realloc
#endif // ASMJIT_REALLOC

#ifndef ASMJIT_FREE
# define ASMJIT_FREE ::free
#endif // ASMJIT_FREE

// ============================================================================
// [AsmJit - Calling Conventions]
// ============================================================================

#ifdef ASMJIT_X86
# ifdef __GNUC__
#  define ASMJIT_REGPARM_1 __attribute__((regparm(1)))
#  define ASMJIT_REGPARM_2 __attribute__((regparm(2)))
#  define ASMJIT_REGPARM_3 __attribute__((regparm(3)))
#  define ASMJIT_FASTCALL __attribute__((fastcall))
#  define ASMJIT_STDCALL __attribute__((stdcall))
#  define ASMJIT_CDECL __attribute__((cdecl))
# else
#  define ASMJIT_FASTCALL __fastcall
#  define ASMJIT_STDCALL __stdcall
#  define ASMJIT_CDECL __cdecl
# endif
#else
# define ASMJIT_FASTCALL
# define ASMJIT_STDCALL
# define ASMJIT_CDECL
#endif // ASMJIT_X86

#ifndef ASMJIT_UNUSED
# define ASMJIT_UNUSED(var) ((void)var)
#endif // ASMJIT_UNUSED

#ifndef ASMJIT_NOP
# define ASMJIT_NOP() ((void)0)
#endif // ASMJIT_NOP

// [AsmJit - C++ Compiler Support]
#define ASMJIT_TYPE_TO_TYPE(_Type_) _Type_ 
#define ASMJIT_HAS_STANDARD_DEFINE_OPTIONS
#define ASMJIT_HAS_PARTIAL_TEMPLATE_SPECIALIZATION

// Support for VC6
#if defined(_MSC_VER) && _MSC_VER < 1400

namespace AsmJit
{
	template<typename T> struct _Type2Type
	{
		typedef T Type;
	};
}

#undef ASMJIT_TYPE_TO_TYPE
#define ASMJIT_TYPE_TO_TYPE(_Type_) ::AsmJit::_Type2Type<_Type_>::Type

#undef ASMJIT_HAS_STANDARD_DEFINE_OPTIONS
#undef ASMJIT_HAS_PARTIAL_TEMPLATE_SPECIALIZATION

#endif

// ============================================================================
// [AsmJit - Types]
// ============================================================================

#include <cstdint>

#ifdef ASMJIT_X86
typedef int32_t sysint_t;
typedef uint32_t sysuint_t;
#else
typedef int64_t sysint_t;
typedef uint64_t sysuint_t;
#endif

#ifdef _MSC_VER
# define ASMJIT_INT64_C(num) num##i64
# define ASMJIT_UINT64_C(num) num##ui64
#else
# define ASMJIT_INT64_C(num) num##LL
# define ASMJIT_UINT64_C(num) num##ULL
#endif

// ============================================================================
// [AsmJit - C++ Macros]
// ============================================================================

template<typename T, size_t N> inline size_t ASMJIT_ARRAY_SIZE(T (&)[N]) { return N; }

#define ASMJIT_NO_COPY(__type__) \
private: \
	__type__(const __type__ &other); \
	__type__& operator=(const __type__ &other); \
public:

// ============================================================================
// [AsmJit - Debug]
// ============================================================================

// If ASMJIT_DEBUG and ASMJIT_NO_DEBUG is not defined then ASMJIT_DEBUG will be
// detected using the compiler specific macros. This enables to set the build 
// type using IDE.
#if !defined(ASMJIT_DEBUG) && !defined(ASMJIT_NO_DEBUG)
# ifdef _DEBUG
#  define ASMJIT_DEBUG
# endif // _DEBUG
#endif // !ASMJIT_DEBUG && !ASMJIT_NO_DEBUG

// ============================================================================
// [AsmJit - Initialize/DontInitialize]
// ============================================================================

// TODO: This should be moved to AsmJit namespace!

// Skip documenting this.
#ifndef ASMJIT_NODOC
struct _Initialize {};
struct _DontInitialize {};
#endif // !ASMJIT_NODOC

// ============================================================================
// [AsmJit - OS Support]
// ============================================================================

#ifdef ASMJIT_WINDOWS
# include "windowsh_wrapper.h"
#endif // ASMJIT_WINDOWS

// [Guard]
#endif // _ASMJIT_CORE_BUILD_H

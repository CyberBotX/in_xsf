// Sets up common environment for Shay Green's libraries.
// To change configuration options, modify blargg_config.h, not this file.

// Gb_Snd_Emu 0.2.0
#ifndef BLARGG_COMMON_H
#define BLARGG_COMMON_H

#include <cstdlib>
#include <cassert>
#include <cstdint>

#undef BLARGG_COMMON_H
// allow blargg_config.h to #include blargg_common.h
#include "blargg_config.h"
#ifndef BLARGG_COMMON_H
#define BLARGG_COMMON_H

// blargg_err_t (0 on success, otherwise error string)
#ifndef blargg_err_t
typedef const char *blargg_err_t;
#endif

// BLARGG_COMPILER_HAS_BOOL: If 0, provides bool support for old compiler. If 1,
// compiler is assumed to support bool. If undefined, availability is determined.
#ifndef BLARGG_COMPILER_HAS_BOOL
# ifdef __MWERKS__
#  if !__option(bool)
#   define BLARGG_COMPILER_HAS_BOOL 0
#  endif
# elif defined(_MSC_VER)
#  if _MSC_VER < 1100
#   define BLARGG_COMPILER_HAS_BOOL 0
#  endif
# elif defined(__GNUC__)
// supports bool
# elif __cplusplus < 199711
#  define BLARGG_COMPILER_HAS_BOOL 0
# endif
#endif
#if defined(BLARGG_COMPILER_HAS_BOOL) && !BLARGG_COMPILER_HAS_BOOL
// If you get errors here, modify your blargg_config.h file
typedef int bool;
const bool true = 1;
const bool false = 0;
#endif

#endif
#endif

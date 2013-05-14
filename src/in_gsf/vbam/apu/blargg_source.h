/* Included at the beginning of library source files, AFTER all other #include lines.
Sets up helpful macros and services used in my source code. Since this is only "active"
in my source code, I don't have to worry about polluting the global namespace with
unprefixed names. */

// Gb_Snd_Emu 0.2.0
#ifndef BLARGG_SOURCE_H
#define BLARGG_SOURCE_H

// The following four macros are for debugging only. Some or all might be defined
// to do nothing, depending on the circumstances. Described is what happens when
// a particular macro is defined to do something. When defined to do nothing, the
// macros do NOT evaluate their argument(s).

// If expr is false, prints file and line number, then aborts program. Meant for
// checking internal state and consistency. A failed assertion indicates a bug
// in MY code.
//
// void assert( bool expr );
#include <cassert>

// If expr yields non-NULL error string, returns it from current function,
// otherwise continues normally.
#undef RETURN_ERR
#define RETURN_ERR(expr) \
do \
{ \
	blargg_err_t blargg_return_err_ = (expr); \
	if (blargg_return_err_) \
		return blargg_return_err_; \
} while (0)

#endif

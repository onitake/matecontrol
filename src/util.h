/**
 * Matemat controller firmware
 * Copyright (C) 2014 Chaostreff Basel
 * 
 * util.h
 * Utility macros and constants
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _UTIL_H
#define _UTIL_H

#if (__STDC_VERSION__ >= 201112L)
/* C11 is guaranteed to support static_assert if assert.h is included */
#include <assert.h>
#elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
/* GCC 4.6 and later supports _Static_assert, but not static_assert, unless C11 mode is enabled (only from 4.7) */
#define static_assert(condition, message) _Static_assert(condition, message)
#else
/* No support for non-C11 compilers */
#define static_assert(condition, message)
#endif

/** Put the symbol into the .noinit section, disabling clearing it on startup */
#define ATTRIBUTE_NOINIT __attribute__((section (".noinit")))

#endif /*_UTIL_H*/
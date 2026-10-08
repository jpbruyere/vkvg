/*
 * Copyright (c) 2018-2022 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <stddef.h>

// cross platform os helpers
#if defined(_WIN32) || defined(_WIN64)
// disable warning on iostream functions on windows
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <direct.h>
#if defined(_WIN64)
#ifndef isnan
#define isnan _isnanf
#endif
#endif
#define vkvg_inline     __forceinline
#define disable_warning (warn)
#define reset_warning   (warn)
#elif __APPLE__
#include <math.h>
#define vkvg_inline     static
#define disable_warning (warn)
#define reset_warning   (warn)
#elif __unix__
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#define vkvg_inline     static inline __attribute((always_inline))
#define disable_warning (warn) #pragma GCC diagnostic ignored "-W" #warn
#define reset_warning   (warn) #pragma GCC diagnostic warning "-W" #warn
#if __linux__
void _linux_register_error_handler();
#endif
#endif

#if defined(__clang__)
#define DIAGNOSTIC_DISABLE_UNUSED _Pragma("clang diagnostic push") \
_Pragma("clang diagnostic ignored \"-Wunused-function\"")
#define DIAGNOSTIC_RESTORE_UNUSED  _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#define DIAGNOSTIC_DISABLE_UNUSED _Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wunused-function\"")
#define DIAGNOSTIC_RESTORE_UNUSED  _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#define DIAGNOSTIC_DISABLE_UNUSED __pragma(warning(push)) \
__pragma(warning(disable : 4505))
#define DIAGNOSTIC_RESTORE_UNUSED  __pragma(warning(pop))
#else
#define DIAGNOSTIC_DISABLE_UNUSED
#define DIAGNOSTIC_RESTORE_UNUSED
#endif

#if defined(__GNUC__) || defined(__clang__)
/* GCC and Clang (Linux, macOS, MinGW) */
#define vkvg_likely(x)   __builtin_expect(!!(x), 1)
#define vkvg_unlikely(x) __builtin_expect(!!(x), 0)
#elif defined(_MSC_VER)
/* MSVC (Windows)                                                 */
/* MSVC does not have a static branch predictor builtin.          */
/* We fall back to standard evaluation. MSVC relies on PGO        */
/* (Profile-Guided Optimization) to sort branches at link time.  */
#define vkvg_likely(x)   (!!(x))
#define vkvg_unlikely(x) (!!(x))
#else
/* Fallback for any other exotic compiler */
#define vkvg_likely(x)   (!!(x))
#define vkvg_unlikely(x) (!!(x))
#endif

// 1. Check if we are in a POSIX environment that supports strncasecmp
#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200112L)
#include <strings.h> /* Provides the native strncasecmp */
#else
/* 2. Fallback: Implement our own C11-compliant version */
#include <ctype.h>

static inline int strncasecmp(const char *s1, const char *s2, size_t n) {
    while (n-- > 0) {
        unsigned char u1 = (unsigned char)*s1;
        unsigned char u2 = (unsigned char)*s2;
        int diff = tolower(u1) - tolower(u2);
        if (diff != 0 || u1 == '\0') return diff;
        s1++; s2++;
    }
    return 0;
}
#endif


const char* getUserDir();
int create_dir(const char* path);
int create_dir_tree(const char* path);

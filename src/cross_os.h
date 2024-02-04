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
#ifndef CROSS_OS_H
#define CROSS_OS_H

//cross platform os helpers
#if defined(_WIN32) || defined(_WIN64)
	//disable warning on iostream functions on windows
	#define _CRT_SECURE_NO_WARNINGS
	#include "windows.h"
	#if defined(_WIN64)
		#ifndef isnan
			#define isnan _isnanf
		#endif
	#endif
	#define vkvg_inline __forceinline
	#define disable_warning (warn)
	#define reset_warning (warn)
#elif __APPLE__
	#include <math.h>
	#define vkvg_inline static
	#define disable_warning (warn)
	#define reset_warning (warn)

#elif __unix__
	#include <unistd.h>
	#include <sys/types.h>
	#include <pwd.h>
	#define vkvg_inline static inline __attribute((always_inline))
	#define disable_warning (warn) #pragma GCC diagnostic ignored "-W"#warn
	#define reset_warning (warn) #pragma GCC diagnostic warning "-W"#warn
	#if __linux__
		void _linux_register_error_handler ();
	#endif
#endif

const char* getUserDir ();

#endif // CROSS_OS_H

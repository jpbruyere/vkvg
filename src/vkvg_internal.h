/*
 * Copyright (c) 2018 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
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
#ifndef VKVG_INTERNAL_H
#define VKVG_INTERNAL_H

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>

#define M_PIF               3.14159265358979323846f /* float pi */
#define M_PIF_2             1.57079632679489661923f
#define PATH_CLOSED_BIT     0x80000000              /* most significant bit of path elmts is closed/open path state */
#define PATH_HAS_CURVES_BIT 0x40000000              /* 2d most significant bit of path elmts start = true if curve data are present,
                                                       stored to avoid emiting join in curves */
#define PATH_IS_CURVE_BIT   0x80000000              /* most significant bit of path elmts end mark curves data in path array */

#define PATH_ELT_MASK       0x3FFFFFFF              /* Bit mask for fetching path element value */

#define ROUNDF(f, c) (((float)((int)((f) * (c))) / (c)))
#define ROUND_DOWN(v,p) (floorf(v * p) / p)
#define EQUF(a, b) (fabsf(a-b)<=FLT_EPSILON)

#include "vectors.h"
#include "cross_mutex.h"

//width of the stencil buffer will determine the number of context saving/restore layers
//the two first bits of the stencil are the FILL and the CLIP bits, all other bits are
//used to store clipping bit on context saving. 8 bit stencil will allow 6 save/restore layer
#define FB_STENCIL_FORMAT VK_FORMAT_S8_UINT
#define FB_COLOR_FORMAT VK_FORMAT_B8G8R8A8_UNORM
#define VKVG_FENCE_TIMEOUT UINT64_MAX
#endif

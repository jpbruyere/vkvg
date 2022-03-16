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
#ifndef VKVG_INTERNAL_H
#define VKVG_INTERNAL_H

 //disable warning on iostream functions on windows
#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>  // needed before stdarg.h on Windows
#include <stdarg.h>
#include <string.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PIF
	#define M_PIF				3.14159265358979323846f /* float pi */
	#define M_PIF_2				1.57079632679489661923f
	#define M_2_PIF				0.63661977236758134308f	 // 2/pi
#endif

/*#ifndef M_2_PI
	#define M_2_PI		0.63661977236758134308	// 2/pi
#endif*/

#ifdef DEBUG
#define LOG(level,...) {				\
	if ((vkvg_log_level) & (level))		\
		fprintf (stdout, __VA_ARGS__);	\
}
#else
#define LOG
#endif

#define PATH_CLOSED_BIT		0x80000000				/* most significant bit of path elmts is closed/open path state */
#define PATH_HAS_CURVES_BIT 0x40000000				/* 2rd most significant bit of path elmts is curved status
													 * for main path, this indicate that curve datas are present.
													 * For segments, this indicate that the segment is curved or not */
#define PATH_IS_CONVEX_BIT	0x20000000				/* simple rectangle or circle. */
#define PATH_ELT_MASK		0x1FFFFFFF				/* Bit mask for fetching path element value */

#define ROUNDF(f, c) (((float)((int)((f) * (c))) / (c)))
#define ROUND_DOWN(v,p) (floorf(v * p) / p)
#define EQUF(a, b) (fabsf(a-b)<=FLT_EPSILON)

#ifndef MAX
	#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
	#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#include "deps/tinycthread.h"
#include "cross_os.h"
//width of the stencil buffer will determine the number of context saving/restore layers
//the two first bits of the stencil are the FILL and the CLIP bits, all other bits are
//used to store clipping bit on context saving. 8 bit stencil will allow 6 save/restore layer
#define FB_COLOR_FORMAT VK_FORMAT_B8G8R8A8_UNORM
#define VKVG_SURFACE_IMGS_REQUIREMENTS (VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT|VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT|\
	VK_FORMAT_FEATURE_TRANSFER_DST_BIT|VK_FORMAT_FEATURE_TRANSFER_SRC_BIT|VK_FORMAT_FEATURE_BLIT_SRC_BIT)
#define VKVG_PNG_WRITE_IMG_REQUIREMENTS (VK_FORMAT_FEATURE_TRANSFER_SRC_BIT|VK_FORMAT_FEATURE_TRANSFER_DST_BIT|VK_FORMAT_FEATURE_BLIT_DST_BIT)
//30 seconds fence timeout
#define VKVG_FENCE_TIMEOUT 30000000000
//#define VKVG_FENCE_TIMEOUT 10000

#include "vkvg.h"
#include "vkvg_buff.h"
#include "vkh.h"
#include "vectors.h"

/*typedef struct {
	vkvg_status_t status;
} _vkvg_no_mem_struct;*/

static vkvg_status_t _no_mem_status = VKVG_STATUS_NO_MEMORY;

#endif

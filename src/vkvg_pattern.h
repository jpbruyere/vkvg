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
#ifndef VKVG_PATTERN_H
#define VKVG_PATTERN_H

#include "vkvg_internal.h"

typedef struct _vkvg_pattern_t {
	vkvg_status_t		status;
	uint32_t			references;
	vkvg_pattern_type_t type;
	vkvg_extend_t		extend;
	vkvg_filter_t		filter;
	vkvg_matrix_t		matrix;
	bool				hasMatrix;
	void*				data;
}vkvg_pattern_t;

typedef struct _vkvg_gradient_t {
	vkvg_color_t	colors[16];
#ifdef VKVG_VK_SCALAR_BLOCK_SUPPORTED
	float			stops[16];
#else
	vec4			stops[16];
#endif
	vec4			cp[2];
	uint32_t		count;
}vkvg_gradient_t;

#endif

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

#include "vkvg_surface_internal.h"
#include "vkvg_context_internal.h"
#include "vkvg_device_internal.h"
#include "vkvg_pattern.h"

VkvgPattern vkvg_pattern_create_for_surface (VkvgSurface surf){
	VkvgPattern pat = (vkvg_pattern_t*)calloc(1, sizeof(vkvg_pattern_t));
	if (!pat)
		return (VkvgPattern)&_no_mem_status;

	pat->type = VKVG_PATTERN_TYPE_SURFACE;
	pat->extend = VKVG_EXTEND_NONE;
	pat->data = surf;
	pat->references = 1;

	vkvg_surface_reference (surf);
	if (surf->status)
		pat->status = surf->status;

	return pat;
}
vkvg_status_t vkvg_pattern_get_linear_points (VkvgPattern pat, float* x0, float* y0, float* x1, float* y1) {
	if (pat->status)
		return pat->status;
	if (pat->type != VKVG_PATTERN_TYPE_LINEAR)
		return VKVG_STATUS_PATTERN_TYPE_MISMATCH;

	vkvg_gradient_t* grad = (vkvg_gradient_t*)pat->data;

	*x0 = grad->cp[0].x;
	*y0 = grad->cp[0].y;
	*x1 = grad->cp[0].z;
	*y1 = grad->cp[0].w;
	return VKVG_STATUS_SUCCESS;
}
vkvg_status_t vkvg_pattern_edit_linear (VkvgPattern pat, float x0, float y0, float x1, float y1){
	if (pat->status)
		return pat->status;
	if (pat->type != VKVG_PATTERN_TYPE_LINEAR)
		return VKVG_STATUS_PATTERN_TYPE_MISMATCH;

	vkvg_gradient_t* grad = (vkvg_gradient_t*)pat->data;

	grad->cp[0] = (vec4){{x0}, {y0}, {x1}, {y1}};
	return VKVG_STATUS_SUCCESS;
}
VkvgPattern vkvg_pattern_create_linear (float x0, float y0, float x1, float y1){
	VkvgPattern pat = (vkvg_pattern_t*)calloc(1, sizeof(vkvg_pattern_t));
	if (!pat)
		return (VkvgPattern)&_no_mem_status;
	pat->type = VKVG_PATTERN_TYPE_LINEAR;
	pat->extend = VKVG_EXTEND_NONE;

	pat->data = (void*)calloc(1,sizeof(vkvg_gradient_t));

	if (pat->data) {
		vkvg_pattern_edit_linear(pat, x0, y0, x1, y1);
		pat->references = 1;
	} else
		pat->status = VKVG_STATUS_NO_MEMORY;

	return pat;
}
vkvg_status_t vkvg_pattern_edit_radial (VkvgPattern pat,
										float cx0, float cy0, float radius0,
										float cx1, float cy1, float radius1) {
	if (pat->status)
		return pat->status;
	if (pat->type != VKVG_PATTERN_TYPE_RADIAL)
		return VKVG_STATUS_PATTERN_TYPE_MISMATCH;

	vkvg_gradient_t* grad = (vkvg_gradient_t*)pat->data;

	vec2 c0 = {cx0, cy0};
	vec2 c1 = {cx1, cy1};

	if (radius0 > radius1 - 1.0f)
		radius0 = radius1 - 1.0f;
	vec2 u = vec2_sub (c0, c1);
	float l = vec2_length(u);
	if (l + radius0 + 1.0f >= radius1) {
		vec2 v = vec2_div_s(u, l);
		c0 = vec2_add(c1, vec2_mult_s (v, radius1 - radius0 - 1.0f));
	}

	grad->cp[0] = (vec4){{c0.x}, {c0.y},{radius0},{0}};
	grad->cp[1] = (vec4){{c1.x}, {c1.y},{radius1},{0}};
	return VKVG_STATUS_SUCCESS;
}
VkvgPattern vkvg_pattern_create_radial (float cx0, float cy0, float radius0,
										float cx1, float cy1, float radius1){
	VkvgPattern pat = (vkvg_pattern_t*)calloc(1, sizeof(vkvg_pattern_t));
	if (!pat)
		return (VkvgPattern)&_no_mem_status;
	pat->type = VKVG_PATTERN_TYPE_RADIAL;
	pat->extend = VKVG_EXTEND_NONE;

	pat->data = (void*)calloc(1,sizeof(vkvg_gradient_t));

	if (pat->data) {
		vkvg_pattern_edit_radial (pat, cx0, cy0, radius0, cx1, cy1, radius1);
		pat->references = 1;
	} else
		pat->status = VKVG_STATUS_NO_MEMORY;

	return pat;
}
VkvgPattern vkvg_pattern_reference (VkvgPattern pat) {
	if (!pat->status)
		pat->references++;
	return pat;
}
uint32_t vkvg_pattern_get_reference_count (VkvgPattern pat) {
	if (pat->status)
		return 0;
	return pat->references;
}
vkvg_status_t vkvg_pattern_add_color_stop (VkvgPattern pat, float offset, float r, float g, float b, float a) {
	if (pat->status)
		return pat->status;
	if (pat->type == VKVG_PATTERN_TYPE_SURFACE || pat->type == VKVG_PATTERN_TYPE_SOLID)
		return VKVG_STATUS_PATTERN_TYPE_MISMATCH;

	vkvg_gradient_t* grad = (vkvg_gradient_t*)pat->data;
#ifdef VKVG_PREMULT_ALPHA
	vkvg_color_t c = {a*r,a*g,a*b,a};
#else
	vkvg_color_t c = {r,g,b,a};
#endif
	grad->colors[grad->count] = c;
#ifdef VKVG_VK_SCALAR_BLOCK_SUPPORTED
	grad->stops[grad->count] = offset;
#else
	grad->stops[grad->count].r = offset;
#endif
	grad->count++;
}
void vkvg_pattern_set_extend (VkvgPattern pat, vkvg_extend_t extend){
	if (pat->status)
		return;
	pat->extend = extend;
}
void vkvg_pattern_set_filter (VkvgPattern pat, vkvg_filter_t filter){
	if (pat->status)
		return;
	pat->filter = filter;
}

vkvg_extend_t vkvg_pattern_get_extend (VkvgPattern pat){
	if (pat->status)
		return (vkvg_extend_t)0;
	return pat->extend;
}
vkvg_filter_t vkvg_pattern_get_filter (VkvgPattern pat){
	if (pat->status)
		return (vkvg_filter_t)0;
	return pat->filter;
}
vkvg_pattern_type_t vkvg_pattern_get_type (VkvgPattern pat){
	if (pat->status)
		return (vkvg_pattern_type_t)0;
	return pat->type;
}
vkvg_status_t vkvg_pattern_get_color_stop_count (VkvgPattern pat, uint32_t* count) {
	if (pat->status)
		return pat->status;
	if (pat->type == VKVG_PATTERN_TYPE_SURFACE || pat->type == VKVG_PATTERN_TYPE_SOLID)
		return VKVG_STATUS_PATTERN_TYPE_MISMATCH;
	vkvg_gradient_t* grad = (vkvg_gradient_t*)pat->data;
	*count = grad->count;
	return VKVG_STATUS_SUCCESS;
}
vkvg_status_t vkvg_pattern_get_color_stop_rgba (VkvgPattern pat, uint32_t index,
												float* offset, float* r, float* g, float* b, float* a) {
	if (pat->status)
		return pat->status;
	if (pat->type == VKVG_PATTERN_TYPE_SURFACE || pat->type == VKVG_PATTERN_TYPE_SOLID)
		return VKVG_STATUS_PATTERN_TYPE_MISMATCH;
	vkvg_gradient_t* grad = (vkvg_gradient_t*)pat->data;
	if (index >= grad->count)
		return VKVG_STATUS_INVALID_INDEX;
#ifdef VKVG_VK_SCALAR_BLOCK_SUPPORTED
	*offset = grad->stops[index];
#else
	*offset = grad->stops[index].r;
#endif
	vkvg_color_t c = grad->colors[index];
	*r = c.r;
	*g = c.g;
	*b = c.b;
	*a = c.a;
	return VKVG_STATUS_SUCCESS;
}
void vkvg_pattern_set_matrix (VkvgPattern pat, const vkvg_matrix_t* matrix) {
	if (pat->status)
		return;
	pat->matrix		= *matrix;
	pat->hasMatrix	= true;
}
void vkvg_pattern_get_matrix (VkvgPattern pat, vkvg_matrix_t* matrix) {
	if (pat->status)
		return;
	if (pat->hasMatrix)
		*matrix = pat->matrix;
	else
		*matrix = VKVG_IDENTITY_MATRIX;
}
vkvg_status_t vkvg_pattern_status (VkvgPattern pat) {
	return pat->status;
}

void vkvg_pattern_destroy(VkvgPattern pat)
{
	if (pat->status)
		return;
	pat->references--;
	if (pat->references > 0)
		return;

	if (pat->type == VKVG_PATTERN_TYPE_SURFACE) {
		VkvgSurface surf = (VkvgSurface)pat->data;
		vkvg_surface_destroy (surf);
	}else
		free (pat->data);

	free(pat);
}

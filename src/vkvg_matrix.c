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

//most of the matrix logic is grabbed from cairo, so here is the
//licence:
/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2002 University of Southern California
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is University of Southern
 * California.
 *
 * Contributor(s):
 *	Carl D. Worth <cworth@cworth.org>
 */

#include "vkvg_matrix.h"

#define ISFINITE(x) ((x) * (x) >= 0.) /* check for NaNs */

//matrix computations mainly taken from http://cairographics.org
static void _vkvg_matrix_scalar_multiply (vkvg_matrix_t *matrix, float scalar)
{
	matrix->xx *= scalar;
	matrix->yx *= scalar;

	matrix->xy *= scalar;
	matrix->yy *= scalar;

	matrix->x0 *= scalar;
	matrix->y0 *= scalar;
}
void _vkvg_matrix_get_affine (const vkvg_matrix_t *matrix,
			  float *xx, float *yx,
			  float *xy, float *yy,
			  float *x0, float *y0)
{
	*xx	 = matrix->xx;
	*yx	 = matrix->yx;

	*xy	 = matrix->xy;
	*yy	 = matrix->yy;

	if (x0)
	*x0 = matrix->x0;
	if (y0)
	*y0 = matrix->y0;
}
static void _vkvg_matrix_compute_adjoint (vkvg_matrix_t *matrix)
{
	/* adj (A) = transpose (C:cofactor (A,i,j)) */
	float a, b, c, d, tx, ty;

	_vkvg_matrix_get_affine (matrix,
				  &a,  &b,
				  &c,  &d,
				  &tx, &ty);

	vkvg_matrix_init (matrix,
			   d, -b,
			   -c, a,
			   c*ty - d*tx, b*tx - a*ty);
}
float _vkvg_matrix_compute_determinant (const vkvg_matrix_t *matrix)
{
	float a, b, c, d;

	a = matrix->xx; b = matrix->yx;
	c = matrix->xy; d = matrix->yy;

	return a*d - b*c;
}
vkvg_status_t vkvg_matrix_invert (vkvg_matrix_t *matrix)
{
	float det;

	/* Simple scaling|translation matrices are quite common... */
	if (matrix->xy == 0. && matrix->yx == 0.) {
		matrix->x0 = -matrix->x0;
		matrix->y0 = -matrix->y0;

		if (matrix->xx != 1.f) {
			if (matrix->xx == 0.)
			return VKVG_STATUS_INVALID_MATRIX;

			matrix->xx = 1.f / matrix->xx;
			matrix->x0 *= matrix->xx;
		}

		if (matrix->yy != 1.f) {
			if (matrix->yy == 0.)
			return VKVG_STATUS_INVALID_MATRIX;

			matrix->yy = 1.f / matrix->yy;
			matrix->y0 *= matrix->yy;
		}

		return VKVG_STATUS_SUCCESS;
	}

	/* inv (A) = 1/det (A) * adj (A) */
	det = _vkvg_matrix_compute_determinant (matrix);

	if (! ISFINITE (det))
		return VKVG_STATUS_INVALID_MATRIX;

	if (det == 0)
		return VKVG_STATUS_INVALID_MATRIX;

	_vkvg_matrix_compute_adjoint (matrix);
	_vkvg_matrix_scalar_multiply (matrix, 1 / det);

	return VKVG_STATUS_SUCCESS;
}
void vkvg_matrix_init_identity (vkvg_matrix_t *matrix)
{
	vkvg_matrix_init (matrix,
			   1, 0,
			   0, 1,
			   0, 0);
}

void vkvg_matrix_init (vkvg_matrix_t *matrix,
		   float xx, float yx,
		   float xy, float yy,
		   float x0, float y0)
{
	matrix->xx = xx; matrix->yx = yx;
	matrix->xy = xy; matrix->yy = yy;
	matrix->x0 = x0; matrix->y0 = y0;
}

void vkvg_matrix_init_translate (vkvg_matrix_t *matrix, float tx, float ty)
{
	vkvg_matrix_init (matrix,
			   1, 0,
			   0, 1,
			   tx, ty);
}
void vkvg_matrix_init_scale (vkvg_matrix_t *matrix, float sx, float sy)
{
	vkvg_matrix_init (matrix,
			   sx,	0,
			   0, sy,
			   0, 0);
}
void vkvg_matrix_init_rotate (vkvg_matrix_t *matrix, float radians)
{
	float  s;
	float  c;

	s = sinf (radians);
	c = cosf (radians);

	vkvg_matrix_init (matrix,
			   c, s,
			   -s, c,
			   0, 0);
}
void vkvg_matrix_translate (vkvg_matrix_t *matrix, float tx, float ty)
{
	vkvg_matrix_t tmp;

	vkvg_matrix_init_translate (&tmp, tx, ty);

	vkvg_matrix_multiply (matrix, &tmp, matrix);
}
void vkvg_matrix_scale (vkvg_matrix_t *matrix, float sx, float sy)
{
	vkvg_matrix_t tmp;

	vkvg_matrix_init_scale (&tmp, sx, sy);

	vkvg_matrix_multiply (matrix, &tmp, matrix);
}
void vkvg_matrix_rotate (vkvg_matrix_t *matrix, float radians)
{
	vkvg_matrix_t tmp;

	vkvg_matrix_init_rotate (&tmp, radians);

	vkvg_matrix_multiply (matrix, &tmp, matrix);
}
void vkvg_matrix_multiply (vkvg_matrix_t *result, const vkvg_matrix_t *a, const vkvg_matrix_t *b)
{
	vkvg_matrix_t r;

	r.xx = a->xx * b->xx + a->yx * b->xy;
	r.yx = a->xx * b->yx + a->yx * b->yy;

	r.xy = a->xy * b->xx + a->yy * b->xy;
	r.yy = a->xy * b->yx + a->yy * b->yy;

	r.x0 = a->x0 * b->xx + a->y0 * b->xy + b->x0;
	r.y0 = a->x0 * b->yx + a->y0 * b->yy + b->y0;

	*result = r;
}
void vkvg_matrix_transform_distance (const vkvg_matrix_t *matrix, float *dx, float *dy)
{
	float new_x, new_y;

	new_x = (matrix->xx * *dx + matrix->xy * *dy);
	new_y = (matrix->yx * *dx + matrix->yy * *dy);

	*dx = new_x;
	*dy = new_y;
}
void vkvg_matrix_transform_point (const vkvg_matrix_t *matrix, float *x, float *y)
{
	vkvg_matrix_transform_distance (matrix, x, y);

	*x += matrix->x0;
	*y += matrix->y0;
}
void vkvg_matrix_get_scale (const vkvg_matrix_t *matrix, float *sx, float *sy) {
	*sx = sqrt (matrix->xx * matrix->xx + matrix->xy * matrix->xy);
	/*if (matrix->xx < 0)
		*sx = -*sx;*/
	*sy = sqrt (matrix->yx * matrix->yx + matrix->yy * matrix->yy);
	/*if (matrix->yy < 0)
		*sy = -*sy;*/
}

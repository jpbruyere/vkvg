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
#ifndef VKVG_VECTORS_H
#define VKVG_VECTORS_H

#include "vkvg_internal.h"

typedef struct {
	float x;
	float y;
}vec2;

static const vec2 vec2_unit_x = {1.f,0};
static const vec2 vec2_unit_y = {0,1.f};

typedef struct {
	double x;
	double y;
}vec2d;

/*const vec2d vec2d_unit_x = {1.0,0};
const vec2d vec2d_unit_y = {0,1.0};*/

typedef struct {
	float x;
	float y;
	float z;
}vec3;

typedef struct {
	union {
		float x;
		float r;
	};
	union {
		float y;
		float g;
	};
	union {
		float z;
		float width;
		float b;
	};
	union {
		float w;
		float height;
		float a;
	};
}vec4;

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t z;
	uint16_t w;
}vec4i16;

typedef struct {
	int16_t x;
	int16_t y;
}vec2i16;

typedef struct {
	vec2 row0;
	vec2 row1;
}mat2;

// compute length of float vector 2d
vkvg_inline	float vec2_length(vec2 v){
	return sqrtf (v.x*v.x + v.y*v.y);
}
// compute normal direction vector from line defined by 2 points in double precision
vkvg_inline	vec2d vec2d_line_norm(vec2d a, vec2d b)
{
	vec2d d = {b.x - a.x, b.y - a.y};
	double md = sqrt (d.x*d.x + d.y*d.y);
	d.x/=md;
	d.y/=md;
	return d;
}
// compute normal direction vector from line defined by 2 points
vkvg_inline	vec2 vec2_line_norm(vec2 a, vec2 b)
{
	vec2 d = {b.x - a.x, b.y - a.y};
	float md = sqrtf (d.x*d.x + d.y*d.y);
	d.x/=md;
	d.y/=md;
	return d;
}
// compute sum of two double precision vectors
vkvg_inline	vec2d vec2d_add (vec2d a, vec2d b){
	return (vec2d){a.x + b.x, a.y + b.y};
}
// compute subbstraction of two double precision vectors
vkvg_inline	vec2d vec2d_sub (vec2d a, vec2d b){
	return (vec2d){a.x - b.x, a.y - b.y};
}
// multiply 2d vector by scalar
vkvg_inline	vec2d vec2d_mult_s(vec2d a, double m){
	return (vec2d){a.x*m,a.y*m};
}
vkvg_inline	vec2d vec2d_div_s(vec2d a, double m){
	return (vec2d){a.x/m,a.y/m};
}
// compute length of double vector 2d
vkvg_inline	double vec2d_length(vec2d v){
	return sqrt (v.x*v.x + v.y*v.y);
}
// normalize double vector
vkvg_inline	vec2d vec2d_norm(vec2d a)
{
	double m = sqrt (a.x*a.x + a.y*a.y);
	return (vec2d){a.x/m, a.y/m};
}
// compute perpendicular vector
vkvg_inline	vec2d vec2d_perp (vec2d a){
	return (vec2d){a.y, -a.x};
}
vkvg_inline	bool vec2d_isnan (vec2d v){
	return (bool)(isnan (v.x) || isnan (v.y));
}


// test equality of two single precision vectors
vkvg_inline	bool vec2_equ (vec2 a, vec2 b){
	return (EQUF(a.x,b.x)&EQUF(a.y,b.y));
}
// compute sum of two single precision vectors
vkvg_inline	vec2 vec2_add (vec2 a, vec2 b){
	return (vec2){a.x + b.x, a.y + b.y};
}
// compute subbstraction of two single precision vectors
vkvg_inline	vec2 vec2_sub (vec2 a, vec2 b){
	return (vec2){a.x - b.x, a.y - b.y};
}
// multiply 2d vector by scalar
vkvg_inline	vec2 vec2_mult_s(vec2 a, float m){
	return (vec2){a.x*m,a.y*m};
}
// devide 2d vector by scalar
vkvg_inline	vec2 vec2_div_s(vec2 a, float m){
	return (vec2){a.x/m,a.y/m};
}
// normalize float vector
vkvg_inline	vec2 vec2_norm(vec2 a)
{
	float m = sqrtf (a.x*a.x + a.y*a.y);
	return (vec2){a.x/m, a.y/m};
}
// compute perpendicular vector
vkvg_inline	vec2 vec2_perp (vec2 a){
	return (vec2){a.y, -a.x};
}
// compute opposite of single precision vector
vkvg_inline	void vec2_inv (vec2* v){
	v->x = -v->x;
	v->y = -v->y;
}
// test if one component of float vector is nan
vkvg_inline	bool vec2_isnan (vec2 v){
	return (bool)(isnanf (v.x) || isnanf (v.y));
}
// test if one component of double vector is nan
vkvg_inline float vec2_dot (vec2 a, vec2 b) {
	return (a.x * b.x) + (a.y * b.y);
}
vkvg_inline float vec2_det (vec2 a, vec2 b) {
	return a.x * b.y - a.y * b.x;
}
vkvg_inline float vec2_slope (vec2 a, vec2 b) {
	return (b.y - a.y) / (b.x - a.x);
}


// convert double precision vector to single precision
vkvg_inline	vec2 vec2d_to_vec2(vec2d vd){
	return (vec2){(float)vd.x,(float)vd.y};
}
vkvg_inline	bool vec4_equ (vec4 a, vec4 b){
	return (EQUF(a.x,b.x)&EQUF(a.y,b.y)&EQUF(a.z,b.z)&EQUF(a.w,b.w));
}
vkvg_inline	vec2 mat2_mult_vec2 (mat2 m, vec2 v) {
	return (vec2){
		(m.row0.x * v.x) + (m.row0.y * v.y),
		(m.row1.x * v.x) + (m.row1.y * v.y)
	};
}
vkvg_inline	float mat2_det (mat2* m) {
	return (m->row0.x * m->row1.y) - (m->row0.y * m->row1.y);
}

#endif

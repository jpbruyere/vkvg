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
typedef struct {
    double x;
    double y;
}vec2d;

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

float		vec2_length	(vec2 v);
vec2		vec2_create	(float x, float y);
vec2		vec2_norm	(vec2 a);
vec2		vec2_perp	(vec2 a);
vec2		vec2_add	(vec2 a, vec2 b);
vec2		vec2_sub	(vec2 a, vec2 b);
vec2		vec2_mult	(vec2 a, float m);
bool		vec2_equ	(vec2 a, vec2 b);
vec2		vec2_line_norm	(vec2 a, vec2 b);

double		vec2d_length(vec2d v);
vec2d		vec2d_norm	(vec2d a);
vec2d		vec2d_perp	(vec2d a);
vec2d		vec2d_add	(vec2d a, vec2d b);
vec2d		vec2d_sub	(vec2d a, vec2d b);
vec2d		vec2d_mult	(vec2d a, double m);
vec2d		vec2d_line_norm(vec2d a, vec2d b);

vec2		vec2d_to_vec2(vec2d vd);
void        vec2_inv    (vec2* v);
#endif

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

#include "vectors.h"

// compute normal direction vector from line defined by 2 points in double precision
vec2d vec2d_line_norm(vec2d a, vec2d b)
{
    vec2d d = {b.x - a.x, b.y - a.y};
    double md = sqrt (d.x*d.x + d.y*d.y);
    d.x/=md;
    d.y/=md;
    return d;
}
// compute normal direction vector from line defined by 2 points
vec2 vec2_line_norm(vec2 a, vec2 b)
{
    vec2 d = {b.x - a.x, b.y - a.y};
    float md = sqrtf (d.x*d.x + d.y*d.y);
    d.x/=md;
    d.y/=md;
    return d;
}
// compute length of double vector 2d
double vec2d_length(vec2d v){
    return sqrt (v.x*v.x + v.y*v.y);
}
// compute length of float vector 2d
float vec2_length(vec2 v){
    return sqrtf (v.x*v.x + v.y*v.y);
}
// normalize float vector
vec2 vec2_norm(vec2 a)
{
    float m = sqrtf (a.x*a.x + a.y*a.y);
    return (vec2){a.x/m, a.y/m};
}
// normalize double vector
vec2d vec2d_norm(vec2d a)
{
    double m = sqrt (a.x*a.x + a.y*a.y);
    return (vec2d){a.x/m, a.y/m};
}
// multiply 2d vector by scalar
vec2d vec2d_mult(vec2d a, double m){
    return (vec2d){a.x*m,a.y*m};
}
// multiply 2d vector by scalar
vec2 vec2_mult(vec2 a, float m){
    return (vec2){a.x*m,a.y*m};
}
// compute perpendicular vector
vec2d vec2d_perp (vec2d a){
    return (vec2d){a.y, -a.x};
}
// compute perpendicular vector
vec2 vec2_perp (vec2 a){
    return (vec2){a.y, -a.x};
}
// convert double precision vector to single precision
vec2 vec2d_to_vec2(vec2d vd){
    return (vec2){(float)vd.x,(float)vd.y};
}
// compute sum of two single precision vectors
vec2 vec2_add (vec2 a, vec2 b){
    return (vec2){a.x + b.x, a.y + b.y};
}
// compute sum of two double precision vectors
vec2d vec2d_add (vec2d a, vec2d b){
    return (vec2d){a.x + b.x, a.y + b.y};
}
// compute subbstraction of two single precision vectors
vec2 vec2_sub (vec2 a, vec2 b){
    return (vec2){a.x - b.x, a.y - b.y};
}
// compute subbstraction of two double precision vectors
vec2d vec2d_sub (vec2d a, vec2d b){
    return (vec2d){a.x - b.x, a.y - b.y};
}
// test equality of two single precision vectors
bool vec2_equ (vec2 a, vec2 b){
    /*if ((ROUNDF(a.x,10000) == ROUNDF(b.x,10000)) && (ROUNDF(a.y,10000) == ROUNDF(b.y,10000)))
        return true;
    return false;*/
    return (EQUF(a.x,b.x)&EQUF(a.y,b.y));
}
// compute opposite of single precision vector
void vec2_inv (vec2* v){
    v->x = -v->x;
    v->y = -v->y;
}
// test if one component of float vector is nan
bool vec2_isnan (vec2 v){
    return (bool)(isnanf (v.x) || isnanf (v.y));
}
// test if one component of double vector is nan
bool vec2d_isnan (vec2d v){
    return (bool)(isnan (v.x) || isnan (v.y));
}

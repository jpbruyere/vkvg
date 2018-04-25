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

#define ROUNDF(f, c) (((float)((int)((f) * (c))) / (c)))

inline vec2 vec2_create (float x, float y) {
    vec2 v = {x,y};
    return v;
}
vec2 vec2_line_norm(vec2 a, vec2 b)
{
    vec2 d = {b.x - a.x, b.y - a.y};
    float md = sqrt (d.x*d.x + d.y*d.y);
    d.x/=md;
    d.y/=md;
    return d;
}
double vec2d_length(vec2d v){
    return sqrt (v.x*v.x + v.y*v.y);
}
float vec2_length(vec2 v){
    return sqrt (v.x*v.x + v.y*v.y);
}
vec2 vec2_norm(vec2 a)
{
    float m = sqrt (a.x*a.x + a.y*a.y);
    vec2 d = {a.x/m, a.y/m};
    return d;
}
vec2d vec2d_norm(vec2d a)
{
    double m = sqrt (a.x*a.x + a.y*a.y);
    vec2d d = {a.x/m, a.y/m};
    return d;
}
vec2d vec2d_mult(vec2d a, double m){
    vec2d r = {a.x*m,a.y*m};
    return r;
}
vec2 vec2_mult(vec2 a, float m){
    vec2 r = {a.x*m,a.y*m};
    return r;
}

vec2d vec2d_line_norm(vec2d a, vec2d b)
{
    vec2d d = {b.x - a.x, b.y - a.y};
    double md = sqrt (d.x*d.x + d.y*d.y);
    d.x/=md;
    d.y/=md;
    return d;
}
vec2d vec2d_perp (vec2d a){
    vec2d vp = {a.y, -a.x};
    return vp;
}
vec2 vec2_perp (vec2 a){
    vec2 vp = {a.y, -a.x};
    return vp;
}
vec2 vec2d_to_vec2(vec2d vd){
    vec2 v = {vd.x,vd.y};
    return v;
}
vec2 vec2_add (vec2 a, vec2 b){
    vec2 r = {a.x + b.x, a.y + b.y};
    return r;
}
vec2d vec2d_add (vec2d a, vec2d b){
    vec2d r = {a.x + b.x, a.y + b.y};
    return r;
}
vec2 vec2_sub (vec2 a, vec2 b){
    vec2 r = {a.x - b.x, a.y - b.y};
    return r;
}
vec2d vec2d_sub (vec2d a, vec2d b){
    vec2d r = {a.x - b.x, a.y - b.y};
    return r;
}
bool vec2_equ (vec2 a, vec2 b){
    if ((ROUNDF(a.x,10000) == ROUNDF(b.x,10000)) && (ROUNDF(a.y,10000) == ROUNDF(b.y,10000)))
        return true;
    return false;
}
void vec2_inv (vec2* v){
    v->x = -v->x;
    v->y = -v->y;
}

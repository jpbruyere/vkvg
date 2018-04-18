#include "vectors.h"

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
    if ((a.x == b.x) && (a.y == b.y))
        return true;
    return false;
}

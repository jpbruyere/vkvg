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

#include "vkvg_surface_internal.h"
#include "vkvg_context_internal.h"
#include "vkvg_device_internal.h"
#include "vkvg_pattern.h"

VkvgPattern vkvg_pattern_create(){
    VkvgPattern pat = (vkvg_pattern_t*)calloc(1, sizeof(vkvg_pattern_t));
    pat->type = VKVG_PATTERN_TYPE_SOLID;
    pat->extend = VKVG_EXTEND_NONE;
    pat->data = (vkvg_color_t*)calloc(1,sizeof(vkvg_color_t));

    pat->references = 1;

    return pat;
}
VkvgPattern vkvg_pattern_create_rgba (float r, float g, float b, float a){
    VkvgPattern pat = (vkvg_pattern_t*)calloc(1, sizeof(vkvg_pattern_t));
    pat->type = VKVG_PATTERN_TYPE_SOLID;
    pat->extend = VKVG_EXTEND_NONE;
    vkvg_color_t* c = (vkvg_color_t*)calloc(1,sizeof(vkvg_color_t));

#ifdef VKVG_PREMULT_ALPHA
    c->r = r * a;
    c->g = g * a;
    c->b = b * a;
    c->a = a;
#else
    c->r = r;
    c->g = g;
    c->b = b;
    c->a = a;
#endif

    pat->data = c;

    pat->references = 1;

    return pat;
}
VkvgPattern vkvg_pattern_create_rgb (float r, float g, float b){
    return vkvg_pattern_create_rgba (r,g,b,1.f);
}
VkvgPattern vkvg_pattern_create_for_surface (VkvgSurface surf){
    VkvgPattern pat = (vkvg_pattern_t*)calloc(1, sizeof(vkvg_pattern_t));
    pat->type = VKVG_PATTERN_TYPE_SURFACE;
    pat->extend = VKVG_EXTEND_NONE;
    pat->data = surf;

    pat->references = 1;
    vkvg_surface_reference (surf);

    return pat;
}
VkvgPattern vkvg_pattern_create_linear (float x0, float y0, float x1, float y1){
    VkvgPattern pat = (vkvg_pattern_t*)calloc(1, sizeof(vkvg_pattern_t));
    pat->type = VKVG_PATTERN_TYPE_LINEAR;
    pat->extend = VKVG_EXTEND_PAD;
    vkvg_gradient_t* grad = (vkvg_gradient_t*)calloc(1,sizeof(vkvg_gradient_t));

    vec4 cp0 = {x0, y0}, cp1 = {x1, y1};
    grad->cp[0] = cp0;
    grad->cp[1] = cp1;

    pat->data = grad;

    pat->references = 1;

    return pat;
}
VkvgPattern vkvg_pattern_create_radial (float cx0, float cy0, float radius0,
                                        float cx1, float cy1, float radius1){
    VkvgPattern pat = (vkvg_pattern_t*)calloc(1, sizeof(vkvg_pattern_t));

    vkvg_gradient_t* grad = (vkvg_gradient_t*)calloc(1,sizeof(vkvg_gradient_t));

    vec4 cp0 = {cx0, cy0}, cp1 = {cx1, cy1}, rads = {radius0, radius1};
    grad->cp[0] = cp0;
    grad->cp[1] = cp1;
    grad->cp[2] = rads;

    pat->data = grad;

    pat->references = 1;

    return pat;
}
VkvgPattern vkvg_pattern_reference (VkvgPattern pat) {
    pat->references++;
    return pat;
}
uint32_t vkvg_pattern_get_reference_count (VkvgPattern pat) {
    return pat->references;
}
void vkvg_patter_add_color_stop (VkvgPattern pat, float offset, float r, float g, float b, float a) {
    if (pat->type == VKVG_PATTERN_TYPE_SURFACE || pat->type == VKVG_PATTERN_TYPE_SOLID){

        return;
    }

    vkvg_gradient_t* grad = (vkvg_gradient_t*)pat->data;
    vkvg_color_t c = {r,g,b,a};
    grad->colors[grad->count] = c;
    grad->stops[grad->count].r = offset;
    grad->count++;
}
void vkvg_pattern_set_extend (VkvgPattern pat, vkvg_extend_t extend){
    pat->extend = extend;
}
void vkvg_pattern_set_filter (VkvgPattern pat, vkvg_filter_t filter){
    pat->filter = filter;
}

vkvg_extend_t vkvg_pattern_get_extend (VkvgPattern pat){
    return pat->extend;
}
vkvg_filter_t vkvg_pattern_get_filter (VkvgPattern pat){
    return pat->filter;
}

void vkvg_pattern_destroy(VkvgPattern pat)
{
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


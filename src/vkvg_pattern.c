#include "vkvg_surface_internal.h"
#include "vkvg_context_internal.h"
#include "vkvg_device_internal.h"
#include "vkvg_pattern.h"

VkvgPattern _init_pattern (VkvgDevice dev){
    VkvgPattern pat = (vkvg_pattern_t*)calloc(1, sizeof(vkvg_pattern_t));
    pat->dev = dev;
    return pat;
}

VkvgPattern vkvg_pattern_create(VkvgDevice dev){
    VkvgPattern pat = _init_pattern (dev);
    pat->type = VKVG_PATTERN_TYPE_SOLID;
    pat->data = (vkvg_color_t*)calloc(1,sizeof(vkvg_color_t));
    return pat;
}

VkvgPattern vkvg_pattern_create_for_surface (VkvgSurface surf){
    VkvgPattern pat = _init_pattern (surf->dev);
    pat->data = surf;
    return pat;
}
VkvgPattern vkvg_pattern_create_linear (VkvgDevice dev, float x0, float y0, float x1, float y1){
    VkvgPattern pat = _init_pattern (dev);

    vkvg_gradient_t* grad = (vkvg_gradient_t*)calloc(1,sizeof(vkvg_gradient_t));

    vec2 cp0 = {x0, y0}, cp1 = {x1, y1};
    grad->cp[0] = cp0;
    grad->cp[1] = cp1;

    pat->data = grad;
    return pat;
}
VkvgPattern vkvg_pattern_create_radial (VkvgDevice dev, float cx0, float cy0, float radius0,
                                        float cx1, float cy1, float radius1){
    VkvgPattern pat = _init_pattern (dev);

    vkvg_gradient_t* grad = (vkvg_gradient_t*)calloc(1,sizeof(vkvg_gradient_t));

    vec2 cp0 = {cx0, cy0}, cp1 = {cx1, cy1}, rads = {radius0, radius1};
    grad->cp[0] = cp0;
    grad->cp[1] = cp1;
    grad->cp[2] = rads;

    pat->data = grad;
    return pat;
}
void vkvg_patter_add_color_stop (VkvgPattern pat, float offset, float r, float g, float b, float a) {
    if (pat->type == VKVG_PATTERN_TYPE_SURFACE || pat->type == VKVG_PATTERN_TYPE_SOLID){

        return;
    }

    vkvg_gradient_t* grad = (vkvg_gradient_t*)pat->data;
    vkvg_color_t c = {r,g,b,a};
    grad->colors[grad->count] = c;
    grad->stops[grad->count] = offset;
}
void vkvg_pattern_set_extend (VkvgPattern pat, vkvg_extend_t extend){
    pat->extend = extend;
}

void vkvg_set_source (VkvgContext ctx, VkvgPattern pat){

}

void vkvg_pattern_destroy(VkvgPattern pat)
{
    if (pat->type != VKVG_PATTERN_TYPE_SURFACE)
        free (pat->data);

    free(pat);
}


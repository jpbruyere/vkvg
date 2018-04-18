#ifndef VKVG_PATTERN_H
#define VKVG_PATTERN_H

#include "vkvg_internal.h"
#include "vkvg.h"
#include "vkh.h"

typedef struct _vkvg_pattern_t {
    vkvg_extend_t       extend;
    vkvg_pattern_type_t type;
    void*               data;
}vkvg_pattern_t;

typedef struct _vkvg_gradient_t {
    vec4            cp[3];
    vkvg_color_t    colors[16];
    vec4            stops[16];
    uint32_t        count;
}vkvg_gradient_t;

#endif

// Copyright (c) 2018-2026 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
#ifndef VKVG_PATTERN_H
#define VKVG_PATTERN_H

#include "vkvg_internal.h"

typedef struct _vkvg_pattern_t {
    vkvg_status_t       status;
    atomic_int          references;
    vkvg_pattern_type_t type;
    vkvg_extend_t       extend;
    vkvg_filter_t       filter;
    vkvg_matrix_t       matrix;
    bool                hasMatrix;
} vkvg_pattern_t;

typedef struct _vkvg_surface_pattern_t {
    vkvg_pattern_t      base;
    VkvgSurface         surf;
} vkvg_surface_pattern_t;

typedef struct _vkvg_gradient_t {
    vkvg_color_t colors[16];
#ifdef VKVG_ENABLE_VK_SCALAR_BLOCK_LAYOUT
    float stops[16];
#else
    vec4 stops[16];
#endif
    vec4     cp[2];
    uint32_t count;
} vkvg_gradient_t;

typedef struct _vkvg_gradient_pattern_t {
    vkvg_pattern_t      base;
    vkvg_gradient_t     gradient;
} vkvg_gradient_pattern_t;

#endif

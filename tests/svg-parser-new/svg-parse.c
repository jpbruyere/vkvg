#include "vkvg-svg.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include <immintrin.h>

#define ARRAY_INIT         8
#define ARRAY_ELEMENT_TYPE void *

#define ARRAY_IMPLEMENTATION<
#include "array.h"

//#define PREPROC_SVG
#define PROCESS_SVG                                                                                        \
    int   surfW = 0, surfH = 0;                                                                                        \
    float xScale = 1, yScale = 1;                                                                                      \
    if (svg->forced_width) {                                                                                           \
        if (svg->width.units == svg_unit_percentage)                                                                        \
            surfW = _get_pixel_coord(svg->forced_width, &svg->width);                                                              \
        else                                                                                                           \
            surfW = svg->forced_width;                                                                                        \
    } else if (svg->width.units == svg_unit_percentage) {                                                                   \
        if (svg->hasViewBox)                                                                                                \
            surfW = _get_pixel_coord(svg->viewBox.w, &svg->width);                                                          \
        else                                                                                                           \
            return 0;                                                                                                  \
    } else                                                                                                             \
        surfW = svg->width.number;                                                                                          \
    if (svg->forced_height) {                                                                                                 \
        if (svg->height.units == svg_unit_percentage)                                                                       \
            surfH = _get_pixel_coord(svg->forced_height, &svg->height);                                                            \
        else                                                                                                           \
            surfH = svg->forced_height;                                                                                       \
    } else if (svg->height.units == svg_unit_percentage) {                                                                  \
        if (svg->hasViewBox)                                                                                                \
            surfH = _get_pixel_coord(svg->viewBox.h, &svg->height);                                                         \
        else                                                                                                           \
            return 0;                                                                                                  \
    } else                                                                                                             \
        surfH = svg->height.number;                                                                                         \
    if (!svg->hasViewBox) {                                                                                                 \
        svg->viewBox = (svg_viewbox){0, 0, surfW, surfH};                                                              \
        if (svg->width.units != svg_unit_percentage)                                                                        \
            svg->viewBox.w = svg->width.number;                                                                             \
        if (svg->height.units != svg_unit_percentage)                                                                       \
            svg->viewBox.h = svg->height.number;                                                                            \
    }                                                                                                                  \
    \
    xScale = (float)surfW / svg->viewBox.w;                                                                            \
    yScale = (float)surfH / svg->viewBox.h;                                                                            \
    if (svg->queryDimensions) {                                                                                        \
        svg->width  = (svg_length_or_percentage){surfW, svg_unit_px};                                                                                           \
        svg->height = (svg_length_or_percentage){surfH, svg_unit_px};                                                                                           \
        return 0;                                                                                                      \
    }                                                                                                                  \
    if (!svg->ctx) {                                                                                                   \
        svg->surf = vkvg_surface_create(svg->dev, surfW, surfH);                                                       \
        svg->ctx  = vkvg_create(svg->surf);                                                                            \
        vkvg_clear (svg->ctx);                                                                                         \
        svg->ownContext = true;                                                                                        \
    }                                                                                                                  \
    vkvg_set_fill_rule(svg->ctx, VKVG_FILL_RULE_NON_ZERO);                                                             \
    if (svg->preserveAspectRatio) {                                                                                    \
        if (xScale < yScale)                                                                                           \
            vkvg_scale(svg->ctx, xScale, xScale);                                                                      \
        else                                                                                                           \
            vkvg_scale(svg->ctx, yScale, yScale);                                                                      \
    } else                                                                                                             \
        vkvg_scale(svg->ctx, xScale, yScale);

#define POSTPROC_SVG                                                                                                   \
{                                                                                                                      \
    if (svg->ownContext)                                                                                               \
        vkvg_destroy(svg->ctx);                                                                                        \
}


#define PREPROC_G   vkvg_save (svg->ctx);
#define POSTPROC_G  vkvg_restore (svg->ctx);

#define PRE_PROC_DEFS   svg->inDefs = true;
#define POST_PROC_DEFS  svg->inDefs = false;
//=== LINE ===
#define PREPROC_LINE    vkvg_save(svg->ctx);        \
                        parentData = _new_line();

#define PROCESS_LINE  _process_element(svg, &attribs, parentData, false);
#define POSTPROC_LINE vkvg_restore(svg->ctx);

#define SVG_ATT_LINE_X1           try_parse_length_or_percentage(svg, &(CAST(line)->x1));
#define SVG_ATT_LINE_Y1           try_parse_length_or_percentage(svg, &(CAST(line)->y1));
#define SVG_ATT_LINE_X2           try_parse_length_or_percentage(svg, &(CAST(line)->x2));
#define SVG_ATT_LINE_Y2           try_parse_length_or_percentage(svg, &(CAST(line)->y2));
//============

//=== RECT ===
#define PREPROC_RECT    vkvg_save(svg->ctx);        \
                        parentData = _new_rect();
#define PROCESS_RECT    _process_element(svg, &attribs, parentData, false);
#define POSTPROC_RECT   vkvg_restore(svg->ctx);
#define SVG_ATT_RECT_X            try_parse_length_or_percentage(svg, &(CAST(rect)->x));
#define SVG_ATT_RECT_Y            try_parse_length_or_percentage(svg, &(CAST(rect)->y));
#define SVG_ATT_RECT_WIDTH        try_parse_length_or_percentage(svg, &(CAST(rect)->w));
#define SVG_ATT_RECT_HEIGHT       try_parse_length_or_percentage(svg, &(CAST(rect)->h));
#define SVG_ATT_RECT_RX           try_parse_length_or_percentage(svg, &(CAST(rect)->rx));
#define SVG_ATT_RECT_RY           try_parse_length_or_percentage(svg, &(CAST(rect)->ry));
//============

//== CIRCLE ==
#define PREPROC_CIRCLE      vkvg_save(svg->ctx);        \
                            parentData = _new_circle();
#define PROCESS_CIRCLE      _process_element(svg, &attribs, parentData, false);
#define POSTPROC_CIRCLE     vkvg_restore(svg->ctx);
#define SVG_ATT_CIRCLE_CX   try_parse_length_or_percentage(svg, &(CAST(circle)->cx));
#define SVG_ATT_CIRCLE_CY   try_parse_length_or_percentage(svg, &(CAST(circle)->cy));
#define SVG_ATT_CIRCLE_R    try_parse_length_or_percentage(svg, &(CAST(circle)->r));
//============

//= ELLIPSE ==
#define PREPROC_ELLIPSE     vkvg_save(svg->ctx);        \
                            parentData = _new_ellipse();
#define PROCESS_ELLIPSE     _process_element(svg, &attribs, parentData, false);
#define POSTPROC_ELLIPSE    vkvg_restore(svg->ctx);
#define SVG_ATT_ELLIPSE_CX  try_parse_length_or_percentage(svg, &(CAST(ellipse)->cx));
#define SVG_ATT_ELLIPSE_CY  try_parse_length_or_percentage(svg, &(CAST(ellipse)->cy));
#define SVG_ATT_ELLIPSE_RX  try_parse_length_or_percentage(svg, &(CAST(ellipse)->rx));
#define SVG_ATT_ELLIPSE_RY  try_parse_length_or_percentage(svg, &(CAST(ellipse)->ry));
//============

//= POLYLINE =
#define PREPROC_POLYLINE        vkvg_save(svg->ctx);        \
                                parentData = _new_polygon();
#define PROCESS_POLYLINE        _process_element(svg, &attribs, parentData, false);
#define POSTPROC_POLYLINE       vkvg_restore(svg->ctx);
#define SVG_ATT_POLYLINE_POINTS    \
    CASTELT(p,polygon,parentData); \
    p->points = svg->value;        \
    p->points_len = svg->value_len;

//===========

//= POLYGON =
#define PREPROC_POLYGON         vkvg_save(svg->ctx);         \
                                parentData = _new_polygon(); \
                                CAST(polygon)->closed = true;
#define PROCESS_POLYGON         _process_element(svg, &attribs, parentData, false);
#define POSTPROC_POLYGON        vkvg_restore(svg->ctx);
#define SVG_ATT_POLYGON_POINTS     \
    CASTELT(p,polygon,parentData); \
    p->points = svg->value;        \
    p->points_len = svg->value_len;
//============

//=== PATH ===
#define PREPROC_PATH    vkvg_save(svg->ctx);        \
                        parentData = _new_path();

#define PROCESS_PATH    _process_element(svg, &attribs, parentData, false);
#define POSTPROC_PATH   vkvg_restore(svg->ctx);
#define SVG_ATT_D               \
    CASTELT(p,path,parentData); \
    p->d = svg->value;          \
    p->d_len = svg->value_len;
//============

//= LINEAR GRADIENT =
#define PREPROC_LINEARGRADIENT                                                                                         \
    parentData = _new_linear_gradient();

#define PROCESS_LINEARGRADIENT                                                                                         \
    CASTELT(rg,linear_gradient,parentData);                                                                            \
    rg->pattern      = vkvg_pattern_create_linear(rg->x1.number, rg->y1.number, rg->x2.number, rg->y2.number);         \
    rg->id.hash      = svg->currentIdHash;                                                                             \
    rg->id.xlinkHref = svg->currentXlinkHref;                                                                          \
    LOG("SVG: store pattern id:%u href:%u\n", rg->id.hash, rg->id.xlinkHref);                                          \
    if (rg->hasTransform)                                                                                                  \
        vkvg_pattern_set_matrix(rg->pattern, &rg->transform);                                                              \
    array_add(svg->idList, rg);

#define SVG_ATT_LINEARGRADIENT_X1                              try_parse_length_or_percentage(svg, &(CAST(linear_gradient)->x1));
#define SVG_ATT_LINEARGRADIENT_Y1                              try_parse_length_or_percentage(svg, &(CAST(linear_gradient)->y1));
#define SVG_ATT_LINEARGRADIENT_X2                              try_parse_length_or_percentage(svg, &(CAST(linear_gradient)->x2));
#define SVG_ATT_LINEARGRADIENT_Y2                              try_parse_length_or_percentage(svg, &(CAST(linear_gradient)->y2));
#define SVG_ATT_LINEARGRADIENT_GRADIENTUNITS                                        \
    if (!strncasecmp ((char*)svg->value, "userspaceonuse", svg->value_len))         \
        CAST(linear_gradient)->gradientUnits = svg_gradient_unit_userSpaceOnUse;    \
    else if (!strncasecmp ((char*)svg->value, "objectboundingbox", svg->value_len)) \
        CAST(linear_gradient)->gradientUnits = svg_gradient_unit_objectBoundingBox; \
    else {                                                                          \
        LOG("Unrecognized gradient units: %.*s", (int)svg->value_len, svg->value);  \
}
#define SVG_ATT_LINEARGRADIENT_GRADIENTTRANSFORM                \
    CASTELT(rg,linear_gradient,parentData);                     \
    if (!rg->hasTransform)                                      \
        rg->transform = VKVG_IDENTITY_MATRIX;                   \
    rg->hasTransform = try_parse_transform(svg, &rg->transform);
//===================

//= RADIAL GRADIENT =
#define PREPROC_RADIALGRADIENT                                                                                         \
    parentData = _new_radial_gradient();
#define PROCESS_RADIALGRADIENT                                                                             \
    CASTELT(rg,radial_gradient,parentData);                                                                            \
    rg->pattern =                                                                                                      \
    vkvg_pattern_create_radial(rg->fx.number, rg->fy.number, 0, rg->cx.number, rg->cy.number, rg->r.number);       \
    rg->id.hash      = svg->currentIdHash;                                                                             \
    rg->id.xlinkHref = svg->currentXlinkHref;                                                                          \
    LOG("store pattern id:%u href:%u\n", rg->id.hash, rg->id.xlinkHref);                                               \
    if (rg->hasTransform)                                                                                                  \
        vkvg_pattern_set_matrix(rg->pattern, &rg->transform);                                                              \
    array_add(svg->idList, rg);

#define SVG_ATT_RADIALGRADIENT_CX                              try_parse_length_or_percentage(svg, &(CAST(radial_gradient)->cx));
#define SVG_ATT_RADIALGRADIENT_CY                              try_parse_length_or_percentage(svg, &(CAST(radial_gradient)->cy));
#define SVG_ATT_RADIALGRADIENT_R                               try_parse_length_or_percentage(svg, &(CAST(radial_gradient)->r));
#define SVG_ATT_RADIALGRADIENT_FX                              try_parse_length_or_percentage(svg, &(CAST(radial_gradient)->fx));
#define SVG_ATT_RADIALGRADIENT_FY                              try_parse_length_or_percentage(svg, &(CAST(radial_gradient)->fy));
#define SVG_ATT_RADIALGRADIENT_GRADIENTUNITS                   SVG_ATT_LINEARGRADIENT_GRADIENTUNITS
#define SVG_ATT_RADIALGRADIENT_GRADIENTTRANSFORM               SVG_ATT_LINEARGRADIENT_GRADIENTTRANSFORM
//===================

//=== STOP ===

#define PREPROC_STOP                                                                                                    \
    VkvgPattern pat = ((svg_class_gradient*)parentData)->pattern;                                                                   \
    svg_element_gradient_stop *stop = _new_gradient_stop();                                                             \
    stop->color = 0xFF000000;                                                                                           \
    stop->opacity = 1.f;                                                                                                \
    parentData = stop;
// todo multiple compress/decompress of colors
#define POSTPROC_STOP                                                                                                   \
    float a = (float)((stop->color & 0xff000000) >> 24) / 255.0f;                                                       \
    float b = (float)((stop->color & 0x00ff0000) >> 16) / 255.0f;                                                       \
    float g = (float)((stop->color & 0x0000ff00) >> 8) / 255.0f;                                                        \
    float r = (float)(stop->color & 0x000000ff) / 255.0f;                                                               \
    vkvg_pattern_add_color_stop(pat, stop->offset, r, g, b, a * stop->opacity);

#define SVG_ATT_STOP_COLOR                                                          \
    svg_paint_type enabled;                                                         \
    uint32_t color;                                                                 \
    if (try_parse_color(&svg->value, svg->value + svg->value_len, &enabled, &color))\
        CAST(gradient_stop)->color = color;
#define SVG_ATT_STOP_OPACITY    CAST(gradient_stop)->opacity = parse_ratio(svg);
#define SVG_ATT_OFFSET          CAST(gradient_stop)->offset = parse_ratio(svg);

//============

//=== DEFS ===
#define PREPROC_DEFS    svg->inDefs = true;
#define POSTPROC_DEFS   svg->inDefs = false;
//============

//=== USES ===
//#define PREPROC_USE     _process_use(svg, &attribs);
//#define POSTPROC_USE
/*#define PROCESS_USE_X
#define PROCESS_USE_Y
#define PROCESS_USE_WIDTH
#define PROCESS_USE_HEIGHT*/
//============
#define SVG_ATT_STYLE       svg->style      = svg->value;                   \
                            svg->style_end  = svg->value + svg->value_len;
#define SVG_ATT_ID          svg->currentIdHash = hash_svg_id(svg->value, svg->value_len);
#define SVG_ATT_COLOR       try_parse_color(&svg->value, svg->value + svg->value_len, &attribs->color_type, &attribs->color);
#define SVG_ATT_STROKE      try_parse_color(&svg->value, svg->value + svg->value_len, &attribs->stroke_type, &attribs->stroke);
#define SVG_ATT_FILL        try_parse_color(&svg->value, svg->value + svg->value_len, &attribs->fill_type, &attribs->fill);
#define SVG_ATT_FILL_RULE                                               \
    if (!strncasecmp ((char*)svg->value, "evenodd", svg->value_len))           \
        vkvg_set_fill_rule(svg->ctx, VKVG_FILL_RULE_EVEN_ODD);          \
    else if (!strncasecmp ((char*)svg->value, "nonzero", svg->value_len))      \
        vkvg_set_fill_rule(svg->ctx, VKVG_FILL_RULE_NON_ZERO);          \
    else {                                                              \
        LOG("Unrecognized fill-rule: %.*s", (int)svg->value_len, svg->value);\
    }
#define SVG_ATT_STROKE_LINECAP                                                  \
    if (!strncasecmp ((char*)svg->value, "butt", svg->value_len))                      \
        vkvg_set_line_cap(svg->ctx, VKVG_LINE_CAP_BUTT);                        \
    else if (!strncasecmp ((char*)svg->value, "round", svg->value_len))                \
        vkvg_set_line_cap(svg->ctx, VKVG_LINE_CAP_ROUND);                       \
    else if (!strncasecmp ((char*)svg->value, "square", svg->value_len))               \
        vkvg_set_line_cap(svg->ctx, VKVG_LINE_CAP_SQUARE);                      \
    else {                                                                      \
        LOG("Unrecognized stroke-linecap: %.*s", (int)svg->value_len, svg->value);   \
    }
#define SVG_ATT_STROKE_LINEJOIN                                                 \
    if (!strncasecmp ((char*)svg->value, "miter", svg->value_len))                     \
        vkvg_set_line_join(svg->ctx, VKVG_LINE_JOIN_MITER);                     \
    else if (!strncasecmp ((char*)svg->value, "round", svg->value_len))                \
        vkvg_set_line_join(svg->ctx, VKVG_LINE_JOIN_ROUND);                     \
    else if (!strncasecmp ((char*)svg->value, "bevel", svg->value_len))                \
        vkvg_set_line_join(svg->ctx, VKVG_LINE_JOIN_BEVEL);                     \
    else {                                                                      \
        LOG("Unrecognized linejoin: %.*s", (int)svg->value_len, svg->value);    \
    }
#define SVG_ATT_STROKE_WIDTH try_parse_length_or_percentage(svg, &attribs->stroke_width);

#define SVG_ATT_OPACITY         attribs->opacity = parse_ratio(svg);
#define SVG_ATT_FILL_OPACITY    attribs->fill_opacity = parse_ratio(svg);
#define SVG_ATT_STROKE_OPACITY  attribs->stroke_opacity = parse_ratio(svg);


#define SVG_ATT_SVG_WIDTH   try_parse_length_or_percentage(svg, &svg->width);
#define SVG_ATT_SVG_HEIGHT  try_parse_length_or_percentage(svg, &svg->height);
#define SVG_ATT_SVG_VIEWBOX svg->hasViewBox = try_parse_viewbox(svg);
#define SVG_ATT_TRANSFORM   apply_transform(svg);

#include "parser_gen.h"

svg_element_linear_gradient *_new_linear_gradient() {
    svg_element_linear_gradient *g = (svg_element_linear_gradient *)calloc(1, sizeof(svg_element_linear_gradient));
    g->id.type                     = svg_element_type_linear_gradient;
    g->x1                          = (svg_length_or_percentage){0, svg_unit_percentage};
    g->y1                          = (svg_length_or_percentage){0, svg_unit_percentage};
    g->x2                          = (svg_length_or_percentage){100, svg_unit_percentage};
    g->y2                          = (svg_length_or_percentage){0, svg_unit_percentage};
    return g;
}
svg_element_radial_gradient *_new_radial_gradient() {
    svg_element_radial_gradient *g = (svg_element_radial_gradient *)calloc(1, sizeof(svg_element_radial_gradient));
    g->id.type                     = svg_element_type_radial_gradient;
    g->gradientUnits               = svg_gradient_unit_objectBoundingBox;
    g->cx                          = (svg_length_or_percentage){50, svg_unit_percentage};
    g->cy                          = (svg_length_or_percentage){50, svg_unit_percentage};
    g->fx                          = (svg_length_or_percentage){50, svg_unit_percentage};
    g->fy                          = (svg_length_or_percentage){50, svg_unit_percentage};
    g->r                           = (svg_length_or_percentage){50, svg_unit_percentage};
    return g;
}

// A perfectly optimized, branchless UTF-8 length lookup table
static const uint8_t utf8_len_table[256] = {
    // 0x00 to 0x7F: Standard ASCII (1 byte)
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,

    // 0x80 to 0xBF: Continuation bytes (invalid as first byte, but default to 1 to prevent infinite loops)
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,

    // 0xC0 to 0xDF: Multi-byte leaders (2 bytes)
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,

    // 0xE0 to 0xEF: Multi-byte leaders (3 bytes)
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,

    // 0xF0 to 0xF7: Multi-byte leaders (4 bytes)
    4,4,4,4,4,4,4,4,

    // 0xF8 to 0xFF: Invalid UTF-8 (safely fallback to 1)
    1,1,1,1,1,1,1,1
};

int get_utf8_char_length(uint8_t first_byte) {
    return utf8_len_table[first_byte];
}

//#define ISWHITESPACE(c) (c == 0x9 || c == 0x20 || c == 0xA || c == 0xD)
//#define READ if (++ptr < buff_size) c = buff[ptr]; else break

static const double POWERS_OF_10[] = {
    1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0,
    1000000.0, 10000000.0, 100000000.0, 1000000000.0
};


// Helper to skip SVG coordinate separators (spaces, commas, or multiple spaces)
static inline const uint8_t* skip_separators(const uint8_t *buff, const uint8_t *const buff_end) {
    while (buff < buff_end && (*buff == ' ' || *buff == '\t' || *buff == ',' || *buff == '\r' || *buff == '\n')) {
        buff++;
    }
    return buff;
}
static inline const uint8_t* skip_whitespaces(const uint8_t *buff, const uint8_t *const buff_end) {
    while (buff < buff_end && (*buff == ' ' || *buff == '\t' || *buff == '\r' || *buff == '\n')) {
        buff++;
    }
    return buff;
}
static inline bool try_skip_separators(const uint8_t **buff_ptr, const uint8_t *const restrict buff_end) {
    const uint8_t *buff = *buff_ptr;
    while (buff < buff_end && (*buff == ' ' || *buff == ',' || *buff == '\t' || *buff == '\r' || *buff == '\n')) {
        buff++;
    }
    *buff_ptr = buff;
    return buff < buff_end;
}
static inline bool try_skip_whitespaces(const uint8_t **buff_ptr, const uint8_t *const restrict buff_end) {
    const uint8_t *buff = *buff_ptr;
    while (buff < buff_end && (*buff == ' ' || *buff == '\t' || *buff == '\r' || *buff == '\n')) {
        buff++;
    }
    *buff_ptr = buff;
    return buff < buff_end;
}
bool try_parse_float(const uint8_t **buff_ptr, const uint8_t *const restrict buff_end, float *const out_value) {
    const uint8_t *buff = *buff_ptr;
    *out_value = 0;

    if (buff >= buff_end) return false;

    // 1. Process optional sign token
    bool negative = false;
    if (*buff == '-') {
        negative = true;
        buff++;
    } else if (*buff == '+') {
        buff++;
    }

    // Check if we have at least one digit or a decimal dot ahead to validate structure
    if (buff >= buff_end || !((*buff >= '0' && *buff <= '9') || *buff == '.')) {
        return false;
    }

    // 2. Accumulate whole integer component
    double value = 0.0;
    bool has_digits = false;
    while (buff < buff_end && *buff >= '0' && *buff <= '9') {
        value = (value * 10.0) + (*buff - '0');
        buff++;
        has_digits = true;
    }

           // 3. Accumulate fraction component
    if (buff < buff_end && *buff == '.') {
        buff++; // step over '.'

        double frac_value = 0.0;
        const uint8_t *frac_start = buff;

        while (buff < buff_end && *buff >= '0' && *buff <= '9') {
            frac_value = (frac_value * 10.0) + (*buff - '0');
            buff++;
        }

        size_t frac_len = buff - frac_start;
        if (frac_len > 0) {
            has_digits = true;
            if (frac_len < 10) {
                value += frac_value / POWERS_OF_10[frac_len];
            } else {
                double divisor = 1000000000.0;
                for (size_t i = 9; i < frac_len; i++) divisor *= 10.0;
                value += frac_value / divisor;
            }
        } else if (!has_digits) {
            // Found just a '.' or '-.' without any trailing digits
            return false;
        }
    }

           // 4. Handle scientific notation exponent parsing
    if (buff < buff_end && (*buff == 'e' || *buff == 'E')) {
        const uint8_t *exp_checkpoint = buff;
        buff++;

        bool exp_negative = false;
        if (buff < buff_end && *buff == '-') {
            exp_negative = true;
            buff++;
        } else if (buff < buff_end && *buff == '+') {
            buff++;
        }

        if (buff >= buff_end || *buff < '0' || *buff > '9') {
            // Malformed exponent notation (e.g. "1.2e" or "1.2e-"). Rollback exponent block only.
            buff = exp_checkpoint;
            goto emit_result;
        }

        int exponent = 0;
        while (buff < buff_end && *buff >= '0' && *buff <= '9') {
            exponent = (exponent * 10) + (*buff - '0');
            buff++;
        }

        if (exponent > 0 && exponent < 10) {
            if (exp_negative) value /= POWERS_OF_10[exponent];
            else value *= POWERS_OF_10[exponent];
        } else if (exponent >= 10) {
            for (int i = 0; i < exponent; i++) {
                if (exp_negative) value /= 10.0;
                else value *= 10.0;
            }
        }
    }

emit_result:
    // 5. Hard validation: verify we consumed numerical structural characters
    if (!has_digits) return false;

           // Success commit phase: safely update the cursor position and store output
    *buff_ptr  = buff;
    *out_value = negative ? (float)(-value) : (float)value;
    return true;
}
bool try_parse_floats(const uint8_t **buff_ptr, const uint8_t *const restrict buff_end, int floatCount, ...) {
    const uint8_t *buff = *buff_ptr;
    va_list args;
    va_start(args, floatCount);

    for (int i = 0; i < floatCount; i++) {
        float *pF = va_arg(args, float *);

        buff = skip_whitespaces(buff, buff_end);
        if (!try_parse_float(&buff, buff_end, pF)) {
            va_end(args);
            *buff_ptr = buff;
            return false;
        }
        buff = skip_separators(buff, buff_end);
    }
    va_end(args);
    *buff_ptr = buff;
    return true;
}

bool try_find_by_id(svg_context *const svg, uint32_t hash, void **elt) {
    *elt = NULL;
    for (uint32_t i = 0; i < svg->idList->count; i++) {
        if (_get_element_hash(svg->idList->elements[i]) == hash) {
            *elt = svg->idList->elements[i];
            return true;
        }
    }
    return false;
}
void _store_or_throw(svg_context *const svg, void *elt) {
    if (svg->currentIdHash) {
        svg_element_header *id = (svg_element_header *)elt;
        id->hash               = svg->currentIdHash;
        array_add(svg->idList, elt);
        LOG("SVG: store elemnt: %u type:%u\n", id->hash, id->type);
    } else {
        free(elt);
    }
}

// Helper to check if a character is a hex digit
static inline bool is_hex_digit(uint8_t c, uint32_t *const val) {
    if (c >= '0' && c <= '9') { *val = c - '0'; return true; }
    if (c >= 'a' && c <= 'f') { *val = 10 + (c - 'a'); return true; }
    if (c >= 'A' && c <= 'F') { *val = 10 + (c - 'A'); return true; }
    return false;
}

// Fast branchless helper to parse 1 to 3 integer digits from an RGB(A) functional stream
static inline bool parse_rgb_channel(const uint8_t **ptr, const uint8_t *const end, uint32_t *const out_val) {
    while (*ptr < end && (**ptr == ' ' || **ptr == '\t' || **ptr == ',')) (*ptr)++;
    if (*ptr >= end || **ptr < '0' || **ptr > '9') return false;

    uint32_t val = 0;
    int digits = 0;
    while (*ptr < end && **ptr >= '0' && **ptr <= '9' && digits < 3) {
        val = (val * 10) + (**ptr - '0');
        (*ptr)++;
        digits++;
    }
    if (val > 255) val = 255;
    *out_val = val;
    return true;
}

bool try_parse_color(const uint8_t **buff_ptr, const uint8_t *const buff_end, svg_paint_type *isEnabled, uint32_t *colorValue) {
    const uint8_t *ptr = *buff_ptr;
    *colorValue = 0;
    *isEnabled = svg_paint_type_none;

    if (ptr >= buff_end) {
        LOG("Unexpected end of file while parsing color: %.*s\n", (int)(buff_end - *buff_ptr), *buff_ptr);
        return false;
    }

    // 1. Handle Hexadecimal Formats (#FFF or #FFFFFF)
    if (*ptr == '#') {
        ptr++;
        const uint8_t *hex_start = ptr;
        uint32_t digits[6];
        int count = 0;

        while (ptr < buff_end && count < 6 && is_hex_digit(*ptr, &digits[count])) {
            ptr++;
            count++;
        }

        if (count == 3) { // Translate 0xRGB to 0xAABBGGRR
            uint32_t r = (digits[0] << 4) | digits[0];
            uint32_t g = (digits[1] << 4) | digits[1];
            uint32_t b = (digits[2] << 4) | digits[2];
            *colorValue = 0xFF000000 | (b << 16) | (g << 8) | r;
        } else if (count == 6) { // Translate 0xRRGGBB to 0xAABBGGRR
            uint32_t r = (digits[0] << 4) | digits[1];
            uint32_t g = (digits[2] << 4) | digits[3];
            uint32_t b = (digits[4] << 4) | digits[5];
            *colorValue = 0xFF000000 | (b << 16) | (g << 8) | r;
        } else {
            LOG("Malformed hex string for color: %.*s\n", (int)(buff_end - *buff_ptr), *buff_ptr);
            return false; // Malformed hex string
        }

        *isEnabled = svg_paint_type_solid;
        *buff_ptr = ptr;
        return true;
    }

    // 2. Handle functional notations: url(...), rgb(...), rgba(...)
    size_t len = (size_t)(buff_end - ptr);

    if (len >= 5 && !memcmp(ptr, "url", 3)) {
        ptr += 3;
        while (ptr < buff_end && (*ptr == ' ' || *ptr == '\t' || *ptr == '(')) ptr++;
        if (ptr < buff_end && *ptr == '#') {
            ptr++;
            const uint8_t *iri_start = ptr;
            while (ptr < buff_end && *ptr != ')' && *ptr != ' ' && *ptr != '\t') ptr++;

            // Generate IRI reference hash using your precise 32-bit FNV-1a routine
            *colorValue = hash_svg_id(iri_start, ptr - iri_start);
            *isEnabled = svg_paint_type_pattern;

            while (ptr < buff_end && *ptr != ')') ptr++;
            if (ptr < buff_end && *ptr == ')') ptr++;
            *buff_ptr = ptr;
            return true;
        }
        LOG("Malformed url string for color: %.*s\n", (int)(buff_end - *buff_ptr), *buff_ptr);
        return false;
    }

    if (len >= 10 && (!memcmp(ptr, "rgb", 3) || !memcmp(ptr, "rgba", 4))) {
        bool is_rgba = (ptr[3] == 'a');
        ptr += is_rgba ? 4 : 3;

        while (ptr < buff_end && (*ptr == ' ' || *ptr == '\t' || *ptr == '(')) ptr++;

        uint32_t r, g, b, a = 255;
        if (!parse_rgb_channel(&ptr, buff_end, &r) ||
            !parse_rgb_channel(&ptr, buff_end, &g) ||
            !parse_rgb_channel(&ptr, buff_end, &b)) {
            LOG("Malformed rgb string for color: %.*s\n", (int)(buff_end - *buff_ptr), *buff_ptr);
            return false;
        }

        if (is_rgba) {
            // Note: Simplification for demo assuming integer alpha channel.
            // If handling floats (e.g. 0.5), tie try_parse_float right here.
            if (!parse_rgb_channel(&ptr, buff_end, &a)) {
                LOG("Malformed hex string for color: %.*s\n", (int)(buff_end - *buff_ptr), *buff_ptr);
                return false;
            }
        }

        while (ptr < buff_end && *ptr != ')') ptr++;
        if (ptr < buff_end && *ptr == ')') ptr++;

        // Output mapped Big-Endian layout optimized for raw framebuffers
        *colorValue = (a << 24) | (b << 16) | (g << 8) | r;
        *isEnabled = svg_paint_type_solid;
        *buff_ptr = ptr;
        return true;
    }

   // 3. Fallback: Named keyword parsing ("none", "currentColor", "red")
   // Find the word boundary boundary block
    const uint8_t *word_end = ptr;
    while (word_end < buff_end && *word_end != ' ' && *word_end != '\t' && *word_end != ';' && *word_end != ')') {
        word_end++;
    }
    size_t word_len = (size_t)(word_end - ptr);

    if (word_len == 4 && !memcmp(ptr, "none", 4)) {
        *colorValue = 0;
        *isEnabled = svg_paint_type_none;
        *buff_ptr = word_end;
        return true;
    }

    if (word_len == 12 && !memcmp(ptr, "currentColor", 12)) {
        // Handle via current inherit status tracker injection flags
        *colorValue = 0xFFFFFFFF; // Fallback white or inherit
        *isEnabled = svg_paint_type_solid;
        *buff_ptr = word_end;
        return true;
    }

    // Direct O(1) Gperf color palette lookup implementation using our exact signature bounds
    const struct SvgColorName *matched = lookup_svg_color_names((const char *)ptr, word_end - ptr);
    if (matched) {
        *colorValue = matched->value;
        *isEnabled = svg_paint_type_solid;
        *buff_ptr = word_end;
        return true;
    }
    LOG("Unknown color: %.*s\n", (int)(buff_end - *buff_ptr), *buff_ptr);
    return false; // Unknown token format
}
// Fast branchless helper to map 2-character unit suffix bytes to their enum IDs
static inline bool parse_two_char_unit(uint8_t c0, uint8_t c1, svg_unit *const out_unit) {
    // Convert to lowercase via bit manipulation (works perfectly for standard ASCII)
    uint32_t u0 = c0 | 0x20;
    uint32_t u1 = c1 | 0x20;
    uint32_t packed = (u0 << 8) | u1;

    switch (packed) {
    case (('p' << 8) | 'x'): *out_unit = svg_unit_px;  return true;
    case (('p' << 8) | 't'): *out_unit = svg_unit_pt;  return true;
    case (('p' << 8) | 'c'): *out_unit = svg_unit_pc;  return true;
    case (('i' << 8) | 'n'): *out_unit = svg_unit_in;  return true;
    case (('m' << 8) | 'm'): *out_unit = svg_unit_mm;  return true;
    case (('c' << 8) | 'm'): *out_unit = svg_unit_cm;  return true;
    // Extended SVG units if encountered in layout rules:
    case (('e' << 8) | 'm'): *out_unit = svg_unit_px;  return true; // Fallback mapping
    case (('e' << 8) | 'x'): *out_unit = svg_unit_px;  return true;
    }
    return false;
}

bool try_parse_length_or_percentage(svg_context *const svg, svg_length_or_percentage *const lop) {
    const uint8_t *ptr = svg->value;
    const uint8_t *const buff_end = svg->value + svg->value_len;

    // 1. Core numeric extraction utilizing your ultra-fast streaming float engine
    if (!try_parse_float(&ptr, buff_end, &lop->number)) {
        return false;
    }

    // Default to raw pixel scaling factor if we immediately hit stream structural barriers
    if (ptr >= buff_end) {
        lop->units = svg_unit_px;
        return true;
    }

    // 2. Continuous branchless parsing of trailing unit suffix token bytes
    uint8_t c0 = *ptr;

    // Single-character suffix identifier checks
    if (c0 == '%') {
        lop->units = svg_unit_percentage;
        return true;
    }
    if (c0 == 'q' || c0 == 'Q') {
        lop->units = svg_unit_Q;
        return true;
    }

    // Check if we have at least 2 remaining characters to parse a double unit token
    if (ptr + 1 < buff_end) {
        uint8_t c1 = *(ptr + 1);
        svg_unit parsed_unit;

        if (parse_two_char_unit(c0, c1, &parsed_unit)) {
            lop->units = parsed_unit;
            return true;
        }
    }

    // If text trailing characters aren't valid SVG dimensions units (e.g. standard whitespace,
    // commas, or closing brackets), the specification commands defaulting to pixel units.
    lop->units = svg_unit_px;
    return true;
}
bool try_parse_transform(svg_context *const svg, vkvg_matrix_t *const mat) {
    const uint8_t *buff = svg->value;
    const uint8_t *const buff_end = svg->value + svg->value_len;

    while (buff < buff_end) {
        // Skip leading whitespace or transform list separators (spaces, commas)
        buff = skip_separators(buff, buff_end);
        if (buff >= buff_end) break;

               // 1. Identify the transform function name tokens
        const uint8_t *name_start = buff;
        while (buff < buff_end && *buff != '(' && *buff != ' ' && *buff != '\t') {
            buff++;
        }
        size_t name_len = (size_t)(buff - name_start);

               // Advance past any trailing spaces to find the opening bracket '('
        buff = skip_whitespaces(buff, buff_end);
        if (buff >= buff_end || *buff != '(') {
            LOG("error parsing transform: missing opening parenthesis '(' in '%.*s'\n", (int)svg->value_len, svg->value);
            return false;
        }
        buff++; // step over '('

               // 2. Route tokens to appropriate transformation handlers
        if (name_len == 4 && !memcmp(name_start, "none", 4)) {
            break;
        }
        else if (name_len == 6 && !memcmp(name_start, "matrix", 6)) {
            vkvg_matrix_t m, newMat;
            if (!try_parse_floats(&buff, buff_end, 6, &m.xx, &m.yx, &m.xy, &m.yy, &m.x0, &m.y0)) {
                LOG("error parsing transformation matrix values\n");
                return false;
            }
            vkvg_matrix_multiply(&newMat, &m, mat);
            *mat = newMat;
        }
        else if (name_len == 9 && !memcmp(name_start, "translate", 9)) {
            float dx = 0.0f, dy = 0.0f;
            if (!try_parse_floats(&buff, buff_end, 1, &dx)) {
                LOG("error parsing translation component X\n");
                return false;
            }
            // dy is optional in translation transforms; safely try parsing it
            buff = skip_separators(buff, buff_end);
            try_parse_float(&buff, buff_end, &dy);

            vkvg_matrix_translate(mat, dx, dy);
        }
        else if (name_len == 5 && !memcmp(name_start, "scale", 5)) {
            float sx = 0.0f, sy = 0.0f;
            if (!try_parse_floats(&buff, buff_end, 1, &sx)) {
                LOG("error parsing scale factor X\n");
                return false;
            }
            // sy is optional; if missing, default to uniform scaling (sy = sx)
            buff = skip_separators(buff, buff_end);
            if (!try_parse_float(&buff, buff_end, &sy)) {
                sy = sx;
            }
            vkvg_matrix_scale(mat, sx, sy);
        }
        else if (name_len == 6 && !memcmp(name_start, "rotate", 6)) {
            float angle = 0.0f, cx = 0.0f, cy = 0.0f;
            if (!try_parse_floats(&buff, buff_end, 1, &angle)) {
                LOG("error parsing rotation angle component\n");
                return false;
            }

            // Center parameters cx and cy are optionally provided as a pair
            buff = skip_separators(buff, buff_end);
            if (try_parse_float(&buff, buff_end, &cx)) {
                if (!try_parse_floats(&buff, buff_end, 1, &cy)) {
                    LOG("error parsing rotation center Y component\n");
                    return false;
                }
                // Correct transformation order for pivot rotations
                vkvg_matrix_translate(mat, cx, cy);
                vkvg_matrix_rotate(mat, degToRad(angle));
                vkvg_matrix_translate(mat, -cx, -cy);
            } else {
                vkvg_matrix_rotate(mat, degToRad(angle));
            }
        }
        else {
            LOG("unimplemented or unrecognized transform token: %.*s\n", (int)name_len, name_start);
            return false;
        }

               // 3. Clear closing parenthesis structural markers
        buff = skip_whitespaces(buff, buff_end);
        if (buff >= buff_end || *buff != ')') {
            LOG("error parsing transform string: expecting trailing ')'\n");
            return false;
        }
        buff++; // step over ')'
    }

    return true;
}

bool try_parse_viewbox(svg_context *const svg) {
    // 1. Establish the text boundaries based on your isolated attribute value
    const uint8_t *buff           = svg->value;
    const uint8_t *const buff_end = svg->value + svg->value_len;

           // 2. Sequentially stream-parse the 4 coordinates in a single pass
    buff = skip_separators(buff, buff_end);
    if (!try_parse_float(&buff, buff_end, &svg->viewBox.x)) return false;

    buff = skip_separators(buff, buff_end);
    if (!try_parse_float(&buff, buff_end, &svg->viewBox.y)) return false;

    buff = skip_separators(buff, buff_end);
    if (!try_parse_float(&buff, buff_end, &svg->viewBox.w)) return false;

    buff = skip_separators(buff, buff_end);
    if (!try_parse_float(&buff, buff_end, &svg->viewBox.h)) return false;

    // Success check: Ensure we parsed all 4 dimensions cleanly
    return true;
}

void _copy_pattern_color_stops(VkvgPattern const orig, VkvgPattern const dest) {
    uint32_t stopCount;
    if (vkvg_pattern_get_color_stop_count(orig, &stopCount) == VKVG_STATUS_SUCCESS) {
        for (uint32_t i = 0; i < stopCount; i++) {
            float offset, r, g, b, a;
            vkvg_pattern_get_color_stop_rgba(orig, i, &offset, &r, &g, &b, &a);
            vkvg_pattern_add_color_stop(dest, offset, r, g, b, a);
        }
    } else
        LOG("Error processing referenced pattern\n");
}
void _resolve_pattern_href(svg_context *svg, void *rootElt, VkvgPattern pat) {
    void               *elt = NULL;
    svg_element_header *id  = (svg_element_header *)rootElt;
    while (id->xlinkHref) {
        if (try_find_by_id(svg, id->xlinkHref, &elt)) {
            id->xlinkHref = 0; // reset once resolved
            id            = (svg_element_header *)elt;
        } else {
            LOG("xlink:href svg element error  %.*s\n", (int)svg->value_len, svg->value);
            return;
        }
    }
    if (elt) {
        VkvgPattern refPatter = NULL;
        if (_get_element_type(elt) == svg_element_type_radial_gradient)
            refPatter = (VkvgPattern)((svg_element_radial_gradient *)elt)->pattern;
        else if (_get_element_type(elt) == svg_element_type_linear_gradient)
            refPatter = (VkvgPattern)((svg_element_linear_gradient *)elt)->pattern;
        else {
            LOG("xlink:href svg element error, expecting gradient%.*s\n", (int)svg->value_len, svg->value);
            return;
        }
        _copy_pattern_color_stops(refPatter, pat);
        LOG("_resolve_pattern_href: status: %d\n", vkvg_pattern_status(refPatter));
    }

}
void set_pattern(svg_context *svg, uint32_t patternHash) {
    void       *elt;
    VkvgPattern pat;
    if (try_find_by_id(svg, patternHash, &elt)) {
        switch (_get_element_type(elt)) {
        case svg_element_type_linear_gradient: {
            CASTELT(g,linear_gradient,elt);
            _resolve_pattern_href(svg, elt, g->pattern);

            pat = g->pattern;

            float x0 = 0, y0 = 0, x1, y1;
            if (g->gradientUnits == svg_gradient_unit_objectBoundingBox)
                vkvg_path_extents(svg->ctx, &x0, &y0, &x1, &y1);
            else {
                x0 = svg->viewBox.x;
                y0 = svg->viewBox.y;
                x1 = svg->viewBox.x + svg->viewBox.w;
                y1 = svg->viewBox.y + svg->viewBox.h;
            }

            float w = x1 - x0;
            float h = y1 - y0;

            float px0, py0, px1, py1;
            px0 = _get_pixel_coord(w, &g->x1) + x0;
            py0 = _get_pixel_coord(h, &g->y1) + y0;
            px1 = _get_pixel_coord(w, &g->x2) + x0;
            py1 = _get_pixel_coord(h, &g->y2) + y0;

            vkvg_pattern_edit_linear(g->pattern, px0, py0, px1, py1);
            vkvg_set_source(svg->ctx, g->pattern);
        }

        break;
        case svg_element_type_radial_gradient: {
            CASTELT(g,radial_gradient,elt);

            _resolve_pattern_href(svg, elt, g->pattern);

            pat      = g->pattern;
            float x0 = 0, y0 = 0, x1, y1;
            if (g->gradientUnits == svg_gradient_unit_objectBoundingBox)
                vkvg_path_extents(svg->ctx, &x0, &y0, &x1, &y1);
            else {
                x0 = svg->viewBox.x;
                y0 = svg->viewBox.y;
                x1 = svg->viewBox.x + svg->viewBox.w;
                y1 = svg->viewBox.y + svg->viewBox.h;
            }

            float w = x1 - x0;
            float h = y1 - y0;

            float cx, cy, fx, fy, r;
            cx = _get_pixel_coord(w, &g->cx) + x0;
            cy = _get_pixel_coord(h, &g->cy) + y0;
            fx = _get_pixel_coord(w, &g->fx) + x0;
            fy = _get_pixel_coord(h, &g->fy) + y0;
            r  = _get_pixel_coord(w, &g->r);

            vkvg_pattern_edit_radial(g->pattern, cx, cy, 0, fx, fy, r);
            vkvg_set_source(svg->ctx, g->pattern);
        } break;
        }
    } else
        LOG("pattern hash not resolved: %d\n", patternHash);
}
void parse_point_list(svg_context *const svg, const uint8_t *const restrict buff_ptr, const uint8_t *const restrict buff_end) {
    const uint8_t *buff = buff_ptr;
    float x, y;
    if (try_parse_floats (&buff, buff_end, 2, &x, &y)) {
        vkvg_move_to(svg->ctx, x, y);
        while (try_parse_floats (&buff, buff_end, 2, &x, &y)) {
            vkvg_line_to(svg->ctx, x, y);
        }
    }
}
void _parse_path_d_attribute(svg_context *const svg, const uint8_t *const restrict buff_ptr, const uint8_t *const restrict buff_end) {
    const uint8_t *buff = buff_ptr;
    float        x, y, c1x, c1y, c2x, c2y, cpX, cpY, rx, ry, rotx;
    bool         large, sweep;
    enum prevCmd prev     = none;
    int          repeat   = 0;
    float        subpathX = 0, subpathY = 0;
    char         c;

    bool result = false;
    while (try_skip_whitespaces(&buff, buff_end)) {

        if (!repeat)
            c = *buff++;

        switch (c) {
        case 'M':
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 1, &y);
            else
                result = try_parse_floats(&buff, buff_end, 2, &x, &y);
            if (result) {
                if (repeat)
                    vkvg_line_to(svg->ctx, x, y);
                else {
                    vkvg_move_to(svg->ctx, x, y);
                    subpathX = x;
                    subpathY = y;
                }
                repeat = try_parse_floats(&buff, buff_end, 1, &x);
            } else
                return;
            break;
        case 'm':
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 1, &y);
            else
                result = try_parse_floats(&buff, buff_end, 2, &x, &y);
            if (result) {
                if (repeat)
                    vkvg_rel_line_to(svg->ctx, x, y);
                else {
                    vkvg_rel_move_to(svg->ctx, x, y);
                    vkvg_get_current_point(svg->ctx, &subpathX, &subpathY);
                }
                repeat = try_parse_floats(&buff, buff_end, 1, &x);
            } else
                return;
            break;
        case 'L':
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 1, &y);
            else
                result = try_parse_floats(&buff, buff_end, 2, &x, &y);
            if (result) {
                vkvg_line_to(svg->ctx, x, y);
                repeat = try_parse_floats(&buff, buff_end, 1, &x);
            } else
                return;
            break;
        case 'l':
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 1, &y);
            else
                result = try_parse_floats(&buff, buff_end, 2, &x, &y);
            if (result) {
                vkvg_rel_line_to(svg->ctx, x, y);
                repeat = try_parse_floats(&buff, buff_end, 1, &x);
            } else
                return;
            break;
        case 'H':
            if (!repeat) {
                if (!try_parse_floats(&buff, buff_end, 1, &x))
                    return;
            }
            vkvg_get_current_point(svg->ctx, &c1x, &c1y);
            vkvg_line_to(svg->ctx, x, c1y);
            repeat = try_parse_floats(&buff, buff_end, 1, &x);
            break;
        case 'h':
            if (!repeat) {
                if (!try_parse_floats(&buff, buff_end, 1, &x))
                    return;
            }
            vkvg_rel_line_to(svg->ctx, x, 0);
            repeat = try_parse_floats(&buff, buff_end, 1, &x);
            break;
        case 'V':
            if (!repeat) {
                if (!try_parse_floats(&buff, buff_end, 1, &y))
                    return;
            }
            vkvg_get_current_point(svg->ctx, &c1x, &c1y);
            vkvg_line_to(svg->ctx, c1x, y);
            repeat = try_parse_floats(&buff, buff_end, 1, &y);
            break;
        case 'v':
            if (!repeat) {
                if (!try_parse_floats(&buff, buff_end, 1, &y))
                    return;
            }
            vkvg_rel_line_to(svg->ctx, 0, y);
            repeat = try_parse_floats(&buff, buff_end, 1, &y);
            break;
        case 'Q':
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 3, &c1y, &x, &y);
            else
                result = try_parse_floats(&buff, buff_end, 4, &c1x, &c1y, &x, &y);
            if (result) {
                vkvg_quadratic_to(svg->ctx, c1x, c1y, x, y);
                prev   = quad;
                repeat = try_parse_floats(&buff, buff_end, 1, &c1x);
                continue;
            } else
                return;
            break;
        case 'q':
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 3, &c1y, &x, &y);
            else
                result = try_parse_floats(&buff, buff_end, 4, &c1x, &c1y, &x, &y);
            if (result) {
                vkvg_get_current_point(svg->ctx, &cpX, &cpY);
                vkvg_rel_quadratic_to(svg->ctx, c1x, c1y, x, y);
                prev = quad;
                c1x += cpX;
                c1y += cpY;
                repeat = try_parse_floats(&buff, buff_end, 1, &c1x);
                continue;
            } else
                return;
            break;
        case 'T':
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 1, &y);
            else
                result = try_parse_floats(&buff, buff_end, 2, &x, &y);
            if (result) {
                vkvg_get_current_point(svg->ctx, &cpX, &cpY);
                if (prev == quad) {
                    c1x = 2.0 * cpX - c1x;
                    c1y = 2.0 * cpY - c1y;
                } else {
                    c1x = x;
                    c1y = y;
                }
                vkvg_quadratic_to(svg->ctx, c1x, c1y, x, y);
                prev   = quad;
                repeat = try_parse_floats(&buff, buff_end, 1, &x);
                continue;
            } else
                return;
            break;
        case 't':
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 1, &y);
            else
                result = try_parse_floats(&buff, buff_end, 2, &x, &y);
            if (result) {
                vkvg_get_current_point(svg->ctx, &cpX, &cpY);
                if (prev == quad) {
                    c1x = (cpX - c1x);
                    c1y = (cpY - c1y);
                } else {
                    c1x = x;
                    c1y = y;
                }
                vkvg_rel_quadratic_to(svg->ctx, c1x, c1y, x, y);
                prev = quad;
                c1x += cpX;
                c1y += cpY;
                repeat = try_parse_floats(&buff, buff_end, 1, &x);
                continue;
            } else
                return;
            break;
        case 'C':
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 5, &c1y, &c2x, &c2y, &x, &y);
            else
                result = try_parse_floats(&buff, buff_end, 6, &c1x, &c1y, &c2x, &c2y, &x, &y);
            if (result) {
                vkvg_curve_to(svg->ctx, c1x, c1y, c2x, c2y, x, y);
                prev   = cubic;
                repeat = try_parse_floats(&buff, buff_end, 1, &c1x);
                continue;
            } else
                return;
            break;
        case 'c':
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 5, &c1y, &c2x, &c2y, &x, &y);
            else
                result = try_parse_floats(&buff, buff_end, 6, &c1x, &c1y, &c2x, &c2y, &x, &y);
            if (result) {
                vkvg_get_current_point(svg->ctx, &cpX, &cpY);
                vkvg_rel_curve_to(svg->ctx, c1x, c1y, c2x, c2y, x, y);
                c2x += cpX;
                c2y += cpY;
                prev   = cubic;
                repeat = try_parse_floats(&buff, buff_end, 1, &c1x);
                continue;
            } else
                return;
            break;
        case 'S':
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 3, &c1y, &x, &y);
            else
                result = try_parse_floats(&buff, buff_end, 4, &c1x, &c1y, &x, &y);
            if (result) {
                vkvg_get_current_point(svg->ctx, &cpX, &cpY);
                if (prev == cubic)
                    vkvg_curve_to(svg->ctx, 2.0 * cpX - c2x, 2.0 * cpY - c2y, c1x, c1y, x, y);
                else
                    vkvg_curve_to(svg->ctx, cpX, cpY, c1x, c1y, x, y);

                c2x    = c1x;
                c2y    = c1y;
                prev   = cubic;
                repeat = try_parse_floats(&buff, buff_end, 1, &c1x);
                continue;
            } else
                return;
            break;
        case 's':
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 3, &c1y, &x, &y);
            else
                result = try_parse_floats(&buff, buff_end, 4, &c1x, &c1y, &x, &y);
            if (result) {
                vkvg_get_current_point(svg->ctx, &cpX, &cpY);
                if (prev == cubic)
                    vkvg_rel_curve_to(svg->ctx, cpX - c2x, cpY - c2y, c1x, c1y, x, y);
                else
                    vkvg_rel_curve_to(svg->ctx, 0, 0, c1x, c1y, x, y);

                c2x    = cpX + c1x;
                c2y    = cpY + c1y;
                prev   = cubic;
                repeat = try_parse_floats(&buff, buff_end, 1, &c1x);
                continue;
            } else
                return;
            break;
        case 'A': // rx ry x-axis-rotation large-arc-flag sweep-flag x y
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 2, &ry, &rotx);
            else
                result = try_parse_floats(&buff, buff_end, 3, &rx, &ry, &rotx);
            if (result) {
                if (!try_parse_floats(&buff, buff_end, 2, &large, &sweep) || !try_parse_floats(&buff, buff_end, 2, &x, &y))
                    return;
                rotx = rotx * M_PIF / 180.0f;

                vkvg_elliptic_arc_to(svg->ctx, x, y, large, sweep, rx, ry, rotx);

                repeat = try_parse_floats(&buff, buff_end, 1, &rx);
            } else
                return;
            break;
        case 'a': // rx ry x-axis-rotation large-arc-flag sweep-flag x y
            if (repeat)
                result = try_parse_floats(&buff, buff_end, 2, &ry, &rotx);
            else
                result = try_parse_floats(&buff, buff_end, 3, &rx, &ry, &rotx);

            if (result) {
                if (!try_parse_floats(&buff, buff_end, 2, &large, &sweep) || !try_parse_floats(&buff, buff_end, 2, &x, &y))
                    return;
                rotx = degToRad(rotx);

                vkvg_rel_elliptic_arc_to(svg->ctx, x, y, large, sweep, rx, ry, rotx);

                repeat = try_parse_floats(&buff, buff_end, 1, &rx);
            } else
                return;
            break;
        case 'z':
        case 'Z':
            vkvg_close_path(svg->ctx);
            vkvg_move_to(svg->ctx, subpathX, subpathY);
            break;
        default:
            LOG("error parsing path: unexpected char: %c\n", c);
            return;
        }
        prev = none;
    }
}

/*void _parse_path_d_attribute2(svg_context *const svg, const uint8_t *const restrict buff_ptr, const uint8_t *const restrict buff_end) {
    const uint8_t *buff = buff_ptr;
    float        x, y, c1x, c1y, c2x, c2y, cpX, cpY, rx, ry, rotx;
    bool         large, sweep;
    enum prevCmd prev     = none;
    float        subpathX = 0, subpathY = 0;
    char         c;

    while (try_skip_whitespaces(&buff, buff_end)) {

        c = *buff++;

        switch (c) {
        case 'M':
            if (try_parse_floats(&buff, buff_end, 2, &x, &y)) {
                vkvg_move_to(svg->ctx, x, y);
                subpathX = x;
                subpathY = y;
                while (try_parse_floats(&buff, buff_end, 2, &x, &y))
                    vkvg_line_to(svg->ctx, x, y);
            }
            break;
        case 'm':
            if (try_parse_floats(&buff, buff_end, 2, &x, &y)){
                vkvg_rel_move_to(svg->ctx, x, y);
                vkvg_get_current_point(svg->ctx, &subpathX, &subpathY);
                while (try_parse_floats(&buff, buff_end, 2, &x, &y))
                    vkvg_rel_line_to(svg->ctx, x, y);
            }
            break;
        case 'L':
            while (try_parse_floats(&buff, buff_end, 2, &x, &y))
                vkvg_line_to(svg->ctx, x, y);
            break;
        case 'l':
            while (try_parse_floats(&buff, buff_end, 2, &x, &y))
                vkvg_rel_line_to(svg->ctx, x, y);
            break;
        case 'H':
            while (try_parse_floats(&buff, buff_end, 1, &x)) {
                vkvg_get_current_point(svg->ctx, &c1x, &c1y);
                vkvg_line_to(svg->ctx, x, c1y);
            }
            break;
        case 'h':
            while (try_parse_floats(&buff, buff_end, 1, &x))
                vkvg_rel_line_to(svg->ctx, x, 0);
            break;
        case 'V':
            while (try_parse_floats(&buff, buff_end, 1, &y)) {
                vkvg_get_current_point(svg->ctx, &c1x, &c1y);
                vkvg_line_to(svg->ctx, c1x, y);
            }
            break;
        case 'v':
            while (try_parse_floats(&buff, buff_end, 1, &y))
                vkvg_rel_line_to(svg->ctx, 0, y);
            break;
        case 'Q':
            while (try_parse_floats(&buff, buff_end, 4, &c1x, &c1y, &x, &y)) {
                vkvg_quadratic_to(svg->ctx, c1x, c1y, x, y);
                prev   = quad;
            }
            break;
        case 'q':
            while (try_parse_floats(&buff, buff_end, 4, &c1x, &c1y, &x, &y)) {
                vkvg_get_current_point(svg->ctx, &cpX, &cpY);
                vkvg_rel_quadratic_to(svg->ctx, c1x, c1y, x, y);
                prev = quad;
                c1x += cpX;
                c1y += cpY;
            }
            break;
        case 'T':
            while (try_parse_floats(&buff, buff_end, 2, &x, &y)) {
                vkvg_get_current_point(svg->ctx, &cpX, &cpY);
                if (prev == quad) {
                    c1x = 2.0 * cpX - c1x;
                    c1y = 2.0 * cpY - c1y;
                } else {
                    c1x = x;
                    c1y = y;
                }
                vkvg_quadratic_to(svg->ctx, c1x, c1y, x, y);
                prev   = quad;
            }
            break;
        case 't':
            while (try_parse_floats(&buff, buff_end, 2, &x, &y)) {
                vkvg_get_current_point(svg->ctx, &cpX, &cpY);
                if (prev == quad) {
                    c1x = (cpX - c1x);
                    c1y = (cpY - c1y);
                } else {
                    c1x = x;
                    c1y = y;
                }
                vkvg_rel_quadratic_to(svg->ctx, c1x, c1y, x, y);
                prev = quad;
                c1x += cpX;
                c1y += cpY;
            }
            break;
        case 'C':
            while (try_parse_floats(&buff, buff_end, 6, &c1x, &c1y, &c2x, &c2y, &x, &y)) {
                vkvg_curve_to(svg->ctx, c1x, c1y, c2x, c2y, x, y);
                prev   = cubic;
            }
            break;
        case 'c':
            while (try_parse_floats(&buff, buff_end, 6, &c1x, &c1y, &c2x, &c2y, &x, &y)) {
                vkvg_get_current_point(svg->ctx, &cpX, &cpY);
                vkvg_rel_curve_to(svg->ctx, c1x, c1y, c2x, c2y, x, y);
                c2x += cpX;
                c2y += cpY;
                prev   = cubic;
            }
            break;
        case 'S':
            while (try_parse_floats(&buff, buff_end, 4, &c1x, &c1y, &x, &y)) {
                vkvg_get_current_point(svg->ctx, &cpX, &cpY);
                if (prev == cubic)
                    vkvg_curve_to(svg->ctx, 2.0 * cpX - c2x, 2.0 * cpY - c2y, c1x, c1y, x, y);
                else
                    vkvg_curve_to(svg->ctx, cpX, cpY, c1x, c1y, x, y);

                c2x    = c1x;
                c2y    = c1y;
                prev   = cubic;
            }
            break;
        case 's':
            while (try_parse_floats(&buff, buff_end, 4, &c1x, &c1y, &x, &y)) {
                vkvg_get_current_point(svg->ctx, &cpX, &cpY);
                if (prev == cubic)
                    vkvg_rel_curve_to(svg->ctx, cpX - c2x, cpY - c2y, c1x, c1y, x, y);
                else
                    vkvg_rel_curve_to(svg->ctx, 0, 0, c1x, c1y, x, y);

                c2x    = cpX + c1x;
                c2y    = cpY + c1y;
                prev   = cubic;
            }
            break;
        case 'A': // rx ry x-axis-rotation large-arc-flag sweep-flag x y
            while (try_parse_floats(&buff, buff_end, 3, &rx, &ry, &rotx)) {
                if (!try_parse_floats(&buff, buff_end, 2, &large, &sweep) || !try_parse_floats(&buff, buff_end, 2, &x, &y))
                    return;
                rotx = rotx * M_PIF / 180.0f;

                vkvg_elliptic_arc_to(svg->ctx, x, y, large, sweep, rx, ry, rotx);
            }
            break;
        case 'a': // rx ry x-axis-rotation large-arc-flag sweep-flag x y
            while (try_parse_floats(&buff, buff_end, 3, &rx, &ry, &rotx)) {
                if (!try_parse_floats(&buff, buff_end, 2, &large, &sweep) || !try_parse_floats(&buff, buff_end, 2, &x, &y))
                    return;
                rotx = degToRad(rotx);

                vkvg_rel_elliptic_arc_to(svg->ctx, x, y, large, sweep, rx, ry, rotx);
            }
            break;
        case 'z':
        case 'Z':
            vkvg_close_path(svg->ctx);
            vkvg_move_to(svg->ctx, subpathX, subpathY);
            break;
        default:
            LOG("error parsing path: unexpected char: %c\n", c);
            return;
        }
    }
}
*/

static inline float _normalized_diagonal(float w, float h) { return sqrtf(powf(w, 2) + powf(h, 2)) / sqrtf(2); }
void draw(svg_context *svg, SvgPresentationAttributes *const attribs) {
    LOG("SVG Draw: %.*s\n", (int)svg->elt_len, svg->elt);
    if (attribs->fill_type) {
        vkvg_set_opacity(svg->ctx, attribs->opacity * attribs->fill_opacity);
        if (attribs->fill_type == svg_paint_type_pattern)
            set_pattern(svg, attribs->fill);
        else
            vkvg_set_source_color(svg->ctx, attribs->fill);
        if (attribs->stroke_type) {
            vkvg_fill_preserve(svg->ctx);
        } else {
            vkvg_fill(svg->ctx);
            return;
        }
    }
    if (attribs->stroke_type) {
        vkvg_set_opacity(svg->ctx, attribs->opacity * attribs->stroke_opacity);
        if (attribs->stroke_type == svg_paint_type_pattern)
            set_pattern(svg, attribs->stroke);
        else
            vkvg_set_source_color(svg->ctx, attribs->stroke);
        vkvg_set_line_width(svg->ctx, attribs->stroke_width.number);
        //vkvg_set_line_cap(svg->ctx, attribs->);
        vkvg_stroke(svg->ctx);
    }
}
void  _process_element(svg_context *svg, SvgPresentationAttributes *const attribs, void *elt, bool use) {
    if (!(svg->inDefs || svg->skipDraw)) {
        LOG("process element: %.*s \n", (int)svg->elt_len, svg->elt);
        switch (_get_element_type(elt)) {
        case svg_element_type_rect: {
            CASTELT(r, rect, elt);
            if (r->w.number && r->h.number && (attribs->fill_type || attribs->stroke_type)) {
                float   x   = _get_pixel_coord(svg->viewBox.w, &r->x), y = _get_pixel_coord(svg->viewBox.h, &r->y),
                        w   = _get_pixel_coord(svg->viewBox.w, &r->w), h = _get_pixel_coord(svg->viewBox.h, &r->h),
                        rx  = _get_pixel_coord(svg->viewBox.w, &r->rx), ry = _get_pixel_coord(svg->viewBox.h, &r->ry);

                if (rx > w / 2.0f)
                    rx = w / 2.0f;
                if (ry > h / 2.0f)
                    ry = h / 2.0f;
                if (rx > 0 || ry > 0) {
                    if (rx == 0)
                        rx = ry;
                    else if (ry == 0)
                        ry = rx;
                    vkvg_rounded_rectangle2(svg->ctx, x, y, w, h, rx, ry);
                } else
                    vkvg_rectangle(svg->ctx, x, y, w, h);
                draw(svg, attribs);
            }
        } break;
        case svg_element_type_circle: {
            CASTELT(c, circle, elt);
            if (c->r.number > 0 && (attribs->fill_type || attribs->stroke_type)) {
                float cx = _get_pixel_coord(svg->viewBox.w, &c->cx), cy = _get_pixel_coord(svg->viewBox.h, &c->cy),
                    r = _get_pixel_coord(_normalized_diagonal(svg->viewBox.w, svg->viewBox.h), &c->r);

                vkvg_arc(svg->ctx, cx, cy, r, 0, M_PIF * 2);
                draw(svg, attribs);
            }
        } break;
        case svg_element_type_line: {
            CASTELT(l, line, elt);
            if (attribs->stroke_type) {
                float x1 = _get_pixel_coord(svg->viewBox.w, &l->x1), y1 = _get_pixel_coord(svg->viewBox.h, &l->y1),
                    x2 = _get_pixel_coord(svg->viewBox.w, &l->x2), y2 = _get_pixel_coord(svg->viewBox.h, &l->y2);

                vkvg_move_to(svg->ctx, x1, y1);
                vkvg_line_to(svg->ctx, x2, y2);

                attribs->fill_type = false;
                draw(svg, attribs);
            }
        } break;
        case svg_element_type_ellipse: {
            CASTELT(e, ellipse, elt);
            if (e->rx.number && e->ry.number && (attribs->fill_type || attribs->stroke_type)) {
                float cx = _get_pixel_coord(svg->viewBox.w, &e->cx), cy = _get_pixel_coord(svg->viewBox.h, &e->cy),
                    rx = _get_pixel_coord(svg->viewBox.w, &e->rx), ry = _get_pixel_coord(svg->viewBox.h, &e->ry);

                vkvg_ellipse(svg->ctx, rx, ry, cx, cy, 0);
                draw(svg, attribs);
            }
        } break;
        case svg_element_type_path: {
            CASTELT(p, path, elt);
            _parse_path_d_attribute(svg, p->d, p->d + p->d_len);
            draw(svg, attribs);
        } break;
        case svg_element_type_polygon: {
            CASTELT(p, polygon, elt);
            parse_point_list(svg, p->points, p->points + p->points_len);
            if (p->closed)
                vkvg_close_path(svg->ctx);
            draw(svg, attribs);
        } break;
        default:
            LOG("Unprocessed element type: %d\n", _get_element_type(elt));
            return;
        }
    }
    if (!use)
        _store_or_throw(svg, elt);
    svg->currentIdHash = 0;
}
void _process_use(svg_context *svg, SvgPresentationAttributes *const attribs) {
    if (!svg->currentXlinkHref) {
        LOG("no xlink:href defined for use element\n");
        return;
    }
    void *elt;
    if (!try_find_by_id(svg, svg->currentXlinkHref, &elt)) {
        LOG("xlink:href not resolved %.*s\n", (int)svg->value_len, svg->value);
        return;
    }
    _process_element(svg, attribs, elt, true);
}
void apply_transform(svg_context *svg) {
    vkvg_matrix_t current;
    vkvg_get_matrix(svg->ctx, &current);
    if (try_parse_transform(svg, &current))
        vkvg_set_matrix(svg->ctx, &current);
}
int parse_children(SVG_COMMON_SIG);

int try_parse_attibute(svg_context *const svg) {
    const uint8_t *buff = svg->style ? svg->style : svg->buff_ptr;
    const uint8_t *const buff_end = svg->style ? svg->style_end : svg->buff_end;
    uint8_t val_delim;
    while (buff < buff_end) {
        if (*buff > 64) {
            svg->att = buff;
            while (++buff < buff_end) {
                if (*buff < 65) {
                    if (*buff == '-')
                        continue;
                    if (*buff == ':') {
                        if (svg->style) {
                            svg->att_len = buff - svg->att;
                            svg->value = buff + 1;
                            while (++buff < buff_end) {
                                if (*buff == ';')
                                    break;
                            }
                            svg->value_len = buff - svg->value;
                            svg->style = buff + 1;
                            return 1;
                        } else {
                            svg->ns = svg->att;
                            svg->ns_len = buff - 1 - svg->ns;
                            svg->att = buff + 1;
                            continue;
                        }
                    }
                    svg->att_len = buff - svg->att;

                    //expecting '=' or white space
                    while (*buff != '=') {
                        if (++buff == buff_end) {
                            perror("malformed xml, expecting '=' or white space.\n");
                            return 0;
                        }
                    }
                    while (++buff < buff_end) {
                        if (*buff == '"' || *buff == '\'') {
                            val_delim = *buff;
                            break;
                        }
                    }
                    svg->value = buff + 1;
                    while (++buff < buff_end) {
                        if (*buff == val_delim)
                            break;
                    }
                    svg->value_len = buff - svg->value;
                    svg->buff_ptr = buff + 1;
                    return 1;
                }
            }
        } else if (*buff == '/' || *buff == '>' || *buff == '<' ) {
            svg->buff_ptr = buff;
            return 0;
        }
        buff++;
    }
    if (svg->style)
        svg->style = 0;
    return 0;
}

int parse_element(SVG_COMMON_SIG) {
    const uint8_t *buff = svg->buff_ptr;
    const uint8_t *const buff_end = svg->buff_end;

    while (buff < buff_end) {
        if (*buff == '>') {
            //read element childrens
            svg->buff_ptr = ++buff;
            parse_children(svg, attribs, parentData);
            buff = svg->buff_ptr;
            return 1;
        } else if (*buff == '/') {
            //self closing tag
            if (++buff < buff_end && *buff == '>') {
                svg->buff_ptr = ++buff;
                return 1;
            }
            perror("malformed xml, expecting '>'\n");
            return 0;
        } else if (*buff == '<') {
            perror("malformed xml, unexpected '<', expecting '>'\n");
            return 0;
        } else
            buff++;
    }
    return 0;
}

int parse_children(SVG_COMMON_SIG) {
    const uint8_t *buff = svg->buff_ptr;
    const uint8_t *const buff_end = svg->buff_end;
    //store current element name
    const uint8_t *const ns = svg->ns;
    const uint8_t *const elt = svg->elt;
    const size_t ns_len = svg->ns_len;
    const size_t elt_len = svg->elt_len;
    uint8_t c = 0;

    while (buff < buff_end) {
        if (*buff == '<') {
            if (++buff == buff_end)
                return -1;
            c = *buff;
            if (c > 64) {//element name
                svg->elt = buff;
                while (++buff < buff_end) {
                    if (*buff < 65) {
                        if (*buff == ':') {
                            svg->ns = svg->elt;
                            svg->ns_len = buff - 1 - svg->ns;
                            svg->elt = buff + 1;
                            continue;
                        }
                        break;
                    }
                }
                svg->elt_len = buff - svg->elt;
                svg->buff_ptr = buff;
                elt_lut_func(svg, *attribs, parentData);
                buff = svg->buff_ptr;
                continue;
            } else if (c == '!') {
                if (++buff == buff_end)
                    return -1;
                if (*buff == '-' && ++buff < buff_end && *buff == '-') {
                    const uint8_t* commentStart = buff + 1;
                    while (++buff + 2 < buff_end) {
                        if (*buff == '-' && *(buff+1) == '-' && *(buff+2) == '>') {
                            printf("comment: ");
                            fwrite(commentStart, sizeof(uint8_t), buff - commentStart, stdout);
                            printf("\n");
                            fflush(stdout);
                            buff+=3;
                            break;
                        }
                    }
                    continue;
                } else if (buff + 7 < buff_end && !memcmp (buff, "[CDATA[", 7)) {
                    buff+=7;
                    const uint8_t* cdataStart = buff;
                    while (++buff + 2 < buff_end) {
                        if (*buff == ']' && *(buff+1) == ']' && *(buff+2) == '>') {
                            printf("cdata: ");
                            fwrite(cdataStart, sizeof(uint8_t), buff - cdataStart, stdout);
                            printf("\n");
                            fflush(stdout);
                            buff+=3;
                            break;
                        }
                    }
                    continue;
                } else if (buff + 7 < buff_end && !memcmp (buff, "DOCTYPE", 7)) {
                    buff+=7;
                    const uint8_t* doctypeStart = buff;
                    while (++buff < buff_end) {
                        if (*buff == '>') {
                            printf("doctype: ");
                            fwrite(doctypeStart, sizeof(uint8_t), buff - doctypeStart, stdout);
                            printf("\n");
                            fflush(stdout);
                            buff++;
                            break;
                        }
                    }
                    continue;
                }
            } else if (c == '/'){
                //closing tag
                if ((++buff) + elt_len < buff_end && !memcmp (buff, elt, elt_len)) {
                    svg->buff_ptr = buff + 1;
                    return 0;
                }
            } else if (c == '?') {
                continue;
            }
            printf("malformed xml\n");
            return -1;
        } else {
            buff++;
        }
    }
}

void vkvg_svg_get_dimensions(VkvgSvg svg, uint32_t *width, uint32_t *height) {
    /*if (!svg) {
        *width = *height = 0;
        return;
    }
    *width  = svg->width;
    *height = svg->height;*/
}
void vkvg_svg_render(VkvgSvg svg, VkvgContext ctx, const char *id) {
    /*if (!svg || !ctx || vkvg_status(ctx))
        return;
    _create_from_file_handle(NULL, svg->width, svg->height, svg->fileHandle, ctx, id);
    rewind(svg->fileHandle);*/
}
VkvgSvg vkvg_svg_load(const char *svgFilePath) {
    /*vkvg_svg_t *svg = (vkvg_svg_t *)calloc(1, sizeof(vkvg_svg_t));
    if (!svg)
        return NULL;
    FILE *f = fopen(svgFilePath, "r");
    if (f) {
        fseek(f, 0, SEEK_END);
        svg->size = ftell(f);
        rewind(f);

        svg->buffer = (char *)malloc(sizeof(char) * svg->size);
        fread(svg->buffer, 1, svg->size, f);

        fclose(f);

        svg->fileHandle = fmemopen((void *)svg->buffer, svg->size, "r");
    }
    _query_dimensions(svg);
    return svg;*/
}
VkvgSvg vkvg_svg_load_fragment(char *svgFragment) {
    /*if (!svgFragment)
        return NULL;
    vkvg_svg_t *svg = (vkvg_svg_t *)calloc(1, sizeof(vkvg_svg_t));
    if (!svg)
        return NULL;
    svg->size   = strlen(svgFragment);
    svg->buffer = (char *)malloc(sizeof(char) * svg->size);
    memcpy(svg->buffer, svgFragment, svg->size);
    svg->fileHandle = fmemopen((void *)svg->buffer, svg->size, "r");
    _query_dimensions(svg);
    return svg;*/
}

void vkvg_svg_destroy(VkvgSvg svg) {
    /*if (!svg)
        return;
    fclose(svg->fileHandle);
    free(svg->buffer);
    free(svg);*/
}

VkvgSurface vkvg_surface_create_from_svg_fragment(VkvgDevice dev, uint32_t width, uint32_t height, char *svgFragment) {
 /*   FILE       *f    = fmemopen((void *)svgFragment, strlen(svgFragment), "r");
    VkvgSurface surf = _create_from_file_handle(dev, width, height, f, NULL, NULL);
    fclose(f);
    return surf;*/
    return NULL;
}
VkvgSurface vkvg_surface_create_from_svg(VkvgDevice dev, uint32_t width, uint32_t height, const char *svgFilePath) {
    FILE *file = fopen(svgFilePath, "r");
    if (!file) {
        LOG("[SVG]Error opening file");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    if (size <= 0) {
        fclose(file);
        return EXIT_SUCCESS; // Empty file
    }
    fseek(file, 0, SEEK_SET);
    uint8_t *const file_buffer = malloc((size_t)size);
    if (!file_buffer) {
        LOG("[SVG]Memory allocation failed");
        fclose(file);
        return NULL;
    }
    size_t bytes_read = fread(file_buffer, 1, (size_t)size, file);
    fclose(file);

    svg_context svg = {file_buffer, file_buffer + bytes_read};
    vkvg_device_create_info_t dev_info = {0};
    svg.preserveAspectRatio = true;
    svg.dev     = dev;
    svg.width   = (svg_length_or_percentage) {100.0f, svg_unit_percentage};
    svg.height  = (svg_length_or_percentage) {100.0f, svg_unit_percentage};
    svg.idList  = array_create();
    svg.forced_width = width;
    svg.forced_height = height;

    SvgPresentationAttributes attribs = {
        0xff000000,
        0xff000000,
        0xff000000,
        0xff000000,
        svg_paint_type_solid,
        svg_paint_type_none,
        svg_paint_type_none,
        svg_paint_type_none,
        1.0f,
        1.0f,
        1.0f,
        1.0f, // opacities
        {1, svg_unit_px}
    };
    attribs.text_anchor = SVG_ANCHOR_START;

    parse_children (&svg, &attribs, NULL);

    free(file_buffer);

    for (uint32_t i = 0; i < svg.idList->count; i++) {
        switch (_get_element_type(svg.idList->elements[i])) {
        case svg_element_type_linear_gradient:
            vkvg_pattern_destroy((VkvgPattern)((svg_element_linear_gradient *)svg.idList->elements[i])->pattern);
            break;
        case svg_element_type_radial_gradient:
            vkvg_pattern_destroy((VkvgPattern)((svg_element_radial_gradient *)svg.idList->elements[i])->pattern);
            break;
        }
        free(svg.idList->elements[i]);
    }
    array_destroy(svg.idList);

    return svg.surf;
}

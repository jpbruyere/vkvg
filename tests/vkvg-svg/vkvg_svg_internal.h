#ifndef VKVG_SVG_INTERNAL_H
#define VKVG_SVG_INTERNAL_H

#include "svg_elt_gperf.h"
#include "array.h"
#include "vkvg.h"

#include <stdint.h>
#include <stdio.h>

typedef enum {
    svg_element_type_rect,
    svg_element_type_circle,
    svg_element_type_line,
    svg_element_type_ellipse,
    svg_element_type_polygon,
    svg_element_type_polyline,
    svg_element_type_path,
    svg_element_type_linear_gradient,
    svg_element_type_radial_gradient
} svg_element_type;

typedef enum {
    svg_paint_type_none,
    svg_paint_type_solid,
    svg_paint_type_pattern,
} svg_paint_type;

typedef enum {
    svg_unit_cm,
    svg_unit_mm,
    svg_unit_Q,
    svg_unit_in,
    svg_unit_pt,
    svg_unit_pc,
    svg_unit_px,
    svg_unit_percentage,
    svg_unit_deg,
    svg_unit_grad,
    svg_unit_rad
} svg_unit;

typedef enum { svg_gradient_unit_objectBoundingBox, svg_gradient_unit_userSpaceOnUse } svg_gradient_unit;

typedef enum { svg_text_anchor_start, svg_text_anchor_middle, svg_text_anchor_end } svg_text_anchor;

typedef struct {
    float    number;
    svg_unit units;
} svg_length_or_percentage;

typedef struct {
    uint32_t         hash;
    svg_element_type type;
    uint32_t         xlinkHref; // 0 if unused
} svg_element_header;

typedef struct {
    svg_element_header       id;
    svg_length_or_percentage x;
    svg_length_or_percentage y;
    svg_length_or_percentage w;
    svg_length_or_percentage h;
    svg_length_or_percentage rx;
    svg_length_or_percentage ry;
} svg_element_rect;

typedef struct {
    svg_element_header       id;
    svg_length_or_percentage cx;
    svg_length_or_percentage cy;
    svg_length_or_percentage r;
} svg_element_circle;

typedef struct {
    svg_element_header       id;
    svg_length_or_percentage x1;
    svg_length_or_percentage y1;
    svg_length_or_percentage x2;
    svg_length_or_percentage y2;
} svg_element_line;

typedef struct {
    svg_element_header       id;
    svg_length_or_percentage cx;
    svg_length_or_percentage cy;
    svg_length_or_percentage rx;
    svg_length_or_percentage ry;
} svg_element_ellipse;

typedef struct {
    svg_element_header id;
} svg_element_polygon;

typedef struct {
    svg_element_header id;
} svg_element_polyline;

typedef struct {
    svg_element_header id;
    char              *d;
} svg_element_path;

typedef struct {
    svg_element_header       id;
    svg_gradient_unit        gradientUnits;
    svg_length_or_percentage cx;
    svg_length_or_percentage cy;
    svg_length_or_percentage fx;
    svg_length_or_percentage fy;
    svg_length_or_percentage r;
    VkvgPattern              pattern;
} svg_element_radial_gradient;

typedef struct {
    svg_element_header       id;
    svg_gradient_unit        gradientUnits;
    svg_length_or_percentage x1;
    svg_length_or_percentage x2;
    svg_length_or_percentage y1;
    svg_length_or_percentage y2;
    VkvgPattern              pattern;
} svg_element_linear_gradient;

typedef struct {
    svg_paint_type  hasColor;
    svg_paint_type  hasStroke;
    svg_paint_type  hasFill;
    uint32_t        color;
    uint32_t        stroke;
    uint32_t        fill;
    float           opacity;
    float           fill_opacity;
    float           stroke_opacity;
    svg_text_anchor text_anchor;
} svg_attributes;

static inline uint32_t         _get_element_hash(void *elt);
static inline svg_element_type _get_element_type(void *elt);

typedef struct {
    float x;
    float y;
    float w;
    float h;
} svg_viewbox;

enum prevCmd { none, quad, cubic };

typedef struct {
    const uint8_t *buff_ptr;
    const uint8_t *buff_end;
    const uint8_t *ns;
    const uint8_t *elt;
    const uint8_t *att;
    const uint8_t *value;
    size_t         ns_len;
    size_t         elt_len;
    size_t         att_len;
    size_t         value_len;
    VkvgDevice     dev;
    VkvgContext    ctx;
    VkvgSurface    surf;
    uint32_t       width; // force surface width & height
    uint32_t       height;
    svg_viewbox    viewBox;

    //-- flags --
    uint32_t    is_in_defs          : 1;
    uint32_t    queryDimensions     : 1;
    uint32_t    preserveAspectRatio : 1;
    uint32_t    skip                : 1; // skip tag and children
    uint32_t    skipDraw            : 1;
    uint32_t    inDefs              : 1;     // in defs
    uint32_t    skipStore           : 1;  // use
    uint32_t    ownContext          : 1; // must destroy vkvg ctx on clean
    //-----------

    uint32_t    renderOnlyIdHash;
    uint32_t    currentIdHash;
    uint32_t    currentXlinkHref;
    array_t    *idList;
    /*long		currentTagStartPos;*/
} svg_context;

int parse_element_tag(svg_context * const svg);

#define PARSE_ELEMENT parse_element_tag(svg);

#define PREPROC_SVG printf("element svg correctly identify\n");
#define POSTPROC_SVG

#define PREPROC_G printf("element g correctly identify\n");
#define POSTPROC_G

#define PREPROC_PATH printf("element path correctly identify\n");
#define POSTPROC_PATH

#define SVG_ELT_LUT_FUNC_HEAD                                                                                   \
int elt_lut_func(svg_context *const svg) {                                                                      \
    const struct SvgEltKeyword *res = lookup_svg_elt_token((const char*)svg->elt, svg->elt_len);                \
    int token_id = (res != NULL) ? res->id : -1;                                                                \
    switch (token_id) {

#define SVG_ELT_LUT_FUNC_FOOTER                                                                                 \
    default:                                                                                                    \
        printf("Unidentify element: ");                                                                         \
        fwrite(svg->elt, sizeof(uint8_t), svg->elt_len, stdout);                                                \
        printf("\n");                                                                                           \
        PARSE_ELEMENT                                                                                           \
        break;                                                                                                  \
    }                                                                                                           \
}
#endif // VKVG_SVG_INTERNAL_H

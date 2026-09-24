#ifndef VKVG_SVG_INTERNAL_H
#define VKVG_SVG_INTERNAL_H

#include <stdint.h>
#include <stdio.h>

#include "svg_elt_gperf.h"
#include "svg_att_gperf.h"
#include "svg_colors_gperf.h"

#include "array.h"
#include "vkvg.h"

// 1. Define ANSI Color Codes
#define CLR_RESET   "\x1b[0m"
#define CLR_RED     "\x1b[31m"
#define CLR_GREEN   "\x1b[32m"
#define CLR_YELLOW  "\x1b[33m"
#define CLR_BLUE    "\x1b[34m"

#define DEBUG_LOG
#ifdef LOG
#undef LOG
#endif

#ifdef DEBUG_LOG
#define LOG(...) { fprintf(stdout, CLR_RESET "[SVG] " __VA_ARGS__); fflush(stdout); }
#else
#define LOG
#endif
#define LOGE(...) { fprintf(stdout, CLR_RED "[SVG] " __VA_ARGS__); fflush(stdout); }

#ifndef M_PIF
#define M_PIF ((float)3.14159265358979323846)
#endif
#define degToRad(x) (x * M_PIF / 180.0)

#define FNV_32_PRIME 0x01000193
#define FNV_32_OFFSET 0x811C9DC5

static inline uint32_t hash_svg_id(const uint8_t *str, const size_t len) {
    uint32_t hash = FNV_32_OFFSET;

    for (size_t i = 0; i < len; i++) {
        hash ^= str[i];
        hash *= FNV_32_PRIME;
    }

    return hash;
}

typedef enum {
    svg_element_type_rect,
    svg_element_type_circle,
    svg_element_type_line,
    svg_element_type_ellipse,
    svg_element_type_polygon,
    svg_element_type_path,
    svg_element_type_linear_gradient,
    svg_element_type_radial_gradient,
    svg_element_type_gradient_stop
} svg_element_type;

typedef enum {
    svg_paint_type_none,
    svg_paint_type_solid,
    svg_paint_type_pattern,
} __attribute__((packed)) svg_paint_type;
_Static_assert(sizeof(svg_paint_type) == 1, "Error: svg_paint_type is not exactly 1 byte!");

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
    const uint8_t     *points;
    size_t             points_len;
    bool               closed;
} svg_element_polygon;

typedef struct {
    svg_element_header id;
    const uint8_t     *d;
    size_t             d_len;
} svg_element_path;

typedef struct {
    svg_element_header       id;
    svg_gradient_unit        gradientUnits;
    VkvgPattern              pattern;
    vkvg_matrix_t            transform;
    bool                     hasTransform;
} svg_class_gradient;

typedef struct {
    svg_element_header       id;
    svg_gradient_unit        gradientUnits;
    VkvgPattern              pattern;
    vkvg_matrix_t            transform;
    bool                     hasTransform;
    svg_length_or_percentage cx;
    svg_length_or_percentage cy;
    svg_length_or_percentage fx;
    svg_length_or_percentage fy;
    svg_length_or_percentage r;
} svg_element_radial_gradient;

typedef struct {
    svg_element_header       id;
    svg_gradient_unit        gradientUnits;
    VkvgPattern              pattern;
    vkvg_matrix_t            transform;
    bool                     hasTransform;
    svg_length_or_percentage x1;
    svg_length_or_percentage x2;
    svg_length_or_percentage y1;
    svg_length_or_percentage y2;
} svg_element_linear_gradient;

typedef struct {
    svg_element_header      id;
    uint32_t                color;
    float                   offset;
    float                   opacity;
} svg_element_gradient_stop;


/* --- SVG 1.2 TINY ENUMERATIONS --- */
typedef enum { SVG_VECTOR_EFFECT_NONE = 0, SVG_VECTOR_EFFECT_NON_SCALING_STROKE } SvgVectorEffect;
typedef enum { SVG_FONT_STYLE_NORMAL = 0, SVG_FONT_STYLE_ITALIC, SVG_FONT_STYLE_OBLIQUE } SvgFontStyle;
typedef enum { SVG_FONT_VARIANT_NORMAL = 0, SVG_FONT_VARIANT_SMALL_CAPS } SvgFontVariant;
typedef enum {
    SVG_FONT_WEIGHT_100 = 0, SVG_FONT_WEIGHT_200, SVG_FONT_WEIGHT_300,
    SVG_FONT_WEIGHT_NORMAL, SVG_FONT_WEIGHT_500, SVG_FONT_WEIGHT_600,
    SVG_FONT_WEIGHT_BOLD, SVG_FONT_WEIGHT_800, SVG_FONT_WEIGHT_900
} SvgFontWeight;
typedef enum { SVG_FONT_STRETCH_NORMAL = 0, SVG_FONT_STRETCH_WIDER, SVG_FONT_STRETCH_NARROWER, SVG_FONT_STRETCH_CONDENSED, SVG_FONT_STRETCH_EXPANDED } SvgFontStretch;
typedef enum { SVG_DIR_LTR = 0, SVG_DIR_RTL } SvgDirection;
typedef enum { SVG_BIDI_NORMAL = 0, SVG_BIDI_EMBED, SVG_BIDI_OVERRIDE } SvgUnicodeBidi;
typedef enum { SVG_ANCHOR_START = 0, SVG_ANCHOR_MIDDLE, SVG_ANCHOR_END } SvgTextAnchor;
typedef enum { SVG_TEXT_REND_AUTO = 0, SVG_TEXT_REND_SPEED, SVG_TEXT_REND_LEGIBILITY, SVG_TEXT_REND_GEOMETRIC } SvgTextRendering;
typedef enum { SVG_DISPLAY_NONE = 0, SVG_DISPLAY_INLINE } SvgDisplay;
typedef enum { SVG_VISIBILITY_HIDDEN = 0, SVG_VISIBILITY_VISIBLE } SvgVisibility;
typedef enum { SVG_IMG_REND_AUTO = 0, SVG_IMG_REND_SPEED, SVG_IMG_REND_QUALITY } SvgImageRendering;
typedef enum { SVG_SHAPE_REND_AUTO = 0, SVG_SHAPE_REND_SPEED, SVG_SHAPE_REND_CRISP, SVG_SHAPE_REND_GEOMETRIC } SvgShapeRendering;
typedef enum { SVG_OVERFLOW_VISIBLE = 0, SVG_OVERFLOW_HIDDEN, SVG_OVERFLOW_SCROLL, SVG_OVERFLOW_AUTO } SvgOverflow;
typedef enum { SVG_BUFFER_AUTO = 0, SVG_BUFFER_STATIC, SVG_BUFFER_DYNAMIC } SvgBufferedRendering;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} SvgColor;

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
} svg_attributes_old;

typedef struct {
    uint32_t fill;
    uint32_t stroke;
    uint32_t solid_color;
    uint32_t color;

    svg_paint_type       fill_type;
    svg_paint_type       stroke_type;
    svg_paint_type       solid_type;
    svg_paint_type       color_type;

    float                fill_opacity;
    float                stroke_opacity;
    float                solid_opacity;
    float                opacity;

    svg_length_or_percentage stroke_width;
    //line cap and join to do

    uint32_t             stroke_miterlimit : 6;  // Step 0.25 (0.0 to 15.75)
    SvgFontStyle         font_style        : 2;
    SvgFontVariant       font_variant      : 1;
    SvgFontWeight        font_weight       : 4;
    SvgFontStretch       font_stretch      : 4;
    SvgDirection         direction         : 1;
    SvgUnicodeBidi       unicode_bidi      : 2;
    SvgTextAnchor        text_anchor       : 2;
    SvgTextRendering     text_rendering    : 2;
    uint32_t             font_size         : 16; // Scaled by 100
    uint16_t             font_family_id;

    SvgVectorEffect      vector_effect     : 1;
    SvgDisplay           display           : 1;
    SvgVisibility        visibility        : 1;
    SvgImageRendering    image_rendering   : 2;
    SvgShapeRendering    shape_rendering   : 2;

    SvgOverflow          overflow          : 2;
    SvgBufferedRendering buffered_rend    : 2;

    float*               stroke_dasharray;       // Ignored / NULL for basic demo
    uint16_t             dasharray_count;
    int16_t              stroke_dashoffset;
} SvgPresentationAttributes;

static inline uint32_t         _get_element_hash(void *elt) { return ((svg_element_header *)elt)->hash; }
static inline svg_element_type _get_element_type(void *elt) { return ((svg_element_header *)elt)->type; }

static inline float _get_pixel_coord(const float reference, const svg_length_or_percentage *const lop) {
    switch (lop->units) {
    case svg_unit_percentage:
        return reference * lop->number / 100.0f;
    default:
        return lop->number;
    }
}
/*static inline uint32_t apply_opacity(const uint32_t abgr, const float opacity) {
    uint32_t old_alpha = abgr >> 24;
    int new_alpha = (int)((float)old_alpha * opacity);
    return (abgr & 0x00FFFFFF) | ((uint32_t)new_alpha << 24);
}*/
#define MULTIPLY_ALPHA_ABGR(abgr, mult) \
    (((abgr) & 0x00FFFFFF) |            \
     ((uint32_t)((int)((((abgr) >> 24) * (mult)) + 0.5f) & ~255) ? 255 : (uint32_t)(((abgr) >> 24) * (mult)) << 24))

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
    const uint8_t *style;
    const uint8_t *style_end;
    size_t         ns_len;
    size_t         elt_len;
    size_t         att_len;
    size_t         value_len;

    VkvgDevice     dev;
    VkvgContext    ctx;
    VkvgSurface    surf;
    uint32_t       forced_width; // force surface width & height
    uint32_t       forced_height;

    svg_viewbox              viewBox;
    svg_length_or_percentage x;
    svg_length_or_percentage y;
    svg_length_or_percentage width;
    svg_length_or_percentage height;

    //-- flags --
    uint32_t    queryDimensions     : 1;
    uint32_t    preserveAspectRatio : 1;
    uint32_t    skip                : 1; // skip tag and children
    uint32_t    skipDraw            : 1;
    uint32_t    inDefs              : 1; // in defs
    uint32_t    skipStore           : 1; // use
    uint32_t    ownContext          : 1; // must destroy vkvg ctx on clean
    uint32_t    hasViewBox          : 1;
    //-----------

    uint32_t    renderOnlyIdHash;
    uint32_t    currentIdHash;
    uint32_t    currentXlinkHref;
    array_t    *idList;
    /*long		currentTagStartPos;*/
    SvgEltTokId curEltType;
} svg_context;

#define CREATE_CTOR_ELT(elt)                                                                                           \
svg_element_##elt *_new_##elt() {                                                                                      \
        svg_element_##elt *c = (svg_element_##elt *)calloc(1, sizeof(svg_element_##elt));                              \
        c->id.type           = svg_element_type_##elt;                                                                 \
        return c;                                                                                                      \
}

CREATE_CTOR_ELT(rect)
CREATE_CTOR_ELT(circle)
CREATE_CTOR_ELT(line)
CREATE_CTOR_ELT(ellipse)
CREATE_CTOR_ELT(path)
CREATE_CTOR_ELT(polygon)
CREATE_CTOR_ELT(gradient_stop)
#define CASTELT(var, type, data) svg_element_##type *var = (svg_element_##type *)data
#define CAST(type) ((svg_element_##type *)parentData)

svg_element_linear_gradient *_new_linear_gradient();
svg_element_radial_gradient *_new_radial_gradient();


#define SVG_COMMON_SIG svg_context *const svg, SvgPresentationAttributes *const attribs, void *parentData
#define SVG_SIG_STACK_ATTRIB svg_context *const svg, SvgPresentationAttributes attribs, void *parentData

int parse_element(SVG_COMMON_SIG);
void parse_attributes(SVG_COMMON_SIG);
int parse_children(SVG_COMMON_SIG);
int try_parse_attibute(svg_context *const svg);

bool try_parse_color(const uint8_t **buff_ptr, const uint8_t *const buff_end, svg_paint_type *isEnabled, uint32_t *colorValue);
bool try_parse_transform(svg_context *const svg, vkvg_matrix_t *const mat);
bool try_parse_length_or_percentage(svg_context *const svg, svg_length_or_percentage *const lop);
bool try_parse_viewbox(svg_context *const svg);
float parse_opacity(svg_context *const svg);
void  _process_element(svg_context * const svg, SvgPresentationAttributes *const attribs, void *elt, bool use);
void process_use(svg_context *const svg, SvgPresentationAttributes *const attribs);
void apply_transform(svg_context *svg);
void process_svg(svg_context *const svg);
static inline float parse_ratio(svg_context *const svg) {
    svg_length_or_percentage ratio;
    if (!try_parse_length_or_percentage(svg, &ratio)) {
        LOG("error parsing ratio: %.*s\n", (int)svg->value_len, svg->value);
        return 1.f;
    }
    if (ratio.units == svg_unit_percentage)
        return ratio.number / 100.0f;
    else
        return ratio.number;
}

#define PARSE_ATTRIBUTES                            \
    parse_attributes(svg, &attribs, parentData);    \
    svg->ns_len = ns_len;                           \
    svg->ns = ns;//restore svg->ns to root element after attrib parsing.
#define PARSE_ELEMENT parse_element(svg, &attribs, parentData);
#define SKIP_TAG_AND_CHILDREN                                                                               \
    PARSE_ATTRIBUTES                                                                                        \
    PARSE_ELEMENT

#define SVG_ELT_LUT_FUNC_HEAD                                                                                   \
void elt_lut_func(SVG_SIG_STACK_ATTRIB) {                                                                       \
    const uint8_t *const ns = svg->ns;                                                                          \
    const size_t ns_len = svg->ns_len;                                                                          \
    if (svg->skip) {                                                                                            \
        SKIP_TAG_AND_CHILDREN                                                                                   \
        return;                                                                                                 \
    }                                                                                                           \
    const struct SvgEltKeyword *res = lookup_svg_elt_token((const char*)svg->elt, svg->elt_len);                \
    svg->curEltType = (res != NULL) ? res->id : SVG_TOK_UNKNOWN;                                                \
    switch (svg->curEltType) {

#define SVG_ELT_LUT_FUNC_FOOTER                                                                                 \
    case SVG_TOK_UNKNOWN:                                                                                       \
    default:                                                                                                    \
        LOG("Unidentify element: %.*s\n", (int)svg->elt_len, svg->elt);                                         \
        svg->skip = true;                                                                                       \
        SKIP_TAG_AND_CHILDREN                                                                                   \
        svg->skip = false;                                                                                      \
        break;                                                                                                  \
    }                                                                                                           \
}

#define SVG_ATT_LUT_FUNC_HEAD                                                                                   \
void parse_attributes(SVG_COMMON_SIG) {                                                                         \
    while (try_parse_attibute(svg)) {                                                                           \
        if (svg->skip) {                                                                                        \
            LOG("Skipped Attribute: %.*s\n", (int)svg->att_len, svg->att);                                      \
            continue;                                                                                           \
        }                                                                                                       \
        const struct SvgAttKeyword *res = lookup_svg_att_token((const char*)svg->att, svg->att_len);            \
        SvgAttTokId token_id = (res != NULL) ? res->id : SVG_ATT_TOK_UNKNOWN;                                   \
        switch (token_id) {

#define SVG_ATT_LUT_FUNC_FOOTER                                                                                 \
        case SVG_ATT_TOK_UNKNOWN:                                                                               \
        default:                                                                                                \
            LOG("Unprocess Attribute: %.*s\n", (int)svg->att_len, svg->att);                                    \
            break;                                                                                              \
        }                                                                                                       \
    }                                                                                                           \
}

#endif // VKVG_SVG_INTERNAL_H

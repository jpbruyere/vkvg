//#include "vkvg.h"

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

#define ARRAY_IMPLEMENTATION
#include "array.h"

#define PREPROC_SVG printf("element svg correctly identify\n");
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

#define PREPROC_PATH vkvg_paint(svg->ctx);
#define POSTPROC_PATH

#define SVG_ATT_PROC_ID svg->currentIdHash = hash_svg_id(svg->value, svg->value_len);
#define SVG_ATT_PROC_WIDTH                                                  \
switch(svg->curEltType) {                                                   \
    case SVG_TOK_SVG:                                                       \
        try_parse_length_or_percentage(svg, &svg->width);                   \
        break;                                                              \
}
#define SVG_ATT_PROC_HEIGHT                                                 \
switch(svg->curEltType) {                                                   \
    case SVG_TOK_SVG:                                                       \
        try_parse_length_or_percentage(svg, &svg->height);                  \
        break;                                                              \
}
#define PROCESS_ATT_PROC_COLOR  try_parse_color(svg->value, svg->value + svg->value_len, &attribs.hasColor, &attribs.color);
#define PROCESS_ATT_PROC_STROKE try_parse_color(svg->value, svg->value + svg->value_len, &attribs.hasStroke, &attribs.stroke);
#define PROCESS_ATT_PROC_FILL   try_parse_color(svg->value, svg->value + svg->value_len, &attribs.hasFill, &attribs.fill);


#define SVG_ATT_PROC_VIEWBOX svg->hasViewBox = parse_viewbox(svg);

#include "parser_gen.h"

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

#define CASTELT(var, type, data) svg_element_##type *var = (svg_element_##type *)data

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

void print_tabs(int n) {
    while (n-- > 0) {
        putchar('\t'); // Ou ' ' si tu préfères des espaces
    }
}

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

bool try_parse_color_stream(
    const uint8_t **buff_ptr,
    const uint8_t *const buff_end,
    svg_paint_type *isEnabled,
    uint32_t *colorValue
    ) {
    const uint8_t *ptr = *buff_ptr;
    *colorValue = 0;
    *isEnabled = svg_paint_type_none;

    if (ptr >= buff_end) return false;

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
            return false;
        }

        if (is_rgba) {
            // Note: Simplification for demo assuming integer alpha channel.
            // If handling floats (e.g. 0.5), tie try_parse_float right here.
            if (!parse_rgb_channel(&ptr, buff_end, &a)) return false;
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

bool parse_viewbox(svg_context *const svg) {
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
            LOG("xlink:href svg element error  %s\n", svg->value);
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
            LOG("xlink:href svg element error, expecting gradient%s\n", svg->value);
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
            svg_element_linear_gradient *g = (svg_element_linear_gradient *)elt;

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
            svg_element_radial_gradient *g = (svg_element_radial_gradient *)elt;

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

            vkvg_pattern_edit_radial(g->pattern, cx, cy, 0, cx, cy, r);
            vkvg_set_source(svg->ctx, g->pattern);
        } break;
        }
    } else
        LOG("pattern hash not resolved: %d\n", patternHash);
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
void _parse_path_d_attribute2(svg_context *const svg, const uint8_t *const restrict buff_ptr, const uint8_t *const restrict buff_end) {
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
static inline bool is_command_char(uint8_t c) {
    // Branchless coordinate vs command selector table flag map
    static const uint8_t cmd_map[256] = {
        ['M']=1, ['m']=1, ['L']=1, ['l']=1, ['H']=1, ['h']=1,
        ['V']=1, ['v']=1, ['C']=1, ['c']=1, ['S']=1, ['s']=1,
        ['Q']=1, ['q']=1, ['T']=1, ['t']=1, ['A']=1, ['a']=1,
        ['Z']=1, ['z']=1
    };
    return cmd_map[c];
}

void _parse_path_d_attribute3(svg_context *const svg) {
    const uint8_t *buff = svg->value;
    const uint8_t *const buff_end = svg->value + svg->value_len;

           // Track state context variables for absolute vs relative path conversions
    float current_x = 0.0f;
    float current_y = 0.0f;
    float start_x   = 0.0f;
    float start_y   = 0.0f;

    // Command track state variables
    uint8_t cmd = 0;

    // Control variables for relative scaling transformations
    float x1, y1, x2, y2, x, y;

    while (buff < buff_end) {
        buff = skip_separators(buff, buff_end);
        if (buff >= buff_end) break;

        // Check if the current pointer position is a brand new command token letter
        if (is_command_char(*buff)) {
            cmd = *buff;
            buff++;
            buff = skip_separators(buff, buff_end);
        }

        // Defensive guard checking if we are somehow processing an empty path data slice
        if (cmd == 0) break;

        switch (cmd) {
        // --- MoveTo Commands ---
        case 'M':
        case 'm':
            if (!try_parse_float(&buff, buff_end, &x) ||
                !try_parse_float(&buff, buff_end, &y)) goto malformed;

            if (cmd == 'm') {
                x += current_x;
                y += current_y;
            }
            vkvg_move_to(svg->ctx, x, y);
            current_x = start_x = x;
            current_y = start_y = y;

            // SVG specification requirement: implicit trailing arguments following a MoveTo
            // token are systematically parsed as implicit LineTo ('L' / 'l') operations.
            cmd = (cmd == 'm') ? 'l' : 'L';
            break;

        // --- LineTo Commands ---
        case 'L':
        case 'l':
            if (!try_parse_float(&buff, buff_end, &x) ||
                !try_parse_float(&buff, buff_end, &y)) goto malformed;

            if (cmd == 'l') {
                x += current_x;
                y += current_y;
            }
            vkvg_line_to(svg->ctx, x, y);
            current_x = x;
            current_y = y;
            break;

                   // --- Horizontal LineTo Commands ---
        case 'H':
        case 'h':
            if (!try_parse_float(&buff, buff_end, &x)) goto malformed;
            if (cmd == 'h') x += current_x;

            vkvg_line_to(svg->ctx, x, current_y);
            current_x = x;
            break;

                   // --- Vertical LineTo Commands ---
        case 'V':
        case 'v':
            if (!try_parse_float(&buff, buff_end, &y)) goto malformed;
            if (cmd == 'v') y += current_y;

             vkvg_line_to(svg->ctx, current_x, y);
            current_y = y;
            break;

                   // --- Cubic Bézier Curve Commands ---
        case 'C':
        case 'c':
            if (!try_parse_float(&buff, buff_end, &x1) || !try_parse_float(&buff, buff_end, &y1) ||
                !try_parse_float(&buff, buff_end, &x2) || !try_parse_float(&buff, buff_end, &y2) ||
                !try_parse_float(&buff, buff_end, &x)  || !try_parse_float(&buff, buff_end, &y)) goto malformed;

            if (cmd == 'c') {
                x1 += current_x; y1 += current_y;
                x2 += current_x; y2 += current_y;
                x  += current_x; y  += current_y;
            }
            vkvg_curve_to(svg->ctx, x1, y1, x2, y2, x, y);
            current_x = x;
            current_y = y;
            break;

                   // --- ClosePath Commands ---
        case 'Z':
        case 'z':
            vkvg_close_path(svg->ctx);
            current_x = start_x;
            current_y = start_y;
            // Close path has no parameter args; force clear command state to prevent looping lockups
            cmd = 0;
            break;

        default:
            // Fallback catch block for advanced commands: S, s, Q, q, T, t, A, a
            // We can integrate these components sequentially as needed
            printf("Advanced Command '%c' skipped.\n", cmd);
            cmd = 0; // Unhandled command, escape to avoid infinite loop
            break;
        }
    }
    return;

malformed:
    perror("Malformed SVG string structure detected inside path data payload.\n");
}

static inline float _normalized_diagonal(float w, float h) { return sqrtf(powf(w, 2) + powf(h, 2)) / sqrtf(2); }
int draw(svg_context *svg, SvgPresentationAttributes *const attribs) {
    LOG("SVG Draw: %s\n", svg->elt);
    if (attribs->fill_type) {
        vkvg_set_opacity(svg->ctx, attribs->opacity * attribs->fill_opacity);
        if (attribs->fill_type == svg_paint_type_pattern)
            set_pattern(svg, attribs->fill);
        else
            vkvg_set_source_color(svg->ctx, attribs->fill);
        if (attribs->stroke_type) {
            vkvg_fill_preserve(svg->ctx);
            vkvg_set_opacity(svg->ctx, attribs->opacity * attribs->stroke_opacity);
            if (attribs->stroke_type == svg_paint_type_pattern)
                set_pattern(svg, attribs->stroke);
            else
                vkvg_set_source_color(svg->ctx, attribs->stroke);
            vkvg_stroke(svg->ctx);
        } else
            vkvg_fill(svg->ctx);
    } else if (attribs->stroke_type) {
        vkvg_set_opacity(svg->ctx, attribs->opacity * attribs->fill_opacity);
        if (attribs->stroke_type == svg_paint_type_pattern)
            set_pattern(svg, attribs->stroke);
        else
            vkvg_set_source_color(svg->ctx, attribs->stroke);
        vkvg_stroke(svg->ctx);
    }
}
void  _process_element(svg_context *svg, SvgPresentationAttributes *const attribs, void *elt, bool use) {
    if (!(svg->inDefs || svg->skipDraw)) {
        LOG("process element: %s \n", svg->elt);
        switch (_get_element_type(elt)) {
        case svg_element_type_rect: {
            CASTELT(r, rect, elt);
            if (r->w.number && r->h.number && (attribs->fill_type || attribs->stroke_type)) {
                float x = _get_pixel_coord(svg->viewBox.w, &r->x), y = _get_pixel_coord(svg->viewBox.h, &r->y),
                    w = _get_pixel_coord(svg->viewBox.w, &r->w), h = _get_pixel_coord(svg->viewBox.h, &r->h),
                    rx = _get_pixel_coord(svg->viewBox.w, &r->rx), ry = _get_pixel_coord(svg->viewBox.h, &r->ry);

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
        default:
            LOG("Unprocessed element type: %d\n", _get_element_type(elt));
            return;
        }
    }
    if (!use)
        _store_or_throw(svg, elt);
}

int parse_children(svg_context *const svg, SvgPresentationAttributes * const attribs);

int try_parse_attibute(svg_context *const svg) {
    const uint8_t *buff = svg->buff_ptr;
    const uint8_t *const buff_end = svg->buff_end;

    while (buff < buff_end) {
        if (*buff > 64) {
            svg->att = buff;
            while (++buff < buff_end) {
                if (*buff < 65) {
                    if (*buff == ':') {
                        svg->ns = svg->att;
                        svg->ns_len = buff - 1 - svg->ns;
                        svg->att = buff + 1;
                        continue;
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
                        if (*buff == '"')
                            break;
                    }
                    svg->value = buff + 1;
                    while (++buff < buff_end) {
                        if (*buff == '"')
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
    return 0;
}

int parse_element(svg_context *const svg, SvgPresentationAttributes *const attribs) {
    static uint8_t level = 0;
    const uint8_t *buff = svg->buff_ptr;
    const uint8_t *const buff_end = svg->buff_end;

    while (buff < buff_end) {
        if (*buff == '>') {
            //read element childrens
            svg->buff_ptr = ++buff;
            level++;
            print_tabs(level);
            parse_children(svg, attribs);
            buff = svg->buff_ptr;
            level--;
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

int parse_children(svg_context *const svg, SvgPresentationAttributes *const attribs) {
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
                elt_lut_func(svg, attribs);
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
                }
            } else if (c == '/'){
                //closing tag
                if ((++buff)+elt_len < buff_end && !memcmp (buff, elt, elt_len)) {
                    svg->buff_ptr = buff + 1;
                    return 0;
                }
            } else if (c == '?') {
                continue;
            }
            perror("malformed xml\n");
            return -1;
        } else {
            buff++;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename.svg>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 1;
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
        perror("Memory allocation failed");
        fclose(file);
        return EXIT_FAILURE;
    }
    size_t bytes_read = fread(file_buffer, 1, (size_t)size, file);
    fclose(file);

    // 5. Check and skip UTF-8 BOM if present

    svg_context svg = {file_buffer, file_buffer + bytes_read};
    vkvg_device_create_info_t dev_info = {0};
    svg.dev = vkvg_device_create(&dev_info);
    svg.width = (svg_length_or_percentage) {100.0f, svg_unit_percentage};
    svg.height = (svg_length_or_percentage) {100.0f, svg_unit_percentage};

    SvgPresentationAttributes attribs = {
        0xff000000,
        0xff000000,
        0xff000000,
        0xff000000,
        svg_paint_type_solid,
        svg_paint_type_none,
        1.0f,
        1.0f,
        1.0f,
        1.0f, // opacities
        svg_text_anchor_start,
    };

    parse_children (&svg, &attribs);

    if (!vkvg_surface_status(svg.surf)) {
        vkvg_surface_write_to_png(svg.surf, "test.png");
        vkvg_surface_destroy(svg.surf);
    }

    vkvg_device_destroy(svg.dev);

    free(file_buffer);
    printf("Parsing pipeline completed successfully.\n");
    return EXIT_SUCCESS;
}

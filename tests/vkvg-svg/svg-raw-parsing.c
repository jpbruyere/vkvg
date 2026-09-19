//#include "vkvg.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

#include "svg_elt_gperf.h"

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

#define ISWHITESPACE(c) (c == 0x9 || c == 0x20 || c == 0xA || c == 0xD)
#define READ if (++ptr < buff_size) c = buff[ptr]; else break

typedef struct {
    uint8_t *buff;
    size_t offset;
    size_t len;
} _stream_t;

typedef _stream_t *const restrict stream;

/**
 * @brief Blazing fast, branchless-optimized ASCII to float parser.
 * @param p Pointer to the beginning of the numeric string.
 * @param out_ptr Pointer to advance past the evaluated characters.
 * @return float The converted floating point value.
 */
inline float chunk_norris_fast_atof(const uint8_t *p, const uint8_t **out_ptr) {
    float sign = 1.0f;

    // 1. Instantly parse the sign using branchless arithmetic
    if (*p == '-') { sign = -1.0f; p++; }
    else if (*p == '+') { p++; }

    uint64_t value = 0;
    // 2. Accumulate the integer part (ASCII digits are 48-57)
    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (*p - '0');
        p++;
    }

           // 3. Process decimal point if present
    if (*p == '.') {
        p++;
        uint64_t divisor = 1;
        uint64_t fractional_part = 0;

        while (*p >= '0' && *p <= '9') {
            fractional_part = fractional_part * 10 + (*p - '0');
            divisor *= 10;
            p++;
        }
        // Merge integer and fractional components using scalar float registers
        float result = ((float)value + ((float)fractional_part / (float)divisor)) * sign;
        *out_ptr = p; // Return advanced cursor position
        return result;
    }

    *out_ptr = p;
    return (float)value * sign;
}

int parse_element_tag(const uint8_t **buff_ptr, const uint8_t *const restrict buff_end) {
    const uint8_t *buff = *buff_ptr;
    const uint8_t *tokStart = 0;
    const uint8_t *ns_start = 0;
    while (buff < buff_end) {
        if (*buff > 64) { //attribute name start
            tokStart = buff;
            while (++buff < buff_end) {
                if (*buff < 65) {
                    if (*buff == ':') {
                        ns_start = tokStart;
                        tokStart = buff + 1;
                        continue;
                    }
                    printf("\tAttribute: ");
                    fwrite(tokStart, sizeof(uint8_t), buff-tokStart,stdout);

                    //expecting '=' or white space
                    while (*buff != '=') {
                        if (++buff == buff_end) {
                            perror("malformed xml\n");
                            return -1;
                        }
                    }
                    while (++buff < buff_end) {
                        if (*buff == '"')
                            break;
                    }
                    tokStart = buff;
                    while (++buff < buff_end) {
                        if (*buff == '"')
                            break;
                    }
                    printf("=");
                    fwrite(tokStart, sizeof(uint8_t), buff-tokStart+1,stdout);
                    printf("\n");
                    fflush(stdout);
                    break;
                }
            }
        } else if (*buff == '>') {
            //read element childrens
            break;
        } else if (*buff == '/') {
            if (++buff < buff_end && *buff == '>') {
                *buff_ptr = buff;
                return 0;
            } else {
                perror("malformed xml, expecting comment\n");
                return -1;
            }
        } else if (*buff == '<') {
            perror("malformed xml, expecting comment\n");
            return -1;
        } else
            buff++;
    }
    return -1;
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
    // 2. Determine file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    if (size <= 0) {
        fclose(file);
        return EXIT_SUCCESS; // Empty file
    }
    fseek(file, 0, SEEK_SET);
    // 3. Allocate a contiguous block of memory
    uint8_t *file_buffer = malloc((size_t)size);
    if (!file_buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return EXIT_FAILURE;
    }
    // 4. Read file content entirely into memory
    size_t bytes_read = fread(file_buffer, 1, (size_t)size, file);
    fclose(file);

    // 5. Check and skip UTF-8 BOM if present
    const uint8_t *buff = file_buffer;
    const uint8_t *const buff_end = buff + bytes_read;
    const uint8_t *tokStart = 0;
    const uint8_t *ns_start = 0;
    uint8_t c = 0;
    while (buff < buff_end) {
        if (*buff == '<') {
            if (++buff == buff_end)
                return -1;
            c = *buff;
            if (c > 64) {
                tokStart = buff;
                while (++buff < buff_end) {
                    if (*buff < 65) {
                        if (*buff == ':') {
                            ns_start = tokStart;
                            tokStart = buff + 1;
                            continue;
                        }
                        const struct SvgEltKeyword *res = lookup_svg_elt_token((const char*)tokStart, buff-tokStart);
                        int token_id = (res != NULL) ? res->id : -1;
                        switch (token_id) {
                        case SVG_TOK_SVG:
                            printf("element svg correctly identify\n");
                            break;
                        case SVG_TOK_G:
                            printf("element g correctly identify\n");
                            break;
                        case SVG_TOK_DEFS:
                            printf("element defs correctly identify\n");
                            break;
                        case SVG_TOK_DESC:
                            printf("element desc correctly identify\n");
                            break;
                        case SVG_TOK_TITLE:
                            printf("element title correctly identify\n");
                            break;
                        case SVG_TOK_SYMBOL:
                            printf("element symbol correctly identify\n");
                            break;
                        case SVG_TOK_USE:
                            printf("element use correctly identify\n");
                            break;
                        case SVG_TOK_SWITCH:
                            printf("element switch correctly identify\n");
                            break;
                        case SVG_TOK_IMAGE:
                            printf("element image correctly identify\n");
                            break;
                        case SVG_TOK_STYLE:
                            printf("element style correctly identify\n");
                            break;
                        case SVG_TOK_PATH:
                            printf("element path correctly identify\n");
                            break;
                        case SVG_TOK_RECT:
                            printf("element rect correctly identify\n");
                            break;
                        case SVG_TOK_CIRCLE:
                            printf("element circle correctly identify\n");
                            break;
                        case SVG_TOK_LINE:
                            printf("element line correctly identify\n");
                            break;
                        case SVG_TOK_ELLIPSE:
                            printf("element ellipse correctly identify\n");
                            break;
                        case SVG_TOK_POLYLINE:
                            printf("element polyline correctly identify\n");
                            break;
                        case SVG_TOK_POLYGON:
                            printf("element polygon correctly identify\n");
                            break;
                        case SVG_TOK_TEXT:
                            printf("element text correctly identify\n");
                            break;
                        case SVG_TOK_TSPAN:
                            printf("element tspan correctly identify\n");
                            break;
                        case SVG_TOK_TREF:
                            printf("element tref correctly identify\n");
                            break;
                        case SVG_TOK_TEXTPATH:
                            printf("element textPath correctly identify\n");
                            break;
                        case SVG_TOK_ALTGLYPH:
                            printf("element altGlyph correctly identify\n");
                            break;
                        case SVG_TOK_GLYPHREF:
                            printf("element glyphRef correctly identify\n");
                            break;
                        case SVG_TOK_MARKER:
                            printf("element marker correctly identify\n");
                            break;
                        case SVG_TOK_COLOR_PROFILE:
                            printf("element color-profile correctly identify\n");
                            break;
                        case SVG_TOK_LINEARGRADIENT:
                            printf("element linearGradient correctly identify\n");
                            break;
                        case SVG_TOK_RADIALGRADIENT:
                            printf("element radialGradient correctly identify\n");
                            break;
                        case SVG_TOK_STOP:
                            printf("element stop correctly identify\n");
                            break;
                        case SVG_TOK_PATTERN:
                            printf("element pattern correctly identify\n");
                            break;
                        case SVG_TOK_CLIPPATH:
                            printf("element clipPath correctly identify\n");
                            break;
                        case SVG_TOK_MASK:
                            printf("element mask correctly identify\n");
                            break;
                        case SVG_TOK_FILTER:
                            printf("element filter correctly identify\n");
                            break;
                        case SVG_TOK_FEBLEND:
                            printf("element feBlend correctly identify\n");
                            break;
                        case SVG_TOK_FECOLORMATRIX:
                            printf("element feColorMatrix correctly identify\n");
                            break;
                        case SVG_TOK_FECOMPONENTTRANSFER:
                            printf("element feComponentTransfer correctly identify\n");
                            break;
                        case SVG_TOK_FECOMPOSITE:
                            printf("element feComposite correctly identify\n");
                            break;
                        case SVG_TOK_FECONVOLVEMATRIX:
                            printf("element feConvolveMatrix correctly identify\n");
                            break;
                        case SVG_TOK_FEDIFFUSELIGHTING:
                            printf("element feDiffuseLighting correctly identify\n");
                            break;
                        case SVG_TOK_FEDISPLACEMENTMAP:
                            printf("element feDisplacementMap correctly identify\n");
                            break;
                        case SVG_TOK_FEFLOOD:
                            printf("element feFlood correctly identify\n");
                            break;
                        case SVG_TOK_FEGAUSSIANBLUR:
                            printf("element feGaussianBlur correctly identify\n");
                            break;
                        case SVG_TOK_FEIMAGE:
                            printf("element feImage correctly identify\n");
                            break;
                        case SVG_TOK_FEMERGE:
                            printf("element feMerge correctly identify\n");
                            break;
                        case SVG_TOK_FEMERGENODE:
                            printf("element feMergeNode correctly identify\n");
                            break;
                        case SVG_TOK_FEMORPHOLOGY:
                            printf("element feMorphology correctly identify\n");
                            break;
                        case SVG_TOK_FEOFFSET:
                            printf("element feOffset correctly identify\n");
                            break;
                        case SVG_TOK_FESPECULARLIGHTING:
                            printf("element feSpecularLighting correctly identify\n");
                            break;
                        case SVG_TOK_FETILE:
                            printf("element feTile correctly identify\n");
                            break;
                        case SVG_TOK_FETURBULENCE:
                            printf("element feTurbulence correctly identify\n");
                            break;
                        case SVG_TOK_FEDISTANTLIGHT:
                            printf("element feDistantLight correctly identify\n");
                            break;
                        case SVG_TOK_FEPOINTLIGHT:
                            printf("element fePointLight correctly identify\n");
                            break;
                        case SVG_TOK_FESPOTLIGHT:
                            printf("element feSpotLight correctly identify\n");
                            break;
                        case SVG_TOK_FEFUNCR:
                            printf("element feFuncR correctly identify\n");
                            break;
                        case SVG_TOK_FEFUNCG:
                            printf("element feFuncG correctly identify\n");
                            break;
                        case SVG_TOK_FEFUNCB:
                            printf("element feFuncB correctly identify\n");
                            break;
                        case SVG_TOK_FEFUNCA:
                            printf("element feFuncA correctly identify\n");
                            break;
                        case SVG_TOK_CURSOR:
                            printf("element cursor correctly identify\n");
                            break;
                        case SVG_TOK_A:
                            printf("element a correctly identify\n");
                            break;
                        case SVG_TOK_VIEW:
                            printf("element view correctly identify\n");
                            break;
                        case SVG_TOK_SCRIPT:
                            printf("element script correctly identify\n");
                            break;
                        case SVG_TOK_ANIMATE:
                            printf("element animate correctly identify\n");
                            break;
                        case SVG_TOK_SET:
                            printf("element set correctly identify\n");
                            break;
                        case SVG_TOK_ANIMATEMOTION:
                            printf("element animateMotion correctly identify\n");
                            break;
                        case SVG_TOK_ANIMATECOLOR:
                            printf("element animateColor correctly identify\n");
                            break;
                        case SVG_TOK_ANIMATETRANSFORM:
                            printf("element animateTransform correctly identify\n");
                            break;
                        case SVG_TOK_MPATH:
                            printf("element mpath correctly identify\n");
                            break;
                        case SVG_TOK_FONT:
                            printf("element font correctly identify\n");
                            break;
                        case SVG_TOK_FONT_FACE:
                            printf("element font-face correctly identify\n");
                            break;
                        case SVG_TOK_GLYPH:
                            printf("element glyph correctly identify\n");
                            break;
                        case SVG_TOK_MISSING_GLYPH:
                            printf("element missing-glyph correctly identify\n");
                            break;
                        case SVG_TOK_HKERN:
                            printf("element hkern correctly identify\n");
                            break;
                        case SVG_TOK_VKERN:
                            printf("element vkern correctly identify\n");
                            break;
                        case SVG_TOK_FONT_FACE_URI:
                            printf("element font-face-uri correctly identify\n");
                            break;
                        case SVG_TOK_FONT_FACE_FORMAT:
                            printf("element font-face-format correctly identify\n");
                            break;
                        case SVG_TOK_FONT_FACE_NAME:
                            printf("element font-face-name correctly identify\n");
                            break;
                        case SVG_TOK_FOREIGNOBJECT:
                            printf("element foreignObject correctly identify\n");
                            break;
                        default:
                            printf("Unidentify element: ");
                            fwrite(tokStart, sizeof(uint8_t), buff-tokStart, stdout);
                            printf("\n");
                            break;
                        }
                        parse_element_tag(&buff, buff_end);
                        break;
                    }
                }
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
                } else {
                    perror("malformed xml\n");
                    return -1;
                }
            } else if (c == '?') {

            } else if (c == '/'){
                //closing tag
            } else {
                perror("malformed xml\n");
                return -1;
            }
        } else {
            buff++;
        }
    }

    free(file_buffer);
    printf("Parsing pipeline completed successfully.\n");
    return EXIT_SUCCESS;
}

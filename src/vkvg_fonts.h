// Copyright (c) 2018-2026 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
#ifndef VKVG_FONTS_H
#define VKVG_FONTS_H

// disable warning on iostream functions on windows
#define _CRT_SECURE_NO_WARNINGS

#include "vkvg_internal.h"
#include "vkh_buffer.h"

#ifdef VKVG_USE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H
#if defined(VKVG_LCD_FONT_FILTER) && defined(FT_CONFIG_OPTION_SUBPIXEL_RENDERING)
#include <freetype/ftlcdfil.h>
#endif
#define FT_CHECK_RESULT(f)                                                                                             \
    {                                                                                                                  \
        FT_Error res = (f);                                                                                            \
        if (res != 0) {                                                                                                \
            fprintf(stderr, "Fatal : FreeType error is %x in %s at line %d\n", res, __FILE__, __LINE__);               \
            assert(res == 0);                                                                                          \
        }                                                                                                              \
    }
#else
#include "stb_truetype.h"
#endif

#ifdef VKVG_USE_HARFBUZZ
#include <harfbuzz/hb.h>
#if VKVG_USE_FREETYPE
#include <harfbuzz/hb-ft.h>
#endif
#else
#endif

#ifdef VKVG_USE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

#define FONT_PAGE_SIZE          1024
#define FONT_CACHE_INIT_LAYERS  2
#define FONT_FILE_NAME_MAX_SIZE 1024
#define FONT_NAME_MAX_SIZE      128

// Bjoern Hoehrmann's UTF-8 State Magic Constants
#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

static const uint8_t utf8d[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
    8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
    0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
    0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
    0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
    1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
    1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
    1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

/**
 * @brief Decodes a single byte stream step into a Unicode code point.
 * @param state Pointer to the persistent decoder state tracker (initialize to UTF8_ACCEPT).
 * @param codep Pointer to the resulting uint32_t Unicode scalar value.
 * @param byte The raw next byte from the UTF-8 stream.
 * @return uint32_t Returns UTF8_ACCEPT when a character is fully decoded.
 */
static inline uint32_t decode_utf8_byte(uint32_t *const restrict state, uint32_t *const restrict codep, uint8_t byte) {
    uint32_t type = utf8d[byte];

    *codep = (*state != UTF8_ACCEPT) ?
                 (byte & 0x3fu) | (*codep << 6) :
                 (0xffu >> type) & (byte);

    *state = utf8d[256 + *state * 16 + type];
    return *state;
}
static uint32_t inline decode(uint32_t* state, uint32_t* codep, uint32_t byte) {
    uint32_t type = utf8d[byte];
    *codep = (*state != UTF8_ACCEPT) ?
                 (byte & 0x3fu) | (*codep << 6) :
                 (0xff >> type) & (byte);

    *state = utf8d[256 + *state*16 + type];
    return *state;
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

static inline uint32_t decode_unicode_codepoint(const uint8_t* text) {
    if (text == NULL || *text == '\0')
        return 0;

    uint32_t state = UTF8_ACCEPT;
    uint32_t codepoint = 0;
    int i = 0;
    while (text[i] != '\0') {
        decode_utf8_byte(&state, &codepoint, text[i]);

        if (state == UTF8_ACCEPT)
            return codepoint;

        if (state == UTF8_REJECT) {
            LOG(VKVG_LOG_DEBUG, "Malformed utf8 string.\n");
            return 0;
        }

        i++;
    }

           // If the loop finished but we never reached UTF8_ACCEPT, the string cut off abruptly
    if (state == UTF8_ACCEPT)
        return codepoint;
    LOG(VKVG_LOG_DEBUG, "Error: Incomplete UTF-8 character sequence.\n");
    return 0;
}

static inline int get_utf8_char_length(uint8_t first_byte) {
    return utf8_len_table[first_byte];
}

typedef struct _tex_ref_t tex_ref_t;
typedef struct _tex_ref_t* TexRef;
// texture coordinates of one character in font cache array texture.
typedef struct _char_ref {
    TexRef      texRef;    /* glyph bitmap ref */
    vec2i16     bmpDiff;    /* Difference in pixel between char bitmap top left corner and char glyph*/
    vec4        bounds;     /* normalized float bounds of character bitmap in font cache texture. */
#ifdef VKVG_USE_FREETYPE
    FT_Vector   advance;    /* horizontal or vertical advance */
#else
    vec2 advance;
#endif
} glyph_ref;

// Current location in font cache texture array for new character addition. Each font holds such structure to locate
// where to upload new chars.
typedef struct _tex_ref_t {
    uint8_t pageIdx;  /* Current page number in font cache */
    int     penY;     /* Current Y in cache for next char addition */
    int     height;   /* Height of current line pointed by this structure in pixel */
    bool    released; /* True after font destroy, may be reused for another font */
} tex_ref_t;

DIAGNOSTIC_DISABLE_UNUSED

CTOR_ARRAY(TexRef)
CTOR_ARRAY(uint64_t)
CTOR_ARRAY(VkvgFont)
ARRAY_DEL_ELT(VkvgFont)

typedef struct _tex_page_t {
    array_TexRef    lines;
    float           penY;
} tex_page_t;

CTOR_ARRAY(tex_page_t)

typedef struct _vkvg_font_buffer_t *FontBuffer;
typedef struct _vkvg_font_face_t   *FontFace;

/* Font identification structure */
typedef struct _vkvg_font_face_t{
    FontBuffer      fontBuffer;
    array_uint64_t  queryHashes;
#ifdef VKVG_USE_FREETYPE
    FT_Face         face;     /* FreeType face*/
    mtx_t           mutex;    /* Only one font size at a time may use this face */
#endif
#ifdef VKVG_USE_HARFBUZZ
    hb_font_t*      hb_font; /* HarfBuzz font instance*/
#endif
    array_VkvgFont  sizes;    /* loaded font size array */

#ifndef VKVG_USE_FREETYPE
    stbtt_fontinfo stbInfo; /* stb_truetype structure */
    int            ascent;  /* unscalled stb font metrics */
    int            descent;
    int            lineGap;
#endif
} vkvg_font_face_t;

CTOR_ARRAY(FontFace)
ARRAY_DEL_ELT(FontFace)

typedef struct _vkvg_font_buffer_t {
    uint64_t    fontPathHash;
    size_t      bufferSize;
    unsigned char*  buffer;
    array_FontFace  faces;
} vkvg_font_buffer_t;

CTOR_ARRAY(FontBuffer)
ARRAY_DEL_ELT(FontBuffer)

DIAGNOSTIC_RESTORE_UNUSED

typedef struct _vkvg_font_t {
    vkvg_status_t   status;
    atomic_int      references; // reference count

    FontFace        face;
    VkvgDevice      dev;        // to access font cache when required
#ifdef VKVG_USE_FREETYPE
    FT_F26Dot6      charSize; /* Font size in Point as fixed float 26.6 */
    FT_Size         ftSize;   /* FT size rec */
#else
    uint32_t        charSize; /* Font size in pixel */
    float           scale;    /* scale factor for the given size */
    int             ascent;   /* unscalled stb font metrics */
    int             descent;
    int             lineGap;
#endif

    glyph_ref*      charLookup;/* Lookup table of characteres in cache, if not found, upload is queued*/
    array_TexRef    texLines;  /* texture reference where to add new glyph bmp's in cache */
    int             penX;      /* Current X in cache for next char addition */
    float           height;    /* Height in pixel */
} vkvg_font_t;


// Font cache global structure, entry point for all font related operations.
typedef struct {
#ifdef VKVG_USE_FREETYPE
    FT_Library library; /* FreeType library*/
#else
#endif
#ifdef VKVG_USE_FONTCONFIG
    FcConfig*       config;     /* Font config, used to find font files by font names*/
    //FcPattern*      fcPattern;  /* patter used for all Font config queries */
    FcObjectSet*    fcReqElts;  /* Font config requested metadata elements */
#endif

    int             stagingX;   /* x pen in host buffer */
    uint8_t*        hostBuff;   /* host memory where bitmaps are first loaded */

    VkCommandBuffer cmd;          /* vulkan command buffer for font textures upload */
    vkh_buffer_t    buff;         /* stagin buffer */
    VkhImage        texture;      /* 2d array texture used by contexts to draw characteres */
    VkFormat        texFormat;    /* Format of the fonts texture array */
    uint8_t         texPixelSize; /* Size in byte of a single pixel in a font texture */
    VkFence         uploadFence;  /* Signaled when upload is finished */
    mtx_t           mutex;        /* font cache global mutex, used only if device is in thread aware mode (see:
                                     vkvg_device_set_thread_aware) */
    array_FontBuffer fontBuffers;
    array_tex_page_t texPages;
} _font_cache_t;

#define LOCK_FONTCACHE(dev)                                                                                            \
    if (dev->threadAware)                                                                                              \
        mtx_lock(&dev->fontCache->mutex);
#define UNLOCK_FONTCACHE(dev)                                                                                          \
    if (dev->threadAware)                                                                                              \
        mtx_unlock(&dev->fontCache->mutex);

// Precompute everything necessary to measure and draw one line of text, usefull to draw the same text multiple times.
typedef struct _vkvg_text_run_t {
    VkvgFont                font;       /* vkvg font structure pointer */
    VkvgDevice              dev;        /* vkvg device associated with this text run */
    vkvg_text_extents_t     extents;    /* store computed text extends */
    const char*             text;       /* utf8 char array of text*/
    unsigned int            glyph_count;/* Total glyph count */
#ifdef VKVG_USE_HARFBUZZ
    hb_buffer_t*            hbBuf;      /* HarfBuzz buffer of text */
    hb_glyph_position_t*    glyphs;     /* HarfBuzz computed glyph positions array */
#else
    vkvg_glyph_info_t*      glyphs;     /* computed glyph positions array */
#endif
} vkvg_text_run_t;


void _fonts_cache_create(VkvgDevice dev, const char *fontDirs);
void _font_cache_destroy(VkvgDevice dev);
bool _font_cache_load_font_file_in_memory(vkvg_font_face_t* fontId);
void _font_cache_show_text(VkvgContext ctx, const char* text);
void _font_cache_text_extents(VkvgContext ctx, const char* text, int length, vkvg_text_extents_t* extents);
void _font_cache_font_extents(VkvgContext ctx, vkvg_font_extents_t* extents);
void _font_cache_update_context_descset(VkvgContext ctx);

void text_run_init(VkvgContext ctx, const char* text, int length, VkvgText textRun);
void text_run_term(VkvgText textRun);
void text_run_show_text(VkvgContext ctx, VkvgText tr); // Draw text run
#endif

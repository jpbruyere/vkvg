/*
 * Copyright (c) 2018-2019 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
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
#ifndef VKVG_FONTS_H
#define VKVG_FONTS_H

// disable warning on iostream functions on windows
#define _CRT_SECURE_NO_WARNINGS

#include "vkvg_internal.h"
#include "vkh_buffer.h"

#ifdef VKVG_USE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#if defined(VKVG_LCD_FONT_FILTER) && defined(FT_CONFIG_OPTION_SUBPIXEL_RENDERING)
#include <freetype/ftlcdfil.h>
#endif
#define FT_CHECK_RESULT(f)                                                                                             \
    {                                                                                                                  \
        FT_Error res = (f);                                                                                            \
        if (res != 0) {                                                                                                \
            fprintf(stderr, "Fatal : FreeType error is %d in %s at line %d\n", res, __FILE__, __LINE__);               \
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
#define FONT_CACHE_INIT_LAYERS  1
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
    0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s7
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,2,1,1,1,1,1,1,1,1,1,1, // s8..s9
    1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // sA..sB
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1, // sC..sD
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,1,1,1,1,1,1,1,1, // sE..sF
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // sG..sH
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1, // sI..sJ
};

/**
 * @brief Decodes a single byte stream step into a Unicode code point.
 * @param state Pointer to the persistent decoder state tracker (initialize to UTF8_ACCEPT).
 * @param codep Pointer to the resulting uint32_t Unicode scalar value.
 * @param byte The raw next byte from the UTF-8 stream.
 * @return uint32_t Returns UTF8_ACCEPT when a character is fully decoded.
 */
inline uint32_t decode_utf8_byte(uint32_t *const restrict state, uint32_t *const restrict codep, uint8_t byte) {
    uint32_t type = utf8d[byte];

    *codep = (*state != UTF8_ACCEPT) ?
                 (byte & 0x3fu) | (*codep << 6) :
                 (0xffu >> type) & (byte);

    *state = utf8d[256 + *state * 16 + type];
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

inline int get_utf8_char_length(uint8_t first_byte) {
    return utf8_len_table[first_byte];
}
// texture coordinates of one character in font cache array texture.
typedef struct {
    vec4    bounds;  /* normalized float bounds of character bitmap in font cache texture. */
    vec2i16 bmpDiff; /* Difference in pixel between char bitmap top left corner and char glyph*/
    uint8_t pageIdx; /* Page index in font cache texture array */
#ifdef VKVG_USE_FREETYPE
    FT_Vector advance; /* horizontal or vertical advance */
#else
    vec2 advance;
#endif
} _char_ref;

// Current location in font cache texture array for new character addition. Each font holds such structure to locate
// where to upload new chars.
typedef struct {
    uint8_t pageIdx; /* Current page number in font cache */
    int     penX;    /* Current X in cache for next char addition */
    int     penY;    /* Current Y in cache for next char addition */
    int     height;  /* Height of current line pointed by this structure */
} _tex_ref_t;

// Loaded font structure, one per size, holds informations for glyphes upload in cache and the lookup table of
// characters.
typedef struct {
#ifdef VKVG_USE_FREETYPE
    FT_F26Dot6 charSize; /* Font size*/
    FT_Face    face;     /* FreeType face*/
#else
    uint32_t charSize; /* Font size in pixel */
    float    scale;    /* scale factor for the given size */
    int      ascent;   /* unscalled stb font metrics */
    int      descent;
    int      lineGap;
#endif

#ifdef VKVG_USE_HARFBUZZ
    hb_font_t* hb_font; /* HarfBuzz font instance*/
#endif
    _char_ref** charLookup; /* Lookup table of characteres in cache, if not found, upload is queued*/

    _tex_ref_t curLine; /* tex coord where to add new char bmp's */
} _vkvg_font_t;

/* Font identification structure */
typedef struct {
    uint64_t       names[5];    /* Resolved Input names to this font by fontConfig or custom name set by @ref vkvg_load_from_path. Maximum count is 5.*/
    uint32_t namesCount;        /* Count of resolved names by fontConfig */
    unsigned char* fontBuffer;  /* stb_truetype in memory buffer */
    long           fontBufSize; /* */
    char*          fontFile;    /* Font file full path*/
#ifndef VKVG_USE_FREETYPE
    stbtt_fontinfo stbInfo; /* stb_truetype structure */
    int            ascent;  /* unscalled stb font metrics */
    int            descent;
    int            lineGap;
#endif
    uint32_t      sizeCount; /* available font size loaded */
    _vkvg_font_t* sizes;     /* loaded font size array */
} _vkvg_font_identity_t;

// Font cache global structure, entry point for all font related operations.
typedef struct {
#ifdef VKVG_USE_FREETYPE
    FT_Library library; /* FreeType library*/
#else
#endif
#ifdef VKVG_USE_FONTCONFIG
    FcConfig* config; /* Font config, used to find font files by font names*/
#endif

    int      stagingX; /* x pen in host buffer */
    uint8_t* hostBuff; /* host memory where bitmaps are first loaded */

    VkCommandBuffer cmd;          /* vulkan command buffer for font textures upload */
    vkh_buffer_t    buff;         /* stagin buffer */
    VkhImage        texture;      /* 2d array texture used by contexts to draw characteres */
    VkFormat        texFormat;    /* Format of the fonts texture array */
    uint8_t         texPixelSize; /* Size in byte of a single pixel in a font texture */
    uint8_t texLength;   /* layer count of 2d array texture, starts with FONT_CACHE_INIT_LAYERS count and increased when
                            needed */
    int*    pensY;       /* array of current y pen positions for each texture in cache 2d array */
    VkFence uploadFence; /* Signaled when upload is finished */
    mtx_t   mutex;       /* font cache global mutex, used only if device is in thread aware mode (see:
                            vkvg_device_set_thread_aware) */

    _vkvg_font_identity_t* fonts;      /* Loaded fonts structure array */
    int32_t                fontsCount; /* Loaded fonts array count*/
} _font_cache_t;

#define LOCK_FONTCACHE(dev)                                                                                            \
    if (dev->threadAware)                                                                                              \
        mtx_lock(&dev->fontCache->mutex);
#define UNLOCK_FONTCACHE(dev)                                                                                          \
    if (dev->threadAware)                                                                                              \
        mtx_unlock(&dev->fontCache->mutex);

// Precompute everything necessary to measure and draw one line of text, usefull to draw the same text multiple times.
typedef struct _vkvg_text_run_t {
    _vkvg_font_identity_t* fontId;      /* vkvg font structure pointer */
    _vkvg_font_t*          font;        /* vkvg font structure pointer */
    VkvgDevice             dev;         /* vkvg device associated with this text run */
    vkvg_text_extents_t    extents;     /* store computed text extends */
    const char*            text;        /* utf8 char array of text*/
    unsigned int           glyph_count; /* Total glyph count */
#ifdef VKVG_USE_HARFBUZZ
    hb_buffer_t*         hbBuf;  /* HarfBuzz buffer of text */
    hb_glyph_position_t* glyphs; /* HarfBuzz computed glyph positions array */
#else
    vkvg_glyph_info_t* glyphs; /* computed glyph positions array */
#endif
} vkvg_text_run_t;

// Create font cache.
void _fonts_cache_create(VkvgDevice dev);
// Release all ressources of font cache.
void                   _font_cache_destroy(VkvgDevice dev);
_vkvg_font_identity_t* _font_cache_add_font_identity(VkvgContext ctx, const char* fontFile, const char* name);
bool                   _font_cache_load_font_file_in_memory(_vkvg_font_identity_t* fontId);
// Draw text
void _font_cache_show_text(VkvgContext ctx, const char* text);
// Get text dimmensions
void _font_cache_text_extents(VkvgContext ctx, const char* text, int length, vkvg_text_extents_t* extents);
// Get font global dimmensions
void _font_cache_font_extents(VkvgContext ctx, vkvg_font_extents_t* extents);
// Create text object that could be drawn multiple times minimizing harfbuzz and compute processing.
void _font_cache_create_text_run(VkvgContext ctx, const char* text, int length, VkvgText textRun);
// Release ressources held by a text run.
void _font_cache_destroy_text_run(VkvgText textRun);
// Draw text run
void _font_cache_show_text_run(VkvgContext ctx, VkvgText tr);
// update context font cache descriptor set
void _font_cache_update_context_descset(VkvgContext ctx);
#endif

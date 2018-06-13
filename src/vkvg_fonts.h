/*
 * Copyright (c) 2018 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
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

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <fontconfig/fontconfig.h>

#define FONT_PAGE_SIZE          2048
#define FONT_CACHE_INIT_LAYERS  2
#define FONT_FILE_NAME_MAX_SIZE 256

#include "vkvg_internal.h"
#include "vkvg.h"
#include "vkvg_buff.h"
#include "vkh.h"
//#include "vkh_image.h"


///texture coords of one char
typedef struct {
    vec4    bounds;
    vec2i16	bmpDiff;
    uint8_t pageIdx;
}_char_ref;
//chars texture atlas reference
typedef struct {
    uint8_t     pageIdx;
    int         penX;
    int         penY;
    int         height;
}_tex_ref_t;

typedef struct {
    char*       fontFile;
    FT_F26Dot6  charSize;
    hb_font_t*  hb_font;
    FT_Face     face;
    _char_ref** charLookup;

    _tex_ref_t  curLine;    //tex coord where to add new char bmp's
}_vkvg_font_t;

typedef struct {
    FT_Library		library;
    FcConfig*       config;

    int             stagingX;   //x pen in host buffer
    uint8_t*		hostBuff;	//host mem where bitmaps are first loaded

    VkCommandBuffer cmd;        //upload cmd buff
    vkvg_buff       buff;       //stagin buffer
    VkhImage		cacheTex;	//tex 2d array
    uint8_t         cacheTexLength;  //tex array length
    int*            pensY;      //y pen pos in each texture of array
    VkFence			uploadFence;

    _vkvg_font_t*	fonts;
    uint8_t			fontsCount;
}_font_cache_t;

typedef struct _vkvg_text_run_t {
    hb_buffer_t*        hbBuf;
    _vkvg_font_t*       font;
    VkvgDevice          dev;
    vkvg_text_extents_t extents;
    const char*         text;
    unsigned int         glyph_count;
    hb_glyph_position_t *glyph_pos;
} vkvg_text_run_t;

void _init_fonts_cache		(VkvgDevice dev);
void _destroy_font_cache	(VkvgDevice dev);
void _select_font_face		(VkvgContext ctx, const char* name);
void _set_font_size         (VkvgContext ctx, uint32_t size);
void _show_text				(VkvgContext ctx, const char* text);
void _text_extents           (VkvgContext ctx, const char* text, vkvg_text_extents_t *extents);
void _font_extents           (VkvgContext ctx, vkvg_font_extents_t* extents);

void _create_text_run   (VkvgContext ctx, const char* text, VkvgText textRun);
void _destroy_text_run  (VkvgText textRun);
void _show_text_run     (VkvgContext ctx, VkvgText tr);
#endif

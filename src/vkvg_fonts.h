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

 //disable warning on iostream functions on windows
#define _CRT_SECURE_NO_WARNINGS

#include <ft2build.h>
#include FT_FREETYPE_H

#if defined(VKVG_LCD_FONT_FILTER) && defined(FT_CONFIG_OPTION_SUBPIXEL_RENDERING)
#include <freetype/ftlcdfil.h>
#endif

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

#include <fontconfig/fontconfig.h>

#define FONT_PAGE_SIZE			1024
#define FONT_CACHE_INIT_LAYERS	1
#define FONT_FILE_NAME_MAX_SIZE 1024
#define FONT_NAME_MAX_SIZE		128

#include "vkvg_internal.h"
#include "vkvg.h"
#include "vkvg_buff.h"
#include "vkh.h"

#define FT_CHECK_RESULT(f)																				\
{																										\
	FT_Error res = (f);																					\
	if (res != 0)																				\
	{																									\
		fprintf(stderr,"Fatal : FreeType error is %d in %s at line %d\n", res,	__FILE__, __LINE__); \
		assert(res == 0);																		\
	}																									\
}

//texture coordinates of one character in font cache array texture.
typedef struct {
	vec4	bounds;					/* normalized float bounds of character bitmap in font cache texture. */
	vec2i16	bmpDiff;				/* Difference in pixel between char bitmap top left corner and char glyph*/
	uint8_t pageIdx;				/* Page index in font cache texture array */
}_char_ref;
// Current location in font cache texture array for new character addition. Each font holds such structure to locate
// where to upload new chars.
typedef struct {
	uint8_t		pageIdx;			/* Current page number in font cache */
	int			penX;				/* Current X in cache for next char addition */
	int			penY;				/* Current Y in cache for next char addition */
	int			height;				/* Height of current line pointed by this structure */
}_tex_ref_t;
// Loaded font structure, one per size, holds informations for glyphes upload in cache and the lookup table of characters.
typedef struct {
	FT_F26Dot6	charSize;			/* Font size*/
	FT_Face		face;				/* FreeType face*/
	hb_font_t*	hb_font;			/* HarfBuzz font instance*/
	_char_ref** charLookup;			/* Lookup table of characteres in cache, if not found, upload is queued*/

	_tex_ref_t	curLine;			/* tex coord where to add new char bmp's */
}_vkvg_font_t;
/* Font identification structure */
typedef struct {
	char**				fcNames;		/* Resolved Input names to this font by fontConfig */
	uint32_t			fcNamesCount;	/* Count of resolved names by fontConfig */
	char*				fontFile;		/* Font file full path*/
	uint32_t			sizeCount;		/* available font size loaded */
	_vkvg_font_t*		sizes;			/* loaded font size array */
}_vkvg_font_identity_t;

// Font cache global structure, entry point for all font related operations.
typedef struct {
	FT_Library		library;		/* FreeType library*/
	FcConfig*		config;			/* Font config, used to find font files by font names*/

	int				stagingX;		/* x pen in host buffer */
	uint8_t*		hostBuff;		/* host memory where bitmaps are first loaded */

	VkCommandBuffer cmd;			/* vulkan command buffer for font textures upload */
	vkvg_buff		buff;			/* stagin buffer */
	VkhImage		texture;		/* 2d array texture used by contexts to draw characteres */
	VkFormat		texFormat;		/* Format of the fonts texture array */
	uint8_t			texPixelSize;	/* Size in byte of a single pixel in a font texture */
	uint8_t			texLength;		/* layer count of 2d array texture, starts with FONT_CACHE_INIT_LAYERS count and increased when needed */
	int*			pensY;			/* array of current y pen positions for each texture in cache 2d array */
	VkFence			uploadFence;	/* Signaled when upload is finished */

	_vkvg_font_identity_t*	fonts;			/* Loaded fonts structure array */
	int32_t			fontsCount;		/* Loaded fonts array count*/
}_font_cache_t;
// Precompute everything necessary to draw one line of text, usefull to draw the same text multiple times.
typedef struct _vkvg_text_run_t {
	hb_buffer_t*		hbBuf;		/* HarfBuzz buffer of text */
	_vkvg_font_t*	font;		/* vkvg font structure pointer */
	VkvgDevice			dev;		/* vkvg device associated with this text run */
	vkvg_text_extents_t extents;	/* store computed text extends */
	const char*			text;		/* utf8 char array of text*/
	unsigned int		glyph_count;/* Total glyph count */
	hb_glyph_position_t *glyph_pos; /* HarfBuzz computed glyph positions array */
} vkvg_text_run_t;
//Create font cache.
void _init_fonts_cache		(VkvgDevice dev);
//Release all ressources of font cache.
void _destroy_font_cache	(VkvgDevice dev);
//Select current font for context from font name, create new font entry in cache if required
void _select_font_face		(VkvgContext ctx, const char* name);
void _select_font_path		(VkvgContext ctx, const char* fontFile);
//Draw text
void _show_text				(VkvgContext ctx, const char* text);
//Get text dimmensions
void _text_extents			(VkvgContext ctx, const char* text, vkvg_text_extents_t *extents);
//Get font global dimmensions
void _font_extents			(VkvgContext ctx, vkvg_font_extents_t* extents);
//Create text object that could be drawn multiple times minimizing harfbuzz and compute processing.
void _create_text_run		(VkvgContext ctx, const char* text, VkvgText textRun);
//Release ressources held by a text run.
void _destroy_text_run		(VkvgText textRun);
//Draw text run
void _show_text_run			(VkvgContext ctx, VkvgText tr);
//Trigger stagging buffer to be uploaded in font cache. Groupping upload improve performances.
void _flush_chars_to_tex	(VkvgDevice dev, _vkvg_font_t* f);
#endif

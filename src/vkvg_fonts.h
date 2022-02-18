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

#ifdef VKVG_USE_FREETYPE
	#include <ft2build.h>
	#include FT_FREETYPE_H
	#if defined(VKVG_LCD_FONT_FILTER) && defined(FT_CONFIG_OPTION_SUBPIXEL_RENDERING)
		#include <freetype/ftlcdfil.h>
	#endif
	#define FT_CHECK_RESULT(f)																				\
	{																										\
		FT_Error res = (f);																					\
		if (res != 0)																				\
		{																									\
			fprintf(stderr,"Fatal : FreeType error is %d in %s at line %d\n", res,	__FILE__, __LINE__); \
			assert(res == 0);																		\
		}																									\
	}
#else
	#include "stb_truetype.h"
#endif

#ifdef VKVG_USE_HARFBUZZ
	#include <harfbuzz/hb.h>
	#include <harfbuzz/hb-ft.h>
#else
#endif

#ifdef VKVG_USE_FONTCONFIG
	#include <fontconfig/fontconfig.h>
#endif

#define FONT_PAGE_SIZE			1024
#define FONT_CACHE_INIT_LAYERS	1
#define FONT_FILE_NAME_MAX_SIZE 1024
#define FONT_NAME_MAX_SIZE		128

#include "vkvg_internal.h"
#include "vkvg.h"
#include "vkvg_buff.h"
#include "vkh.h"
#include "vectors.h"


//texture coordinates of one character in font cache array texture.
typedef struct {
	vec4		bounds;				/* normalized float bounds of character bitmap in font cache texture. */
	vec2i16		bmpDiff;			/* Difference in pixel between char bitmap top left corner and char glyph*/
	uint8_t		pageIdx;			/* Page index in font cache texture array */
#ifdef VKVG_USE_FREETYPE
	FT_Vector	advance;			/* horizontal or vertical advance */
#else
	vec2		advance;
#endif
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
#ifdef VKVG_USE_FREETYPE
	FT_F26Dot6		charSize;		/* Font size*/
	FT_Face			face;			/* FreeType face*/
#else
	uint32_t		charSize;		/* Font size in pixel */
	float			scale;			/* scale factor for the given size */
	int				ascent;			/* unscalled stb font metrics */
	int				descent;
	int				lineGap;
#endif

#ifdef VKVG_USE_HARFBUZZ
	hb_font_t*		hb_font;		/* HarfBuzz font instance*/
#endif
	_char_ref**		charLookup;		/* Lookup table of characteres in cache, if not found, upload is queued*/

	_tex_ref_t		curLine;		/* tex coord where to add new char bmp's */
}_vkvg_font_t;

/* Font identification structure */
typedef struct {
	char**				names;		/* Resolved Input names to this font by fontConfig or custom name set by @ref vkvg_load_from_path*/
	uint32_t			namesCount;	/* Count of resolved names by fontConfig */
	char*				fontFile;	/* Font file full path*/
#ifndef VKVG_USE_FREETYPE
	unsigned char*		fontBuffer;	/* stb_truetype in memory buffer */
	stbtt_fontinfo		stbInfo;	/* stb_truetype structure */
	int					ascent;		/* unscalled stb font metrics */
	int					descent;
	int					lineGap;
#endif
	uint32_t			sizeCount;	/* available font size loaded */
	_vkvg_font_t*		sizes;		/* loaded font size array */
}_vkvg_font_identity_t;

// Font cache global structure, entry point for all font related operations.
typedef struct {
#ifdef VKVG_USE_FREETYPE
	FT_Library		library;		/* FreeType library*/
#else
#endif
#ifdef VKVG_USE_FONTCONFIG
	FcConfig*		config;			/* Font config, used to find font files by font names*/
#endif

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
	mtx_t			mutex;			/* font cache global mutex, used only if device is in thread aware mode (see: vkvg_device_set_thread_aware) */

	_vkvg_font_identity_t*	fonts;	/* Loaded fonts structure array */
	int32_t			fontsCount;		/* Loaded fonts array count*/
}_font_cache_t;

#define LOCK_FONTCACHE(dev) \
	if (dev->threadAware)\
		mtx_lock (&dev->fontCache->mutex);
#define UNLOCK_FONTCACHE(dev) \
	if (dev->threadAware)\
		mtx_unlock (&dev->fontCache->mutex);

#ifndef VKVG_USE_HARFBUZZ
typedef struct _glyph_info_t {
  int32_t  x_advance;
  int32_t  y_advance;
  int32_t  x_offset;
  int32_t  y_offset;
  uint32_t codepoint;//should be named glyphIndex, but for harfbuzz compatibility...
} vkvg_glyph_info_t;
#endif

// Precompute everything necessary to measure and draw one line of text, usefull to draw the same text multiple times.
typedef struct _vkvg_text_run_t {
	_vkvg_font_identity_t*	fontId;		/* vkvg font structure pointer */
	_vkvg_font_t*			font;		/* vkvg font structure pointer */
	VkvgDevice				dev;		/* vkvg device associated with this text run */
	vkvg_text_extents_t		extents;	/* store computed text extends */
	const char*				text;		/* utf8 char array of text*/
	unsigned int			glyph_count;/* Total glyph count */
#ifdef VKVG_USE_HARFBUZZ
	hb_buffer_t*			hbBuf;		/* HarfBuzz buffer of text */
	hb_glyph_position_t*	glyphs;		/* HarfBuzz computed glyph positions array */
#else
	vkvg_glyph_info_t*		glyphs;		/* computed glyph positions array */
#endif
} vkvg_text_run_t;

//Create font cache.
void _fonts_cache_create		(VkvgDevice dev);
//Release all ressources of font cache.
void _font_cache_destroy	(VkvgDevice dev);
void _font_cache_add_font_identity	(VkvgContext ctx, const char* fontFile, const char *name);
//Draw text
void _font_cache_show_text				(VkvgContext ctx, const char* text);
//Get text dimmensions
void _font_cache_text_extents			(VkvgContext ctx, const char* text, vkvg_text_extents_t *extents);
//Get font global dimmensions
void _font_cache_font_extents			(VkvgContext ctx, vkvg_font_extents_t* extents);
//Create text object that could be drawn multiple times minimizing harfbuzz and compute processing.
void _font_cache_create_text_run		(VkvgContext ctx, const char* text, VkvgText textRun);
//Release ressources held by a text run.
void _font_cache_destroy_text_run		(VkvgText textRun);
//Draw text run
void _font_cache_show_text_run			(VkvgContext ctx, VkvgText tr);
//update context font cache descriptor set
void _font_cache_update_context_descset (VkvgContext ctx);
#endif

/*
 * Copyright (c) 2018-2022 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
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

#include "vkvg_fonts.h"
#include "vkvg_context_internal.h"
#include "vkvg_surface_internal.h"
#include "vkvg_device_internal.h"

#include "vkh.h"

#include <locale.h>
#include <string.h>
#include <wchar.h>

#ifndef VKVG_USE_FREETYPE
	#define STB_TRUETYPE_IMPLEMENTATION
	#include "stb_truetype.h"
#endif

static int defaultFontCharSize = 12<<6;

void _init_fonts_cache (VkvgDevice dev){
	_font_cache_t* cache = (_font_cache_t*)calloc(1, sizeof(_font_cache_t));

#ifdef VKVG_USE_FONTCONFIG
	cache->config = FcInitLoadConfigAndFonts();
	if (!cache->config) {
		fprintf(stderr, "Font config initialisation failed, consider using 'FONTCONFIG_PATH' and 'FONTCONFIG_FILE' environmane\
					   variables to point to 'fonts.conf' needed for FontConfig startup");
		assert(cache->config);
	}
#endif

#ifdef VKVG_USE_FREETYPE
	FT_CHECK_RESULT(FT_Init_FreeType(&cache->library));
#endif

#if defined(VKVG_LCD_FONT_FILTER) && defined(FT_CONFIG_OPTION_SUBPIXEL_RENDERING)
	FT_CHECK_RESULT(FT_Library_SetLcdFilter (cache->library, FT_LCD_FILTER_LIGHT));
	cache->texFormat = FB_COLOR_FORMAT;
	cache->texPixelSize = 4;
#else
	cache->texFormat = VK_FORMAT_R8_UNORM;
	cache->texPixelSize = 1;
#endif

	cache->texLength = FONT_CACHE_INIT_LAYERS;
	cache->texture = vkh_tex2d_array_create ((VkhDevice)dev, cache->texFormat, FONT_PAGE_SIZE, FONT_PAGE_SIZE,
							cache->texLength ,VMA_MEMORY_USAGE_GPU_ONLY,
							VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	vkh_image_create_descriptor (cache->texture, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_COLOR_BIT,
								 VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

	cache->uploadFence = vkh_fence_create((VkhDevice)dev);

	uint32_t buffLength = FONT_PAGE_SIZE*FONT_PAGE_SIZE*cache->texPixelSize;

	vkvg_buffer_create (dev,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		buffLength, &cache->buff);

	cache->cmd = vkh_cmd_buff_create((VkhDevice)dev,dev->cmdPool,VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	//Set texture cache initial layout to shaderReadOnly to prevent error msg if cache is not fill
	VkImageSubresourceRange subres		= {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,cache->texLength};
	vkh_cmd_begin (cache->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	vkh_image_set_layout_subres(cache->cmd, cache->texture, subres,
								VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
								VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	VK_CHECK_RESULT(vkEndCommandBuffer(cache->cmd));
	_submit_cmd (dev, &cache->cmd, cache->uploadFence);

	cache->hostBuff = (uint8_t*)malloc(buffLength);
	cache->pensY = (int*)calloc(cache->texLength, sizeof(int));

	dev->fontCache = cache;
}
///increase layer count of 2d texture array used as font cache.
void _increase_font_tex_array (VkvgDevice dev){
	LOG(VKVG_LOG_INFO, "_increase_font_tex_array\n");

	_flush_all_contexes (dev);

	_font_cache_t* cache = dev->fontCache;

	vkWaitForFences		(dev->vkDev, 1, &cache->uploadFence, VK_TRUE, UINT64_MAX);
	vkResetCommandBuffer(cache->cmd, 0);
	vkResetFences		(dev->vkDev, 1, &cache->uploadFence);

	uint8_t newSize = cache->texLength + FONT_CACHE_INIT_LAYERS;
	VkhImage newImg = vkh_tex2d_array_create ((VkhDevice)dev, cache->texFormat, FONT_PAGE_SIZE, FONT_PAGE_SIZE,
											  newSize ,VMA_MEMORY_USAGE_GPU_ONLY,
											  VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	vkh_image_create_descriptor (newImg, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_COLOR_BIT,
							   VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

	VkImageSubresourceRange subresNew	= {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,newSize};
	VkImageSubresourceRange subres		= {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,cache->texLength};

	vkh_cmd_begin (cache->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	vkh_image_set_layout_subres(cache->cmd, newImg, subresNew,
								VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	vkh_image_set_layout_subres(cache->cmd, cache->texture, subres,
								VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
								VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkImageCopy cregion = { .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, cache->texLength},
							.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, cache->texLength},
							.extent = {FONT_PAGE_SIZE,FONT_PAGE_SIZE,1}};

	vkCmdCopyImage (cache->cmd, vkh_image_get_vkimage (cache->texture), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
								vkh_image_get_vkimage (newImg), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cregion);

	vkh_image_set_layout_subres(cache->cmd, newImg, subresNew,
								VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
								VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	VK_CHECK_RESULT(vkEndCommandBuffer(cache->cmd));

	_submit_cmd			(dev, &cache->cmd, cache->uploadFence);
	vkWaitForFences		(dev->vkDev, 1, &cache->uploadFence, VK_TRUE, UINT64_MAX);

	cache->pensY = (int*)realloc(cache->pensY, newSize * sizeof(int));
	void* tmp = memset (&cache->pensY[cache->texLength],0,FONT_CACHE_INIT_LAYERS*sizeof(int));

	vkh_image_destroy	(cache->texture);
	cache->texLength   = newSize;
	cache->texture	   = newImg;

	VkvgContext next = dev->lastCtx;
	while (next != NULL){
		_update_descriptor_set	(next, cache->texture, next->dsFont);
		next = next->pPrev;
	}
	_wait_idle(dev);
}
///Start a new line in font cache, increase texture layer count if needed.
void _init_next_line_in_tex_cache (VkvgDevice dev, _vkvg_font_t* f){
	_font_cache_t* cache = dev->fontCache;
	int i;
	for (i = 0; i < cache->texLength; ++i) {
		if (cache->pensY[i] + f->curLine.height >= FONT_PAGE_SIZE)
			continue;
		f->curLine.pageIdx = (unsigned char)i;
		f->curLine.penX = 0;
		f->curLine.penY = cache->pensY[i];
		cache->pensY[i] += f->curLine.height;
		return;
	}
	_flush_chars_to_tex			(dev, f);
	_increase_font_tex_array	(dev);
	_init_next_line_in_tex_cache(dev, f);
}
void _destroy_font_cache (VkvgDevice dev){
	_font_cache_t* cache = (_font_cache_t*)dev->fontCache;

	free (cache->hostBuff);

	for (int i = 0; i < cache->fontsCount; ++i) {
		_vkvg_font_identity_t* f = &cache->fonts[i];
		for (uint32_t j = 0; j < f->sizeCount; j++) {
			_vkvg_font_t* s = &f->sizes[j];
#ifdef VKVG_USE_FREETYPE
			for (int g = 0; g < s->face->num_glyphs; ++g) {
				if (s->charLookup[g]!=NULL)
					free(s->charLookup[g]);
			}
			FT_Done_Face (s->face);
#else
			for (int g = 0; g < f->stbInfo.numGlyphs; ++g) {
				if (s->charLookup[g]!=NULL)
					free(s->charLookup[g]);
			}
#endif

#ifdef VKVG_USE_HARFBUZZ
			hb_font_destroy (s->hb_font);
#endif

			free(s->charLookup);
		}
		free (f->sizes);
		free(f->fontFile);
		for (uint32_t j = 0; j < f->namesCount; j++)
			free (f->names[j]);
		if (f->namesCount > 0)
			free (f->names);
#ifndef VKVG_USE_FREETYPE
		free (f->fontBuffer);
#endif
	}

	free(cache->fonts);
	free(cache->pensY);


	vkvg_buffer_destroy (&cache->buff);
	vkh_image_destroy	(cache->texture);
	//vkFreeCommandBuffers(dev->vkDev,dev->cmdPool, 1, &cache->cmd);
	vkDestroyFence		(dev->vkDev,cache->uploadFence,NULL);
#ifdef VKVG_USE_FREETYPE
	FT_Done_FreeType(cache->library);
#endif
#ifdef VKVG_USE_FONTCONFIG
	FcConfigDestroy(cache->config);
	FcFini();
#endif

	free (dev->fontCache);

}


//flush font stagging buffer to cache texture array
void _flush_chars_to_tex (VkvgDevice dev, _vkvg_font_t* f) {
	_font_cache_t* cache = dev->fontCache;
	if (cache->stagingX == 0)//no char in stagging buff to flush
		return;

	LOG(VKVG_LOG_INFO, "_flush_chars_to_tex pen(%d, %d)\n",f->curLine.penX, f->curLine.penY);
	vkWaitForFences		(dev->vkDev,1,&cache->uploadFence,VK_TRUE,UINT64_MAX);
	vkResetCommandBuffer(cache->cmd,0);
	vkResetFences		(dev->vkDev,1,&cache->uploadFence);

	memcpy(cache->buff.allocInfo.pMappedData, cache->hostBuff, (uint64_t)f->curLine.height * FONT_PAGE_SIZE * cache->texPixelSize);

	vkh_cmd_begin (cache->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkImageSubresourceRange subres		= {VK_IMAGE_ASPECT_COLOR_BIT,0,1,f->curLine.pageIdx,1};
	vkh_image_set_layout_subres(cache->cmd, cache->texture, subres,
								VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkBufferImageCopy bufferCopyRegion = { .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT,0,f->curLine.pageIdx,1},
										   .bufferRowLength = FONT_PAGE_SIZE,
										   .bufferImageHeight = f->curLine.height,
										   .imageOffset = {f->curLine.penX,f->curLine.penY,0},
										   .imageExtent = {FONT_PAGE_SIZE-f->curLine.penX,f->curLine.height,1}};

	vkCmdCopyBufferToImage(cache->cmd, cache->buff.buffer,
						   vkh_image_get_vkimage (cache->texture), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

	vkh_image_set_layout_subres(cache->cmd, cache->texture, subres,
								VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
								VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	VK_CHECK_RESULT(vkEndCommandBuffer(cache->cmd));

	_submit_cmd (dev, &cache->cmd, cache->uploadFence);

	f->curLine.penX += cache->stagingX;
	cache->stagingX = 0;
	memset(cache->hostBuff, 0, (uint64_t)FONT_PAGE_SIZE * FONT_PAGE_SIZE * cache->texPixelSize);
}
//create a new char entry and put glyph in stagging buffer, ready for upload.
_char_ref* _prepare_char (VkvgDevice dev, VkvgText tr, uint32_t gindex){
	_vkvg_font_t* f = tr->font;
#ifdef VKVG_USE_FREETYPE
	#if defined(VKVG_LCD_FONT_FILTER) && defined(FT_CONFIG_OPTION_SUBPIXEL_RENDERING)
		FT_CHECK_RESULT(FT_Load_Glyph(f->face, gindex, FT_LOAD_TARGET_NORMAL));
		FT_CHECK_RESULT(FT_Render_Glyph(f->face->glyph, FT_RENDER_MODE_LCD));
	#else
		FT_CHECK_RESULT(FT_Load_Glyph(f->face, gindex, FT_LOAD_RENDER));
	#endif

	FT_GlyphSlot	slot	= f->face->glyph;
	FT_Bitmap		bmp		= slot->bitmap;
	uint32_t		bmpByteWidth	= bmp.width;
	uint32_t		bmpPixelWidth	= bmp.width;
	uint32_t		bmpRows	= bmp.rows;
	unsigned char*  buffer = bmp.buffer;

#if defined(VKVG_LCD_FONT_FILTER) && defined(FT_CONFIG_OPTION_SUBPIXEL_RENDERING)
	bmpPixelWidth /= 3;
#endif
#else
	stbtt_fontinfo*	pStbInfo = &tr->fontId->stbInfo;
	int c_x1, c_y1, c_x2, c_y2;
	stbtt_GetGlyphBitmapBox (pStbInfo, gindex, f->scale, f->scale, &c_x1, &c_y1, &c_x2, &c_y2);
	uint32_t bmpByteWidth	= c_x2 - c_x1;
	uint32_t bmpPixelWidth	= bmpByteWidth;
	uint32_t bmpRows		= c_y2 - c_y1;
#endif
	uint8_t* data = dev->fontCache->hostBuff;

	if (dev->fontCache->stagingX + f->curLine.penX + bmpPixelWidth > FONT_PAGE_SIZE){
		_flush_chars_to_tex (dev, f);
		_init_next_line_in_tex_cache (dev, f);
	}

	_char_ref* cr = (_char_ref*)malloc(sizeof(_char_ref));
	int penX = dev->fontCache->stagingX;

#ifdef VKVG_USE_FREETYPE
	for(uint32_t y=0; y < bmpRows; y++) {
		for(uint32_t x=0; x < bmpPixelWidth; x++) {
#if defined(VKVG_LCD_FONT_FILTER) && defined(FT_CONFIG_OPTION_SUBPIXEL_RENDERING)
			unsigned char r = buffer[y * bmp.pitch + x * 3];
			unsigned char g = buffer[y * bmp.pitch + x * 3 + 1];
			unsigned char b = buffer[y * bmp.pitch + x * 3 + 2];

			data[(penX + x + y * FONT_PAGE_SIZE) * 4] = b;
			data[(penX + x + y * FONT_PAGE_SIZE) * 4 + 1] = g;
			data[(penX + x + y * FONT_PAGE_SIZE) * 4 + 2] = r;
			data[(penX + x + y * FONT_PAGE_SIZE) * 4 + 3] = (r+g+b)/3;
#else
			data[penX + x + y * FONT_PAGE_SIZE ] = buffer[x + y * bmpPixelWidth];
#endif
		}
	}
	cr->bmpDiff.x	= (int16_t)slot->bitmap_left;
	cr->bmpDiff.y	= (int16_t)slot->bitmap_top;
	cr->advance		= slot->advance;
#else
	int advance;
	int lsb;
	stbtt_GetGlyphHMetrics(pStbInfo, gindex, &advance, &lsb);
	stbtt_MakeGlyphBitmap (pStbInfo, data + penX, bmpPixelWidth, bmpRows, FONT_PAGE_SIZE, f->scale, f->scale, gindex);
	cr->bmpDiff.x	= (int16_t)c_x1;
	cr->bmpDiff.y	= (int16_t)-c_y1;
	cr->advance		= (vec2) {(uint32_t)roundf (f->scale * advance) << 6, 0};
#endif
	vec4 uvBounds = {
		{(float)(penX + f->curLine.penX) / (float)FONT_PAGE_SIZE},
		{(float)f->curLine.penY / (float)FONT_PAGE_SIZE},
		{(float)bmpPixelWidth},
		{(float)bmpRows}};
	cr->bounds		= uvBounds;
	cr->pageIdx		= f->curLine.pageIdx;

	f->charLookup[gindex] = cr;
	dev->fontCache->stagingX += bmpPixelWidth;
	return cr;
}
void _font_add_name (_vkvg_font_identity_t* font, const char* name, int nameLength) {
	if (++font->namesCount == 1)
		font->names = (char**) malloc (sizeof(char*));
	else
		font->names = (char**) realloc (font->names, font->namesCount * sizeof(char*));

	font->names[font->namesCount-1] = (char*)calloc(nameLength, sizeof (char));
	strcpy (font->names[font->namesCount-1], name);
}
void _add_new_font_identity (VkvgContext ctx, const char* fontFile, const char* name){
	_font_cache_t*	cache = (_font_cache_t*)ctx->pSurf->dev->fontCache;
	if (++cache->fontsCount == 1)
		cache->fonts = (_vkvg_font_identity_t*) malloc (cache->fontsCount * sizeof(_vkvg_font_identity_t));
	else
		cache->fonts = (_vkvg_font_identity_t*) realloc (cache->fonts, cache->fontsCount * sizeof(_vkvg_font_identity_t));
	_vkvg_font_identity_t nf = {0};

	int fflength = strlen (fontFile) + 1;
	nf.fontFile = (char*)malloc (fflength * sizeof(char));
	strcpy (nf.fontFile, fontFile);
	_font_add_name (&nf, name, fflength);

	cache->fonts[cache->fontsCount-1] = nf;
}
//select current font for context
void _select_font_face (VkvgContext ctx, const char* name){
	if (strcmp(ctx->selectedFontName, name) == 0)
		return;
	strcpy (ctx->selectedFontName, name);
	ctx->currentFont = NULL;
	ctx->currentFontSize = NULL;
}
_vkvg_font_t* _find_or_create_font_size (VkvgContext ctx) {
	_vkvg_font_identity_t* font = ctx->currentFont;
#ifndef VKVG_USE_FREETYPE
	if (!font->stbInfo.data) {
		FILE* fontFile = fopen(font->fontFile, "rb");
		fseek(fontFile, 0, SEEK_END);
		long ffsize = ftell(fontFile); /* how long is the file ? */
		fseek(fontFile, 0, SEEK_SET); /* reset */
		font->fontBuffer = malloc(ffsize);
		fread(font->fontBuffer, ffsize, 1, fontFile);
		fclose(fontFile);
		if (!stbtt_InitFont(&font->stbInfo, font->fontBuffer, 0))
			printf("failed\n");
		stbtt_GetFontVMetrics(&font->stbInfo, &font->ascent, &font->descent, &font->lineGap);
	}
#endif
	for (uint32_t i = 0; i < font->sizeCount; ++i) {
		if (font->sizes[i].charSize == ctx->selectedCharSize)
			return &font->sizes[i];
	}
	//if not found, create a new font size structure
	if (++font->sizeCount == 1)
		font->sizes = (_vkvg_font_t*) malloc (sizeof(_vkvg_font_t));
	else
		font->sizes = (_vkvg_font_t*) realloc (font->sizes, font->sizeCount * sizeof(_vkvg_font_t));
	_vkvg_font_t newSize = {.charSize = ctx->selectedCharSize};

	VkvgDevice dev = ctx->pSurf->dev;
#ifdef VKVG_USE_FREETYPE
	_font_cache_t*	cache = (_font_cache_t*)ctx->pSurf->dev->fontCache;
	FT_CHECK_RESULT(FT_New_Face (cache->library, font->fontFile, 0, &newSize.face));
	FT_CHECK_RESULT(FT_Set_Char_Size(newSize.face, 0, newSize.charSize, dev->hdpi, dev->vdpi ));

	newSize.charLookup = (_char_ref**)calloc (newSize.face->num_glyphs, sizeof(_char_ref*));

	if (FT_IS_SCALABLE(newSize.face))
		newSize.curLine.height = FT_MulFix(newSize.face->height, newSize.face->size->metrics.y_scale) >> 6;
	else
		newSize.curLine.height = newSize.face->height >> 6;
#else
	newSize.charLookup	= (_char_ref**)calloc (font->stbInfo.numGlyphs, sizeof(_char_ref*));
	//newSize.scale		= stbtt_ScaleForPixelHeight(&font->stbInfo, newSize.charSize);
	newSize.scale		= stbtt_ScaleForMappingEmToPixels(&font->stbInfo, newSize.charSize);
	newSize.curLine.height = roundf (newSize.scale * (font->ascent - font->descent + font->lineGap));
	newSize.ascent		= roundf (newSize.scale * font->ascent);
	newSize.descent		= roundf (newSize.scale * font->descent);
	newSize.lineGap		= roundf (newSize.scale * font->lineGap);
#endif

#ifdef VKVG_USE_HARFBUZZ
	newSize.hb_font = hb_ft_font_create(newSize.face, NULL);
#endif

	_init_next_line_in_tex_cache (dev, &newSize);

	font->sizes[font->sizeCount-1] = newSize;
	return &font->sizes[font->sizeCount-1];
}

//try find font already resolved with fontconfig by font name
bool _tryFindFontByName (VkvgContext ctx, _vkvg_font_identity_t** font){
	_font_cache_t*	cache = (_font_cache_t*)ctx->pSurf->dev->fontCache;
	for (int i = 0; i < cache->fontsCount; ++i) {
		for (uint32_t j = 0; j < cache->fonts[i].namesCount; j++) {
			if (strcmp (cache->fonts[i].names[j], ctx->selectedFontName) == 0) {
				*font = &cache->fonts[i];
				return true;
			}
		}
	}
	return false;
}
bool _tryResolveFontNameWithFontConfig (VkvgContext ctx, _vkvg_font_identity_t** resolvedFont) {
	_font_cache_t*	cache = (_font_cache_t*)ctx->pSurf->dev->fontCache;
	char* fontFile = NULL;

#ifdef VKVG_USE_FONTCONFIG
	FcPattern* pat = FcNameParse((const FcChar8*)ctx->selectedFontName);
	FcConfigSubstitute(cache->config, pat, FcMatchPattern);
	FcDefaultSubstitute(pat);
	FcResult result;
	FcPattern* font = FcFontMatch(cache->config, pat, &result);
	if (font)
		FcPatternGetString(font, FC_FILE, 0, (FcChar8 **)&fontFile);
#endif
	*resolvedFont = NULL;
	if (fontFile) {
		//try find font in cache by path
		for (int i = 0; i < cache->fontsCount; ++i) {
			if (strcmp (cache->fonts[i].fontFile, fontFile) == 0) {
				int fflength = strlen(fontFile) + 1;
				_font_add_name (&cache->fonts[i], ctx->selectedFontName, fflength);
				*resolvedFont = &cache->fonts[i];
				break;
			}
		}
		if (!*resolvedFont) {
			//if not found, create a new vkvg_font
			_add_new_font_identity(ctx, fontFile, ctx->selectedFontName);
			*resolvedFont = &cache->fonts[cache->fontsCount-1];
		}
	}

#ifdef VKVG_USE_FONTCONFIG
	FcPatternDestroy(pat);
	FcPatternDestroy(font);
#endif
	return (fontFile != NULL);
}


//try to find corresponding font in cache (defined by context selectedFont) and create a new font entry if not found.
void _update_current_font (VkvgContext ctx) {
	if (ctx->currentFont == NULL){
		if (ctx->selectedFontName[0] == 0)
			_select_font_face (ctx, "sans");

		if (!_tryFindFontByName(ctx, &ctx->currentFont))
			_tryResolveFontNameWithFontConfig (ctx, &ctx->currentFont);

		ctx->currentFontSize = _find_or_create_font_size (ctx);
	}	
}

#ifdef VKVG_USE_HARFBUZZ
//Get harfBuzz buffer for provided text.
hb_buffer_t * _get_hb_buffer (_vkvg_font_t* font, const char* text) {
	hb_buffer_t *buf = hb_buffer_create();

	const char *lng	 = "fr";
	hb_script_t script = HB_SCRIPT_LATIN;
	script = hb_script_from_string (text, (int)strlen (text));

	hb_direction_t dir = hb_script_get_horizontal_direction(script);
	//dir = HB_DIRECTION_TTB;
	hb_buffer_set_direction (buf, dir);
	hb_buffer_set_script	(buf, script);
	hb_buffer_set_language	(buf, hb_language_from_string (lng, (int)strlen(lng)));
	hb_buffer_add_utf8		(buf, text, (int)strlen(text), 0, (int)strlen(text));

	hb_shape (font->hb_font, buf, NULL, 0);
	return buf;
}
#endif

//retrieve global font extends of context's current font as defined by FreeType
void _font_extents (VkvgContext ctx, vkvg_font_extents_t *extents) {
	_update_current_font (ctx);

	if (ctx->status)
		return;

	//TODO: ensure correct metrics are returned (scalled/unscalled, etc..)
	_vkvg_font_t* font = ctx->currentFontSize;
#ifdef VKVG_USE_FREETYPE
	FT_BBox* bbox = &font->face->bbox;
	FT_Size_Metrics* metrics = &font->face->size->metrics;
	extents->ascent = (float)(FT_MulFix(font->face->ascender, metrics->y_scale) >> 6);//metrics->ascender >> 6;
	extents->descent= -(float)(FT_MulFix(font->face->descender, metrics->y_scale) >> 6);//metrics->descender >> 6;
	extents->height = (float)(FT_MulFix(font->face->height, metrics->y_scale) >> 6);//metrics->height >> 6;
	extents->max_x_advance = (float)(bbox->xMax >> 6);
	extents->max_y_advance = (float)(bbox->yMax >> 6);
#else
	extents->ascent = roundf (font->scale * ctx->currentFont->ascent);
	extents->descent= -roundf (font->scale * ctx->currentFont->descent);
	extents->height = roundf (font->scale * (ctx->currentFont->ascent - ctx->currentFont->descent + ctx->currentFont->lineGap));
	extents->max_x_advance = 0;//TODO
	extents->max_y_advance = 0;
#endif
}
//compute text extends for provided string.
void _text_extents (VkvgContext ctx, const char* text, vkvg_text_extents_t *extents) {
	if (text == NULL) {
		memset(extents, 0, sizeof(vkvg_text_extents_t));
		return;
	}

	_update_current_font (ctx);

	if (ctx->status)
		return;

	vkvg_text_run_t tr = {0};
	_create_text_run (ctx, text, &tr);

	*extents = tr.extents;

	_destroy_text_run (&tr);
}
void _create_text_run (VkvgContext ctx, const char* text, VkvgText textRun) {

	_update_current_font (ctx);

	if (ctx->status)
		return;

	textRun->fontId = ctx->currentFont;
	textRun->font = ctx->currentFontSize;
	textRun->dev = ctx->pSurf->dev;

#ifdef VKVG_USE_HARFBUZZ
	textRun->hbBuf = _get_hb_buffer (ctx->currentFontSize, text);
	textRun->glyphs = hb_buffer_get_glyph_positions	 (textRun->hbBuf, &textRun->glyph_count);
#else
	int textByteLength = strlen (text);
	if (textByteLength > 0) {
		setlocale(LC_ALL, "");
		size_t wsize = mbstowcs(NULL, text, 0);
		wchar_t *tmp = (wchar_t*)malloc((wsize+1) * sizeof (wchar_t));
		textRun->glyph_count = mbstowcs (tmp, text, wsize);
		textRun->glyphs = (vkvg_glyph_info_t*)malloc(textRun->glyph_count * sizeof (vkvg_glyph_info_t));
		for (unsigned int i=0; i<textRun->glyph_count; i++) {
#ifdef VKVG_USE_FREETYPE
			uint32_t gindex = FT_Get_Char_Index( textRun->font->face, tmp[i]);
#else
			uint32_t gindex = stbtt_FindGlyphIndex(&textRun->fontId->stbInfo, tmp[i]);
#endif
			_char_ref* cr = textRun->font->charLookup[gindex];
			if (cr==NULL)
				cr = _prepare_char(textRun->dev, textRun, gindex);
			textRun->glyphs[i].codepoint = gindex;
			textRun->glyphs[i].x_advance = cr->advance.x;
			textRun->glyphs[i].y_advance = cr->advance.y;
			textRun->glyphs[i].x_offset	 = 0;
			textRun->glyphs[i].y_offset	 = 0;
			/*textRun->glyphs[i].x_offset	 = cr->bmpDiff.x;
			textRun->glyphs[i].y_offset	 = cr->bmpDiff.y;*/
		}
		free (tmp);
	}
#endif

	unsigned int string_width_in_pixels = 0;
	for (uint32_t i=0; i < textRun->glyph_count; ++i)
		string_width_in_pixels += textRun->glyphs[i].x_advance >> 6;
#ifdef VKVG_USE_FREETYPE
	FT_Size_Metrics* metrics = &ctx->currentFontSize->face->size->metrics;
	textRun->extents.height = (float)(FT_MulFix(ctx->currentFontSize->face->height, metrics->y_scale) >> 6);// (metrics->ascender + metrics->descender) >> 6;
#else
	textRun->extents.height = textRun->font->ascent - textRun->font->descent + textRun->font->lineGap;
#endif
	textRun->extents.x_advance = (float)string_width_in_pixels;
	if (textRun->glyph_count > 0) {
		textRun->extents.y_advance = (float)(textRun->glyphs[textRun->glyph_count-1].y_advance >> 6);
		textRun->extents.x_bearing = -(float)(textRun->glyphs[0].x_offset >> 6);
		textRun->extents.y_bearing = -(float)(textRun->glyphs[0].y_offset >> 6);
	}

	textRun->extents.width	= textRun->extents.x_advance;
}
void _destroy_text_run (VkvgText textRun) {
#ifdef VKVG_USE_HARFBUZZ
	hb_buffer_destroy (textRun->hbBuf);
#else
	if (textRun->glyph_count > 0)
		free (textRun->glyphs);
#endif
}
#ifdef DEBUG
void _show_texture (vkvg_context* ctx){
	Vertex vs[] = {
		{{0,0},							  0},
		{{0,FONT_PAGE_SIZE},			  0},
		{{FONT_PAGE_SIZE,0},			  0},
		{{FONT_PAGE_SIZE,FONT_PAGE_SIZE}, 0}
	};

	VKVG_IBO_INDEX_TYPE firstIdx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);
	Vertex* pVert = &ctx->vertexCache[ctx->vertCount];
	memcpy (pVert,vs,4*sizeof(Vertex));
	ctx->vertCount+=4;

	_check_vertex_cache_size(ctx);

	_add_tri_indices_for_rect(ctx, firstIdx);
}
#endif
void _show_text_run (VkvgContext ctx, VkvgText tr) {
	unsigned int glyph_count;
#ifdef VKVG_USE_HARFBUZZ
	hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos (tr->hbBuf, &glyph_count);
#else
	vkvg_glyph_info_t* glyph_info = tr->glyphs;
	glyph_count = tr->glyph_count;
#endif

	Vertex v = {{0},ctx->curColor};
	vec2 pen = {0,0};

	if (!_current_path_is_empty(ctx))
		pen = _get_current_position(ctx);

	for (uint32_t i=0; i < glyph_count; ++i) {
		_char_ref* cr = tr->font->charLookup[glyph_info[i].codepoint];

		if (cr==NULL)
			cr = _prepare_char(tr->dev, tr, glyph_info[i].codepoint);

		//continue;
		if (cr!=NULL){
			float uvWidth = cr->bounds.width / (float)FONT_PAGE_SIZE;
			float uvHeight = cr->bounds.height / (float)FONT_PAGE_SIZE;
			vec2 p0 = {pen.x + cr->bmpDiff.x + (tr->glyphs[i].x_offset >> 6),
					   pen.y - cr->bmpDiff.y + (tr->glyphs[i].y_offset >> 6)};
			v.pos = p0;

			VKVG_IBO_INDEX_TYPE firstIdx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);


			/*v.uv.x = cr->bounds.x;
			v.uv.y = cr->bounds.y;
			v.uv.z = cr->pageIdx;*/
			_add_vertex(ctx,v);

			v.pos.y += cr->bounds.height;
			//v.uv.y += uvHeight;
			_add_vertex(ctx,v);

			v.pos.x += cr->bounds.width;
			v.pos.y = p0.y;
			/*v.uv.x += uvWidth;
			v.uv.y = cr->bounds.y;*/
			_add_vertex(ctx,v);

			v.pos.y += cr->bounds.height;
			//v.uv.y += uvHeight;
			_add_vertex(ctx,v);

			_add_tri_indices_for_rect (ctx, firstIdx);
		}

		pen.x += (tr->glyphs[i].x_advance >> 6);
		pen.y -= (tr->glyphs[i].y_advance >> 6);
	}

	vkvg_move_to(ctx, pen.x, pen.y);

	_flush_chars_to_tex(tr->dev, tr->font);
}


void _show_text (VkvgContext ctx, const char* text){

	vkvg_text_run_t tr = {0};
	_create_text_run (ctx, text, &tr);

	if (ctx->status)
		return;

	_show_text_run (ctx, &tr);

	_destroy_text_run (&tr);

	//_show_texture(ctx); return;
}


/*void testfonts(){
	FT_Library		library;
	FT_Face			face;
	FT_GlyphSlot	slot;

	assert(!FT_Init_FreeType(&library));
	assert(!FT_New_Face(library, "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 0, &face));
	assert(!FT_Set_Char_Size(face, 0, ptSize, device_hdpi, device_vdpi ));

	//_build_face_tex(face);

	hb_font_t *hb_font = hb_ft_font_create(face, NULL);
	hb_buffer_t *buf = hb_buffer_create();

	const char *text = "Ленивый рыжий кот";
	const char *lng	 = "en";
	//"كسول الزنجبيل القط","懶惰的姜貓",


	hb_buffer_set_direction (buf, HB_DIRECTION_LTR);
	hb_buffer_set_script	(buf, HB_SCRIPT_LATIN);
	hb_buffer_set_language	(buf, hb_language_from_string(lng,strlen(lng)));
	hb_buffer_add_utf8		(buf, text, strlen(text), 0, strlen(text));

	hb_unicode_funcs_t * unifc = hb_unicode_funcs_get_default();
	hb_script_t sc = hb_buffer_get_script(buf);

	sc = hb_unicode_script(unifc,0x0260);

	FT_CharMap* cm = face->charmap;

	//hb_script_to_iso15924_tag()


	FT_Done_Face	( face );
	FT_Done_FreeType( library );
}*/

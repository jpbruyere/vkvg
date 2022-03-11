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

void _fonts_cache_create (VkvgDevice dev){
	_font_cache_t* cache = (_font_cache_t*)calloc(1, sizeof(_font_cache_t));

	if (dev->threadAware)
		mtx_init (&cache->mutex, mtx_plain);

#ifdef VKVG_USE_FONTCONFIG
	cache->config = FcInitLoadConfigAndFonts ();
	if (!cache->config) {
		LOG(VKVG_LOG_DEBUG, "Font config initialisation failed, consider using 'FONTCONFIG_PATH' and 'FONTCONFIG_FILE' environmane\
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
	_device_submit_cmd (dev, &cache->cmd, cache->uploadFence);

	cache->hostBuff = (uint8_t*)malloc(buffLength);
	cache->pensY = (int*)calloc(cache->texLength, sizeof(int));

	dev->fontCache = cache;
}
///increase layer count of 2d texture array used as font cache.
void _increase_font_tex_array (VkvgDevice dev){
	LOG(VKVG_LOG_INFO, "_increase_font_tex_array\n");

	_font_cache_t* cache = dev->fontCache;

	vkWaitForFences		(dev->vkDev, 1, &cache->uploadFence, VK_TRUE, UINT64_MAX);
	_device_reset_fence (dev, cache->uploadFence);

	vkResetCommandBuffer(cache->cmd, 0);

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
	vkh_image_set_layout_subres(cache->cmd, cache->texture, subres,
								VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
								VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	VK_CHECK_RESULT(vkEndCommandBuffer(cache->cmd));

	_device_submit_cmd			(dev, &cache->cmd, cache->uploadFence);
	vkWaitForFences		(dev->vkDev, 1, &cache->uploadFence, VK_TRUE, UINT64_MAX);

	cache->pensY = (int*)realloc(cache->pensY, newSize * sizeof(int));
	void* tmp = memset (&cache->pensY[cache->texLength],0,FONT_CACHE_INIT_LAYERS*sizeof(int));

	vkh_image_destroy	(cache->texture);

	cache->texLength   = newSize;
	cache->texture	   = newImg;

	_device_wait_idle(dev);
}
//flush font stagging buffer to cache texture array
//Trigger stagging buffer to be uploaded in font cache. Groupping upload improve performances.
void _flush_chars_to_tex (VkvgDevice dev, _vkvg_font_t* f) {

	_font_cache_t* cache = dev->fontCache;
	if (cache->stagingX == 0)//no char in stagging buff to flush
		return;

	LOG(VKVG_LOG_INFO, "_flush_chars_to_tex pen(%d, %d)\n",f->curLine.penX, f->curLine.penY);
	vkWaitForFences		(dev->vkDev,1,&cache->uploadFence,VK_TRUE,UINT64_MAX);
	_device_reset_fence (dev, cache->uploadFence);
	vkResetCommandBuffer(cache->cmd,0);

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

	_device_submit_cmd (dev, &cache->cmd, cache->uploadFence);

	f->curLine.penX += cache->stagingX;
	cache->stagingX = 0;
	memset(cache->hostBuff, 0, (uint64_t)FONT_PAGE_SIZE * FONT_PAGE_SIZE * cache->texPixelSize);
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
void _font_cache_destroy (VkvgDevice dev){
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
		free (f->fontBuffer);
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

	if (dev->threadAware)
		mtx_destroy (&cache->mutex);

	free (dev->fontCache);

}

void _font_cache_update_context_descset (VkvgContext ctx) {
	if (ctx->fontCacheImg)
		vkh_image_destroy (ctx->fontCacheImg);

	LOCK_FONTCACHE (ctx->dev)

	ctx->fontCacheImg = ctx->dev->fontCache->texture;
	vkh_image_reference (ctx->fontCacheImg);

	_update_descriptor_set (ctx, ctx->fontCacheImg, ctx->dsFont);

	UNLOCK_FONTCACHE (ctx->dev)
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

	FT_GlyphSlot	slot			= f->face->glyph;
	FT_Bitmap		bmp				= slot->bitmap;
	uint32_t		bmpByteWidth	= bmp.width;
	uint32_t		bmpPixelWidth	= bmp.width;
	uint32_t		bmpRows			= bmp.rows;
	unsigned char*  buffer			= bmp.buffer;

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

	font->names[font->namesCount-1] = (char*)calloc(nameLength + 1, sizeof (char));
	strcpy (font->names[font->namesCount-1], name);
}
bool _font_cache_load_font_file_in_memory (_vkvg_font_identity_t* fontId) {
	FILE* fontFile = fopen(fontId->fontFile, "rb");
	if (!fontFile)
		return false;
	fseek(fontFile, 0, SEEK_END);
	fontId->fontBufSize = ftell(fontFile); /* how long is the file ? */
	fseek(fontFile, 0, SEEK_SET); /* reset */
	fontId->fontBuffer = malloc(fontId->fontBufSize);
	fread(fontId->fontBuffer, fontId->fontBufSize, 1, fontFile);
	fclose(fontFile);
	return true;
}
_vkvg_font_identity_t* _font_cache_add_font_identity (VkvgContext ctx, const char* fontFilePath, const char* name){
	_font_cache_t*	cache = (_font_cache_t*)ctx->dev->fontCache;
	if (++cache->fontsCount == 1)
		cache->fonts = (_vkvg_font_identity_t*) malloc (cache->fontsCount * sizeof(_vkvg_font_identity_t));
	else
		cache->fonts = (_vkvg_font_identity_t*) realloc (cache->fonts, cache->fontsCount * sizeof(_vkvg_font_identity_t));
	_vkvg_font_identity_t nf = {0};

	if (fontFilePath) {
		int fflength = strlen (fontFilePath) + 1;
		nf.fontFile = (char*)malloc (fflength * sizeof(char));
		strcpy (nf.fontFile, fontFilePath);
	}

	_font_add_name (&nf, name, strlen (name));

	cache->fonts[cache->fontsCount-1] = nf;
	return &cache->fonts[cache->fontsCount-1];
}
//select current font for context
_vkvg_font_t* _find_or_create_font_size (VkvgContext ctx) {
	_vkvg_font_identity_t* font = ctx->currentFont;

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

	VkvgDevice dev = ctx->dev;
#ifdef VKVG_USE_FREETYPE
	_font_cache_t*	cache = (_font_cache_t*)ctx->dev->fontCache;
	FT_CHECK_RESULT(FT_New_Memory_Face (cache->library, font->fontBuffer, font->fontBufSize, 0, &newSize.face));
	FT_CHECK_RESULT(FT_Set_Char_Size(newSize.face, 0, newSize.charSize, dev->hdpi, dev->vdpi ));

	newSize.charLookup = (_char_ref**)calloc (newSize.face->num_glyphs, sizeof(_char_ref*));

	if (FT_IS_SCALABLE(newSize.face))
		newSize.curLine.height = FT_MulFix(newSize.face->height, newSize.face->size->metrics.y_scale) >> 6;
	else
		newSize.curLine.height = newSize.face->height >> 6;
#else
	int result = stbtt_InitFont(&font->stbInfo, font->fontBuffer, 0);
	assert(result && "stbtt_initFont failed");
	if (!result) {
		ctx->status = VKVG_STATUS_INVALID_FONT;
		return NULL;
	}
	stbtt_GetFontVMetrics(&font->stbInfo, &font->ascent, &font->descent, &font->lineGap);
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
	_font_cache_t*	cache = ctx->dev->fontCache;
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

#ifdef VKVG_USE_FONTCONFIG
bool _tryResolveFontNameWithFontConfig (VkvgContext ctx, _vkvg_font_identity_t** resolvedFont) {
	_font_cache_t*	cache = (_font_cache_t*)ctx->dev->fontCache;
	char* fontFile = NULL;

	FcPattern* pat = FcNameParse((const FcChar8*)ctx->selectedFontName);
	FcConfigSubstitute(cache->config, pat, FcMatchPattern);
	FcDefaultSubstitute(pat);
	FcResult result;
	FcPattern* font = FcFontMatch(cache->config, pat, &result);
	if (font)
		FcPatternGetString(font, FC_FILE, 0, (FcChar8 **)&fontFile);
	*resolvedFont = NULL;
	if (fontFile) {
		//try find font in cache by path
		for (int i = 0; i < cache->fontsCount; ++i) {
			if (cache->fonts[i].fontFile && strcmp (cache->fonts[i].fontFile, fontFile) == 0) {
				int fflength = strlen(fontFile) + 1;
				_font_add_name (&cache->fonts[i], ctx->selectedFontName, fflength);
				*resolvedFont = &cache->fonts[i];
				break;
			}
		}
		if (!*resolvedFont) {
			//if not found, create a new vkvg_font
			_vkvg_font_identity_t* fid = _font_cache_add_font_identity(ctx, fontFile, ctx->selectedFontName);
			_font_cache_load_font_file_in_memory (fid);
			*resolvedFont = &cache->fonts[cache->fontsCount-1];
		}
	}

	FcPatternDestroy(pat);
	FcPatternDestroy(font);

	return (fontFile != NULL);
}
#endif


//try to find corresponding font in cache (defined by context selectedFont) and create a new font entry if not found.
void _update_current_font (VkvgContext ctx) {
	if (ctx->currentFont == NULL){
		LOCK_FONTCACHE(ctx->dev)
		if (ctx->selectedFontName[0] == 0)
			_select_font_face (ctx, "sans");

		if (!_tryFindFontByName (ctx, &ctx->currentFont)) {
#ifdef VKVG_USE_FONTCONFIG
			_tryResolveFontNameWithFontConfig (ctx, &ctx->currentFont);
#else
			LOG(VKVG_LOG_ERR, "Unresolved font: %s\n", ctx->selectedFontName);
			UNLOCK_FONTCACHE(ctx->dev)
			ctx->status = VKVG_STATUS_INVALID_FONT;
			return;
#endif
		}

		ctx->currentFontSize = _find_or_create_font_size (ctx);
		UNLOCK_FONTCACHE(ctx->dev)
	}	
}

#ifdef VKVG_USE_HARFBUZZ
//Get harfBuzz buffer for provided text.
hb_buffer_t * _get_hb_buffer (_vkvg_font_t* font, const char* text, int length) {
	hb_buffer_t *buf = hb_buffer_create();

	hb_script_t script = HB_SCRIPT_LATIN;
	hb_unicode_funcs_t* ucfunc = hb_unicode_funcs_get_default ();
	wchar_t firstChar = 0;
	if (mbstowcs (&firstChar, text, 1))
		script = hb_unicode_script (ucfunc, firstChar);

	hb_direction_t dir = hb_script_get_horizontal_direction(script);
	hb_buffer_set_direction (buf, dir);
	hb_buffer_set_script	(buf, script);
	//hb_buffer_set_language	(buf, hb_language_from_string (lng, (int)strlen(lng)));
	hb_buffer_add_utf8		(buf, text, length, 0, length);

	hb_shape (font->hb_font, buf, NULL, 0);

	return buf;
}
#endif

//retrieve global font extends of context's current font as defined by FreeType
void _font_cache_font_extents (VkvgContext ctx, vkvg_font_extents_t *extents) {
	_update_current_font (ctx);

	if (ctx->status)
		return;

	//TODO: ensure correct metrics are returned (scalled/unscalled, etc..)
	_vkvg_font_t* font = ctx->currentFontSize;
#ifdef VKVG_USE_FREETYPE
	FT_BBox*			bbox	= &font->face->bbox;
	FT_Size_Metrics*	metrics = &font->face->size->metrics;

	extents->ascent			= (float)(FT_MulFix(font->face->ascender, metrics->y_scale) >> 6);//metrics->ascender >> 6;
	extents->descent		=-(float)(FT_MulFix(font->face->descender, metrics->y_scale) >> 6);//metrics->descender >> 6;
	extents->height			= (float)(FT_MulFix(font->face->height, metrics->y_scale) >> 6);//metrics->height >> 6;
	extents->max_x_advance	= (float)(bbox->xMax >> 6);
	extents->max_y_advance	= (float)(bbox->yMax >> 6);
#else
	extents->ascent			= roundf (font->scale * ctx->currentFont->ascent);
	extents->descent		=-roundf (font->scale * ctx->currentFont->descent);
	extents->height			= roundf (font->scale * (ctx->currentFont->ascent - ctx->currentFont->descent + ctx->currentFont->lineGap));
	extents->max_x_advance	= 0;//TODO
	extents->max_y_advance	= 0;
#endif
}
//compute text extends for provided string.
void _font_cache_text_extents (VkvgContext ctx, const char* text, int length, vkvg_text_extents_t *extents) {
	if (text == NULL) {
		memset(extents, 0, sizeof(vkvg_text_extents_t));
		return;
	}

	vkvg_text_run_t tr = {0};
	_font_cache_create_text_run (ctx, text, length, &tr);

	if (ctx->status)
		return;

	*extents = tr.extents;

	_font_cache_destroy_text_run (&tr);
}
//text is expected as utf8 encoded
//if length is < 0, text must be null terminated, else it contains glyph count
void _font_cache_create_text_run (VkvgContext ctx, const char* text, int length, VkvgText textRun) {

	_update_current_font (ctx);

	if (ctx->status)
		return;

	textRun->fontId = ctx->currentFont;
	textRun->font = ctx->currentFontSize;
	textRun->dev = ctx->dev;

	LOCK_FONTCACHE (ctx->dev)

#ifdef VKVG_USE_HARFBUZZ
	textRun->hbBuf = _get_hb_buffer (ctx->currentFontSize, text,  length);
	textRun->glyphs = hb_buffer_get_glyph_positions	 (textRun->hbBuf, &textRun->glyph_count);
#else

	size_t wsize;
	if (length < 0)
		wsize = mbstowcs(NULL, text, 0);
	else
		wsize = (size_t)length;
	wchar_t *tmp = (wchar_t*)malloc((wsize+1) * sizeof (wchar_t));
	textRun->glyph_count = mbstowcs (tmp, text, wsize);
	textRun->glyphs = (vkvg_glyph_info_t*)malloc(textRun->glyph_count * sizeof (vkvg_glyph_info_t));
	for (unsigned int i=0; i<textRun->glyph_count; i++) {
#ifdef VKVG_USE_FREETYPE
		uint32_t gindex = FT_Get_Char_Index (textRun->font->face, tmp[i]);
#else
		uint32_t gindex = stbtt_FindGlyphIndex (&textRun->fontId->stbInfo, tmp[i]);
#endif
		_char_ref* cr = textRun->font->charLookup[gindex];
		if (cr==NULL)
			cr = _prepare_char (textRun->dev, textRun, gindex);
		textRun->glyphs[i].codepoint = gindex;
		textRun->glyphs[i].x_advance = cr->advance.x;
		textRun->glyphs[i].y_advance = cr->advance.y;
		textRun->glyphs[i].x_offset	 = 0;
		textRun->glyphs[i].y_offset	 = 0;
		/*textRun->glyphs[i].x_offset	 = cr->bmpDiff.x;
		textRun->glyphs[i].y_offset	 = cr->bmpDiff.y;*/
	}
	free (tmp);
#endif
	
	UNLOCK_FONTCACHE (ctx->dev)

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
void _font_cache_destroy_text_run (VkvgText textRun) {
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
		{{0,0},							  0,  {0,0,0}},
		{{0,FONT_PAGE_SIZE},			  0,  {0,1,0}},
		{{FONT_PAGE_SIZE,0},			  0,  {1,0,0}},
		{{FONT_PAGE_SIZE,FONT_PAGE_SIZE}, 0,  {1,1,0}}
	};

	VKVG_IBO_INDEX_TYPE firstIdx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);
	Vertex* pVert = &ctx->vertexCache[ctx->vertCount];
	memcpy (pVert,vs,4*sizeof(Vertex));
	ctx->vertCount+=4;

	_check_vertex_cache_size(ctx);

	_add_tri_indices_for_rect(ctx, firstIdx);
}
#endif
void _font_cache_show_text_run (VkvgContext ctx, VkvgText tr) {
	unsigned int glyph_count;
#ifdef VKVG_USE_HARFBUZZ
	hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos (tr->hbBuf, &glyph_count);
#else
	vkvg_glyph_info_t* glyph_info = tr->glyphs;
	glyph_count = tr->glyph_count;
#endif

	Vertex v = {{0},ctx->curColor,{0,0,-1}};
	vec2 pen = {0,0};

	if (!_current_path_is_empty(ctx))
		pen = _get_current_position(ctx);

	LOCK_FONTCACHE (ctx->dev)

	for (uint32_t i=0; i < glyph_count; ++i) {
		_char_ref* cr = tr->font->charLookup[glyph_info[i].codepoint];

#ifdef VKVG_USE_HARFBUZZ
		if (cr==NULL)
			cr = _prepare_char(tr->dev, tr, glyph_info[i].codepoint);
#endif

		float uvWidth	= cr->bounds.width  / (float)FONT_PAGE_SIZE;
		float uvHeight	= cr->bounds.height / (float)FONT_PAGE_SIZE;
		vec2 p0 = {pen.x + cr->bmpDiff.x + (tr->glyphs[i].x_offset >> 6),
				   pen.y - cr->bmpDiff.y + (tr->glyphs[i].y_offset >> 6)};
		v.pos = p0;

		VKVG_IBO_INDEX_TYPE firstIdx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);


		v.uv.x = cr->bounds.x;
		v.uv.y = cr->bounds.y;
		v.uv.z = cr->pageIdx;
		_add_vertex(ctx,v);

		v.pos.y += cr->bounds.height;
		v.uv.y += uvHeight;
		_add_vertex(ctx,v);

		v.pos.x += cr->bounds.width;
		v.pos.y = p0.y;
		v.uv.x += uvWidth;
		v.uv.y = cr->bounds.y;
		_add_vertex(ctx,v);

		v.pos.y += cr->bounds.height;
		v.uv.y += uvHeight;
		_add_vertex(ctx,v);

		_add_tri_indices_for_rect (ctx, firstIdx);

		pen.x += (tr->glyphs[i].x_advance >> 6);
		pen.y -= (tr->glyphs[i].y_advance >> 6);
	}

	//equivalent to a moveto
	_finish_path(ctx);
	_add_point (ctx, pen.x, pen.y);
	_flush_chars_to_tex(tr->dev, tr->font);	
	UNLOCK_FONTCACHE (ctx->dev)

	if (ctx->fontCacheImg != ctx->dev->fontCache->texture) {
		vkvg_flush (ctx);
		_font_cache_update_context_descset (ctx);
	}
}

void _font_cache_show_text (VkvgContext ctx, const char* text){

	vkvg_text_run_t tr = {0};
	_font_cache_create_text_run (ctx, text, -1, &tr);

	if (ctx->status)
		return;

	_font_cache_show_text_run (ctx, &tr);

	_font_cache_destroy_text_run (&tr);

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

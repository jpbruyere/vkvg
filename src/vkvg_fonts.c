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

#include "vkvg_fonts.h"
#include "vkvg_context_internal.h"
#include "vkvg_surface_internal.h"
#include "vkvg_device_internal.h"

#include "vkh.h"

int defaultFontCharSize = 12<<6;

void _init_fonts_cache (VkvgDevice dev){
    _font_cache_t* cache = (_font_cache_t*)calloc(1, sizeof(_font_cache_t));

    cache->config = FcInitLoadConfigAndFonts();

    assert(!FT_Init_FreeType(&cache->library));

    cache->cacheTexLength = FONT_CACHE_INIT_LAYERS;
    cache->cacheTex = vkh_tex2d_array_create (dev, VK_FORMAT_R8_UNORM, FONT_PAGE_SIZE, FONT_PAGE_SIZE,
                            cache->cacheTexLength ,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkh_image_create_descriptor (cache->cacheTex, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_COLOR_BIT,
                                 VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    cache->uploadFence = vkh_fence_create_signaled(dev->vkDev);

    uint32_t buffLength = FONT_PAGE_SIZE*FONT_PAGE_SIZE*sizeof(uint8_t);
    cache->buff = vkh_buffer_create(dev,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      buffLength);
    vkh_buffer_map(cache->buff);

    cache->cmd = vkh_cmd_buff_create(dev->vkDev,dev->cmdPool,VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    cache->hostBuff = (uint8_t*)malloc(FONT_PAGE_SIZE*FONT_PAGE_SIZE*sizeof(uint8_t));
    cache->pensY = (int*)calloc(cache->cacheTexLength, sizeof(int));

    dev->fontCache = cache;
}
void _increase_font_tex_array (VkvgDevice dev){
    _font_cache_t* cache = dev->fontCache;

    vkWaitForFences     (dev->vkDev, 1, &cache->uploadFence, VK_TRUE, UINT64_MAX);
    vkResetCommandBuffer(cache->cmd, NULL);
    vkResetFences       (dev->vkDev, 1, &cache->uploadFence);

    uint8_t newSize = cache->cacheTexLength + FONT_CACHE_INIT_LAYERS;
    VkhImage newImg = vkh_tex2d_array_create (dev, VK_FORMAT_R8_UNORM, FONT_PAGE_SIZE, FONT_PAGE_SIZE,
                                              newSize ,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                              VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkh_image_create_descriptor (newImg, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_COLOR_BIT,
                               VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    VkImageSubresourceRange subresNew   = {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,newSize};
    VkImageSubresourceRange subres      = {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,cache->cacheTexLength};

    vkh_cmd_begin (cache->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    vkh_image_set_layout_subres(cache->cmd, newImg, subresNew, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    vkh_image_set_layout_subres(cache->cmd, cache->cacheTex, subres, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkImageCopy cregion = { .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, cache->cacheTexLength},
                            .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, cache->cacheTexLength},
                            .extent = {FONT_PAGE_SIZE,FONT_PAGE_SIZE,1}};

    vkCmdCopyImage (cache->cmd, vkh_image_get_vkimage (cache->cacheTex), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                vkh_image_get_vkimage (newImg), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cregion);

    vkh_image_set_layout_subres(cache->cmd, newImg, subresNew, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    VK_CHECK_RESULT(vkEndCommandBuffer(cache->cmd));

    vkh_cmd_submit      (dev->gQueue, &cache->cmd, cache->uploadFence);
    vkWaitForFences     (dev->vkDev, 1, &cache->uploadFence, VK_TRUE, UINT64_MAX);

    _flush_all_contexes (dev);

    cache->pensY = (int*)realloc(cache->pensY, newSize * sizeof(int));
    memset (cache->pensY + cache->cacheTexLength * sizeof(int),0,FONT_CACHE_INIT_LAYERS*sizeof(int));

    vkh_image_destroy   (cache->cacheTex);
    cache->cacheTexLength   = newSize;
    cache->cacheTex         = newImg;

    VkvgContext next = dev->lastCtx;
    while (next != NULL){
        _update_descriptor_set (next, next->source, next->dsSrc);
        next = next->pPrev;
    }

    _init_all_contexes  (dev);
}
void _init_next_line_in_tex_cache (VkvgDevice dev, _vkvg_font_t* f){
    _font_cache_t* cache = dev->fontCache;
    int i;
    for (i = 0; i < cache->cacheTexLength; ++i) {
        if (cache->pensY[i] + f->curLine.height >= FONT_PAGE_SIZE)
            continue;
        f->curLine.pageIdx = i;
        f->curLine.penX = 0;
        f->curLine.penY = cache->pensY[i];
        cache->pensY[i] += f->curLine.height;
        return;
    }
    _flush_chars_to_tex         (dev, f);
    _increase_font_tex_array    (dev);
    _init_next_line_in_tex_cache(dev, f);
}
void _destroy_font_cache (VkvgDevice dev){
    _font_cache_t* cache = (_font_cache_t*)dev->fontCache;

    //FcFini();

    free (cache->hostBuff);

    for (int i = 0; i < cache->fontsCount; ++i) {
        _vkvg_font_t f = cache->fonts[i];

        for (int g = 0; g < f.face->num_glyphs; ++g) {
            if (f.charLookup[g]!=NULL)
                free(f.charLookup[g]);
        }

        FT_Done_Face (f.face);
        hb_font_destroy (f.hb_font);

        free(f.charLookup);
        free(f.fontFile);
    }

    free(cache->fonts);
    free(cache->pensY);

    vkh_buffer_unmap    (cache->buff);
    vkh_buffer_destroy  (cache->buff);
    vkh_image_destroy   (cache->cacheTex);
    //vkFreeCommandBuffers(dev->vkDev,dev->cmdPool, 1, &cache->cmd);
    vkDestroyFence      (dev->vkDev,cache->uploadFence,NULL);

    free (dev->fontCache);

}

void _dump_glyphs (FT_Face face){
    FT_GlyphSlot    slot;
    char gname[256];

    for (int i = 0; i < face->num_glyphs; ++i) {
        assert(!FT_Load_Glyph(face,i,FT_LOAD_RENDER));
        slot = face->glyph;

        FT_Get_Glyph_Name(face,i,gname,256);


        printf("glyph: %s (%d,%d;%d), max advance:%d\n", gname,
               slot->bitmap.width, slot->bitmap.rows, slot->bitmap.pitch,
               face->size->metrics.max_advance/64);
    }
}

void _flush_chars_to_tex (VkvgDevice dev, _vkvg_font_t* f) {
    _font_cache_t* cache = dev->fontCache;
    if (cache->stagingX == 0)
        return;

    vkWaitForFences     (dev->vkDev,1,&cache->uploadFence,VK_TRUE,UINT64_MAX);
    vkResetCommandBuffer(cache->cmd,NULL);
    vkResetFences       (dev->vkDev,1,&cache->uploadFence);

    memcpy(vkh_buffer_get_mapped_pointer (cache->buff), cache->hostBuff, f->curLine.height * FONT_PAGE_SIZE);

    vkh_cmd_begin (cache->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkImageSubresourceRange subres      = {VK_IMAGE_ASPECT_COLOR_BIT,0,1,f->curLine.pageIdx,1};
    vkh_image_set_layout_subres(cache->cmd, cache->cacheTex, subres, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferImageCopy bufferCopyRegion = { .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT,0,f->curLine.pageIdx,1},
                                           .bufferRowLength = FONT_PAGE_SIZE,
                                           .bufferImageHeight = f->curLine.height,
                                           .imageOffset = {f->curLine.penX,f->curLine.penY,0},
                                           .imageExtent = {FONT_PAGE_SIZE-f->curLine.penX,f->curLine.height,1}};

    vkCmdCopyBufferToImage(cache->cmd, vkh_buffer_get_vkbuffer (cache->buff),
                           vkh_image_get_vkimage (cache->cacheTex), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

    vkh_image_set_layout_subres(cache->cmd, cache->cacheTex, subres, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    VK_CHECK_RESULT(vkEndCommandBuffer(cache->cmd));

    vkh_cmd_submit(dev->gQueue,&cache->cmd,cache->uploadFence);

    f->curLine.penX += cache->stagingX;
    cache->stagingX = 0;
    memset(cache->hostBuff, 0, FONT_PAGE_SIZE * FONT_PAGE_SIZE);
}

_char_ref* _prepare_char (VkvgDevice dev, _vkvg_font_t* f, FT_UInt gindex){
    assert(!FT_Load_Glyph(f->face, gindex, FT_LOAD_RENDER));

    FT_GlyphSlot slot = f->face->glyph;
    FT_Bitmap   bmp  = slot->bitmap;
    uint8_t*    data = dev->fontCache->hostBuff;

    if (dev->fontCache->stagingX + f->curLine.penX + bmp.width > FONT_PAGE_SIZE){
        _flush_chars_to_tex (dev, f);
        _init_next_line_in_tex_cache (dev, f);
    }

    int penX = dev->fontCache->stagingX;
    for(int y=0; y<bmp.rows; y++) {
        for(int x=0; x<bmp.width; x++)
            data[ penX + x + y * FONT_PAGE_SIZE ] =
                bmp.buffer[x + y * bmp.width];
    }

    _char_ref* cr = (_char_ref*)malloc(sizeof(_char_ref));
    vec4 uvBounds = {
        (float)(penX + f->curLine.penX) / (float)FONT_PAGE_SIZE,
        (float)f->curLine.penY / (float)FONT_PAGE_SIZE,
        bmp.width,
        bmp.rows};
    cr->bounds = uvBounds;
    cr->pageIdx = f->curLine.pageIdx;
    cr->bmpDiff.x = slot->bitmap_left;
    cr->bmpDiff.y = slot->bitmap_top;

    f->charLookup[gindex] = cr;
    dev->fontCache->stagingX += bmp.width;
    return cr;
}

void _set_font_size (VkvgContext ctx, uint32_t size){
    ctx->selectedFont.charSize = size << 6;
    ctx->currentFont = NULL;
}
void _select_font_face (VkvgContext ctx, const char* name){
    _font_cache_t*  cache = (_font_cache_t*)ctx->pSurf->dev->fontCache;

    char* fontFile;

    //make pattern from font name
    FcPattern* pat = FcNameParse((const FcChar8*)name);
    FcConfigSubstitute(cache->config, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    // find the font
    FcResult result;
    FcPattern* font = FcFontMatch(cache->config, pat, &result);
    if (font)
    {
        if (FcPatternGetString(font, FC_FILE, 0, (FcChar8 **)&fontFile) == FcResultMatch){
            memset (ctx->selectedFont.fontFile, 0, FONT_FILE_NAME_MAX_SIZE);
            strcpy(ctx->selectedFont.fontFile, fontFile);
        }
    }
    FcPatternDestroy(pat);
    FcPatternDestroy(font);

    ctx->currentFont =  NULL;
}

_vkvg_font_t* _tryFindVkvgFont (VkvgContext ctx){
    _font_cache_t*  cache = (_font_cache_t*)ctx->pSurf->dev->fontCache;
    for (int i = 0; i < cache->fontsCount; ++i) {
        if (strcmp (cache->fonts[i].fontFile, ctx->selectedFont.fontFile)==0 && cache->fonts[i].charSize == ctx->selectedFont.charSize)
            return &cache->fonts[i];
    }
    return NULL;
}

void _show_text (VkvgContext ctx, const char* text){
    VkvgDevice dev = ctx->pSurf->dev;

    if (ctx->currentFont == NULL){
        ctx->currentFont = _tryFindVkvgFont (ctx);
        if (ctx->currentFont == NULL){
            _font_cache_t*  cache = (_font_cache_t*)dev->fontCache;
            //create new font in cache
            cache->fontsCount++;
            if (cache->fontsCount == 1)
                cache->fonts = (_vkvg_font_t*) malloc (cache->fontsCount * sizeof(_vkvg_font_t));
            else
                cache->fonts = (_vkvg_font_t*) realloc (cache->fonts, cache->fontsCount * sizeof(_vkvg_font_t));

            _vkvg_font_t nf = ctx->selectedFont;
            if (nf.charSize == 0)
                nf.charSize = defaultFontCharSize;

            nf.fontFile = (char*)calloc(strlen(ctx->selectedFont.fontFile),sizeof(char));
            strcpy (nf.fontFile, ctx->selectedFont.fontFile);

            assert(!FT_New_Face(cache->library, nf.fontFile, 0, &nf.face));
            assert(!FT_Set_Char_Size(nf.face, 0, nf.charSize, dev->hdpi, dev->vdpi ));
            nf.hb_font = hb_ft_font_create(nf.face, NULL);
            nf.charLookup = (_char_ref*)calloc(nf.face->num_glyphs,sizeof(_char_ref));

            //nf.curLine.height = (nf.face->bbox.xMax - nf.face->bbox.xMin) >> 6;
            if (FT_IS_SCALABLE(nf.face))
                nf.curLine.height = nf.face->size->metrics.height >> 6;
            else
                nf.curLine.height = nf.face->height >> 6;

            _init_next_line_in_tex_cache (dev, &nf);
            cache->fonts[cache->fontsCount-1] = nf;
            ctx->currentFont = &cache->fonts[cache->fontsCount-1];
        }
    }

    hb_buffer_t *buf = hb_buffer_create();

    const char *lng  = "fr";
    hb_script_t script = HB_SCRIPT_LATIN;
    script = hb_script_from_string(text,strlen(text));
    hb_direction_t dir = hb_script_get_horizontal_direction(script);
    //dir = HB_DIRECTION_TTB;
    hb_buffer_set_direction (buf, dir);
    hb_buffer_set_script    (buf, script);
    hb_buffer_set_language  (buf, hb_language_from_string(lng,strlen(lng)));
    hb_buffer_add_utf8      (buf, text, strlen(text), 0, strlen(text));

    _vkvg_font_t* f = ctx->currentFont;
    hb_shape (f->hb_font, buf, NULL, 0);

    unsigned int         glyph_count;
    hb_glyph_info_t     *glyph_info   = hb_buffer_get_glyph_infos (buf, &glyph_count);
    hb_glyph_position_t *glyph_pos    = hb_buffer_get_glyph_positions (buf, &glyph_count);

    unsigned int string_width_in_pixels = 0;
    for (int i=0; i < glyph_count; ++i)
        string_width_in_pixels += glyph_pos[i].x_advance >> 6;


    Vertex v = {};
    vec2 pen = ctx->curPos;

    for (int i=0; i < glyph_count; ++i) {
        _char_ref* cr = f->charLookup[glyph_info[i].codepoint];

        if (cr==NULL)
            cr = _prepare_char(dev,f,glyph_info[i].codepoint);

        //continue;
        if (cr!=NULL){
            float uvWidth = cr->bounds.width / (float)FONT_PAGE_SIZE;
            float uvHeight = cr->bounds.height / (float)FONT_PAGE_SIZE;
            vec2 p0 = {pen.x + cr->bmpDiff.x + (glyph_pos[i].x_offset >> 6),
                       pen.y - cr->bmpDiff.y + (glyph_pos[i].y_offset >> 6)};
            v.pos = p0;

            uint32_t firstIdx = ctx->vertCount;

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
        }

        pen.x += (glyph_pos[i].x_advance >> 6);
        pen.y -= (glyph_pos[i].y_advance >> 6);
    }
    _flush_chars_to_tex(dev,f);
    //_show_texture(ctx); return;
}


void _show_texture (vkvg_context* ctx){
    Vertex vs[] = {
        {{0,0},                             {0,0,1}},
        {{0,FONT_PAGE_SIZE},                {0,1,1}},
        {{FONT_PAGE_SIZE,0},                {1,0,1}},
        {{FONT_PAGE_SIZE,FONT_PAGE_SIZE},   {1,1,1}}
    };

    _add_vertex(ctx,vs[0]);
    _add_vertex(ctx,vs[1]);
    _add_vertex(ctx,vs[2]);
    _add_vertex(ctx,vs[3]);

    _add_tri_indices_for_rect (ctx, 0);
}
/*void testfonts(){
    FT_Library      library;
    FT_Face         face;
    FT_GlyphSlot    slot;

    assert(!FT_Init_FreeType(&library));
    assert(!FT_New_Face(library, "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 0, &face));
    assert(!FT_Set_Char_Size(face, 0, ptSize, device_hdpi, device_vdpi ));

    //_build_face_tex(face);

    hb_font_t *hb_font = hb_ft_font_create(face, NULL);
    hb_buffer_t *buf = hb_buffer_create();

    const char *text = "Ленивый рыжий кот";
    const char *lng  = "en";
    //"كسول الزنجبيل القط","懶惰的姜貓",


    hb_buffer_set_direction (buf, HB_DIRECTION_LTR);
    hb_buffer_set_script    (buf, HB_SCRIPT_LATIN);
    hb_buffer_set_language  (buf, hb_language_from_string(lng,strlen(lng)));
    hb_buffer_add_utf8      (buf, text, strlen(text), 0, strlen(text));

    hb_unicode_funcs_t * unifc = hb_unicode_funcs_get_default();
    hb_script_t sc = hb_buffer_get_script(buf);

    sc = hb_unicode_script(unifc,0x0260);

    FT_CharMap* cm = face->charmap;

    //hb_script_to_iso15924_tag()


    FT_Done_Face    ( face );
    FT_Done_FreeType( library );
}*/


//void main(){
//    testfonts();
//}


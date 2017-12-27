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
#include "vkh_image.h"


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
    VkhBuffer       buff;       //stagin buffer
    VkhImage		cacheTex;	//tex 2d array
    uint8_t         cacheTexLength;  //tex array length
    int*            pensY;      //y pen pos in each texture of array
    VkFence			uploadFence;

    _vkvg_font_t*	fonts;
    uint8_t			fontsCount;
}_font_cache_t;

void _init_fonts_cache		(VkvgDevice dev);
void _destroy_font_cache	(VkvgDevice dev);
void _select_font_face		(VkvgContext ctx, const char* name);
void _set_font_size         (VkvgContext ctx, uint32_t size);
void _show_text				(VkvgContext ctx, const char* text);
#endif

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
#ifndef VKVG_H
#define VKVG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vulkan/vulkan.h>
#include <math.h>

#ifdef VKVG_TILING_OPTIMAL
    #define VKVG_TILING VK_IMAGE_TILING_OPTIMAL
#else
    #define VKVG_TILING VK_IMAGE_TILING_LINEAR
#endif

#define VKVG_SAMPLES 4

#define LOG_ERR			0x00
#define LOG_DEBUG		0x10
#define LOG_INFO		0x20
#define LOG_INFO_PATH   0x40
#define LOG_FULL		0xff

#ifdef DEBUG
static uint8_t log_level	= LOG_ERR;// LOG_INFO | LOG_DEBUG;
#define LOG(level,...) (log_level & level) ? fprintf (stdout, __VA_ARGS__):true;
#else
#define LOG
#endif

typedef enum _vkvg_status {
    VKVG_STATUS_SUCCESS,
    VKVG_STATUS_NO_MEMORY
}vkvg_status_t;

typedef enum _vkvg_direction {
    VKVG_HORIZONTAL	= 0,
    VKVG_VERTICAL	= 1
}vkvg_direction_t;

typedef enum _vkvg_format {
    VKVG_FORMAT_ARGB32,
    VKVG_FORMAT_RGB24,
    VKVG_FORMAT_A8,
    VKVG_FORMAT_A1
} vkvg_format_t;

typedef enum _vkvg_extend {
    VKVG_EXTEND_NONE,
    VKVG_EXTEND_REPEAT,
    VKVG_EXTEND_REFLECT,
    VKVG_EXTEND_PAD
} vkvg_extend_t;

typedef enum _vkvg_filter {
    VKVG_FILTER_FAST,
    VKVG_FILTER_GOOD,
    VKVG_FILTER_BEST,
    VKVG_FILTER_NEAREST,
    VKVG_FILTER_BILINEAR,
    VKVG_FILTER_GAUSSIAN,
} vkvg_filter_t;

typedef enum _vkvg_pattern_type {
    VKVG_PATTERN_TYPE_SOLID,
    VKVG_PATTERN_TYPE_SURFACE,
    VKVG_PATTERN_TYPE_LINEAR,
    VKVG_PATTERN_TYPE_RADIAL,
    VKVG_PATTERN_TYPE_MESH,
    VKVG_PATTERN_TYPE_RASTER_SOURCE,
} vkvg_pattern_type_t;

typedef enum _vkvg_line_cap {
    VKVG_LINE_CAP_BUTT,
    VKVG_LINE_CAP_ROUND,
    VKVG_LINE_CAP_SQUARE
} vkvg_line_cap_t;

typedef enum _vkvg_line_join {
    VKVG_LINE_JOIN_MITER,
    VKVG_LINE_JOIN_ROUND,
    VKVG_LINE_JOIN_BEVEL
} vkvg_line_join_t;

typedef struct _vkvg_color_t{
    float r;
    float g;
    float b;
    float a;
} vkvg_color_t;

#define VKVG_IDENTITY_MATRIX {1,0,0,1,0,0}

typedef struct {
    float xx; float yx;
    float xy; float yy;
    float x0; float y0;
} vkvg_matrix_t;

typedef struct {
    float ascent;
    float descent;
    float height;
    float max_x_advance;
    float max_y_advance;
} vkvg_font_extents_t;

typedef struct {
    float x_bearing;
    float y_bearing;
    float width;
    float height;
    float x_advance;
    float y_advance;
} vkvg_text_extents_t;

typedef struct _vkvg_text_run_t* VkvgText;

typedef struct _vkvg_context_t* VkvgContext;
typedef struct _vkvg_surface_t* VkvgSurface;
typedef struct _vkvg_device_t*  VkvgDevice;
typedef struct _vkvg_pattern_t* VkvgPattern;

VkvgDevice	vkvg_device_create			(VkInstance inst, VkPhysicalDevice phy, VkDevice vkdev, uint32_t qFamIdx, uint32_t qIndex);
void		vkvg_device_destroy			(VkvgDevice dev);
VkvgDevice  vkvg_device_reference           (VkvgDevice dev);
uint32_t    vkvg_device_get_reference_count (VkvgDevice dev);

VkvgSurface vkvg_surface_create			(VkvgDevice dev, uint32_t width, uint32_t height);
VkvgSurface vkvg_surface_create_from_image  (VkvgDevice dev, const char* filePath);
// VkvgSurface vkvg_surface_create_from_bitmap (VkvgDevice dev, unsigned char* img, uint32_t width, uint32_t height);
VkvgSurface vkvg_surface_reference          (VkvgSurface surf);
uint32_t    vkvg_surface_get_reference_count(VkvgSurface surf);

void		vkvg_surface_destroy		(VkvgSurface surf);
void        vkvg_surface_clear          (VkvgSurface surf);
VkImage		vkvg_surface_get_vk_image	(VkvgSurface surf);
VkFormat	vkvg_surface_get_vk_format	(VkvgSurface surf);
uint32_t   	vkvg_surface_get_width      (VkvgSurface surf);
uint32_t	vkvg_surface_get_height     (VkvgSurface surf);
VkImage		vkvg_surface_get_vkh_image	(VkvgSurface surf);
void        vkvg_surface_write_to_png   (VkvgSurface surf, const char* path);

//mimic from cairo, to facilitate usage of vkvg as cairo vulkan backend
typedef enum _vkvg_operator {
    VKVG_OPERATOR_CLEAR,

    VKVG_OPERATOR_SOURCE,
    VKVG_OPERATOR_OVER,
    VKVG_OPERATOR_IN,
    VKVG_OPERATOR_OUT,
    VKVG_OPERATOR_ATOP,

    VKVG_OPERATOR_DEST,
    VKVG_OPERATOR_DEST_OVER,
    VKVG_OPERATOR_DEST_IN,
    VKVG_OPERATOR_DEST_OUT,
    VKVG_OPERATOR_DEST_ATOP,

    VKVG_OPERATOR_XOR,
    VKVG_OPERATOR_ADD,
    VKVG_OPERATOR_SATURATE,

    VKVG_OPERATOR_MULTIPLY,
    VKVG_OPERATOR_SCREEN,
    VKVG_OPERATOR_OVERLAY,
    VKVG_OPERATOR_DARKEN,
    VKVG_OPERATOR_LIGHTEN,
    VKVG_OPERATOR_COLOR_DODGE,
    VKVG_OPERATOR_COLOR_BURN,
    VKVG_OPERATOR_HARD_LIGHT,
    VKVG_OPERATOR_SOFT_LIGHT,
    VKVG_OPERATOR_DIFFERENCE,
    VKVG_OPERATOR_EXCLUSION,
    VKVG_OPERATOR_HSL_HUE,
    VKVG_OPERATOR_HSL_SATURATION,
    VKVG_OPERATOR_HSL_COLOR,
    VKVG_OPERATOR_HSL_LUMINOSITY
} vkvg_operator_t;

/*Context*/
VkvgContext vkvg_create             (VkvgSurface surf);
void        vkvg_destroy			(VkvgContext ctx);
VkvgContext vkvg_reference          (VkvgContext ctx);
uint32_t    vkvg_get_reference_count(VkvgContext ctx);

void vkvg_flush				(VkvgContext ctx);

void vkvg_new_path          (VkvgContext ctx);
void vkvg_close_path		(VkvgContext ctx);
void vkvg_new_sub_path      (VkvgContext ctx);
void vkvg_line_to			(VkvgContext ctx, float x, float y);
void vkvg_rel_line_to       (VkvgContext ctx, float x, float y);
void vkvg_move_to			(VkvgContext ctx, float x, float y);
void vkvg_rel_move_to		(VkvgContext ctx, float x, float y);
void vkvg_arc				(VkvgContext ctx, float xc, float yc, float radius, float a1, float a2);
void vkvg_arc_negative      (VkvgContext ctx, float xc, float yc, float radius, float a1, float a2);
void vkvg_curve_to          (VkvgContext ctx, float x1, float y1, float x2, float y2, float x3, float y3);
void vkvg_rectangle         (VkvgContext ctx, float x, float y, float w, float h);
void vkvg_stroke			(VkvgContext ctx);
void vkvg_stroke_preserve	(VkvgContext ctx);
void vkvg_fill				(VkvgContext ctx);
void vkvg_fill_preserve		(VkvgContext ctx);
void vkvg_paint             (VkvgContext ctx);
void vkvg_clear             (VkvgContext ctx);//use vkClearAttachment to speed up clearing surf
void vkvg_reset_clip        (VkvgContext ctx);
void vkvg_clip              (VkvgContext ctx);
void vkvg_clip_preserve     (VkvgContext ctx);
void vkvg_set_source_rgba	(VkvgContext ctx, float r, float g, float b, float a);
void vkvg_set_source_rgb    (VkvgContext ctx, float r, float g, float b);
void vkvg_set_line_width	(VkvgContext ctx, float width);
void vkvg_set_line_cap      (VkvgContext ctx, vkvg_line_cap_t cap);
void vkvg_set_line_join     (VkvgContext ctx, vkvg_line_join_t join);
void vkvg_set_source_surface(VkvgContext ctx, VkvgSurface surf, float x, float y);
void vkvg_set_source        (VkvgContext ctx, VkvgPattern pat);
void vkvg_set_operator      (VkvgContext ctx, vkvg_operator_t op);

float               vkvg_get_line_width     (VkvgContext ctx);
vkvg_line_cap_t     vkvg_get_line_cap       (VkvgContext ctx);
vkvg_line_join_t    vkvg_get_line_join      (VkvgContext ctx);
vkvg_operator_t     vkvg_get_operator       (VkvgContext ctx);
VkvgPattern         vkvg_get_source         (VkvgContext ctx);

void vkvg_save              (VkvgContext ctx);
void vkvg_restore           (VkvgContext ctx);

void vkvg_translate         (VkvgContext ctx, float dx, float dy);
void vkvg_scale             (VkvgContext ctx, float sx, float sy);
void vkvg_rotate            (VkvgContext ctx, float radians);
void vkvg_transform         (VkvgContext ctx, const vkvg_matrix_t* matrix);
void vkvg_set_matrix        (VkvgContext ctx, const vkvg_matrix_t* matrix);
void vkvg_get_matrix        (VkvgContext ctx, const vkvg_matrix_t* matrix);
void vkvg_identity_matrix   (VkvgContext ctx);

//text
void vkvg_select_font_face	(VkvgContext ctx, const char* name);
void vkvg_set_font_size		(VkvgContext ctx, uint32_t size);
void vkvg_show_text			(VkvgContext ctx, const char* text);
void vkvg_text_extents      (VkvgContext ctx, const char* text, vkvg_text_extents_t* extents);
void vkvg_font_extents      (VkvgContext ctx, vkvg_font_extents_t* extents);

//text run holds harfbuz datas, and prevent recreating them multiple times for the same line of text.
VkvgText    vkvg_text_run_create    (VkvgContext ctx, const char* text);
void        vkvg_text_run_destroy   (VkvgText textRun);
void        vkvg_show_text_run      (VkvgContext ctx, VkvgText textRun);
vkvg_text_extents_t* vkvg_text_run_get_extents (VkvgText textRun);

//pattern
VkvgPattern vkvg_pattern_create             ();
VkvgPattern vkvg_pattern_reference          (VkvgPattern pat);
uint32_t    vkvg_pattern_get_reference_count(VkvgPattern pat);
VkvgPattern vkvg_pattern_create_rgba        (float r, float g, float b, float a);
VkvgPattern vkvg_pattern_create_rgb         (float r, float g, float b);
VkvgPattern vkvg_pattern_create_for_surface (VkvgSurface surf);
VkvgPattern vkvg_pattern_create_linear      (float x0, float y0, float x1, float y1);
VkvgPattern vkvg_pattern_create_radial      (float cx0, float cy0, float radius0,
                                             float cx1, float cy1, float radius1);
void        vkvg_pattern_destroy            (VkvgPattern pat);

void vkvg_patter_add_color_stop (VkvgPattern pat, float offset, float r, float g, float b, float a);
void vkvg_pattern_set_extend    (VkvgPattern pat, vkvg_extend_t extend);
void vkvg_pattern_set_filter    (VkvgPattern pat, vkvg_filter_t filter);

vkvg_extend_t   vkvg_pattern_get_extend (VkvgPattern pat);
vkvg_filter_t   vkvg_pattern_get_filter (VkvgPattern pat);

//matrix
void vkvg_matrix_init_identity (vkvg_matrix_t *matrix);
void vkvg_matrix_init (vkvg_matrix_t *matrix,
           float xx, float yx,
           float xy, float yy,
           float x0, float y0);
void vkvg_matrix_init_translate     (vkvg_matrix_t *matrix, float tx, float ty);
void vkvg_matrix_init_scale         (vkvg_matrix_t *matrix, float sx, float sy);
void vkvg_matrix_init_rotate        (vkvg_matrix_t *matrix, float radians);
void vkvg_matrix_translate          (vkvg_matrix_t *matrix, float tx, float ty);
void vkvg_matrix_scale              (vkvg_matrix_t *matrix, float sx, float sy);
void vkvg_matrix_rotate             (vkvg_matrix_t *matrix, float radians);
void vkvg_matrix_multiply           (vkvg_matrix_t *result, const vkvg_matrix_t *a, const vkvg_matrix_t *b);
void vkvg_matrix_transform_distance (const vkvg_matrix_t *matrix, float *dx, float *dy);
void vkvg_matrix_transform_point    (const vkvg_matrix_t *matrix, float *x, float *y);
void vkvg_matrix_invert             (vkvg_matrix_t *matrix);

#ifdef __cplusplus
}
#endif

#endif

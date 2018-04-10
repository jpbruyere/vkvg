#ifndef VKVG_H
#define VKVG_H

#include <vulkan/vulkan.h>
#include <math.h>

#define VKVG_SAMPLES 4

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

typedef struct _vkvg_context_t* VkvgContext;
typedef struct _vkvg_surface_t* VkvgSurface;
typedef struct _vkvg_device_t*  VkvgDevice;
typedef struct _vkvg_pattern_t* VkvgPattern;

VkvgDevice	vkvg_device_create			(VkPhysicalDevice phy, VkDevice vkdev, VkQueue queue, uint32_t qFam);
void		vkvg_device_destroy			(VkvgDevice dev);

VkvgSurface vkvg_surface_create			(VkvgDevice dev, uint32_t width, uint32_t height);
VkvgSurface vkvg_surface_create_from_image  (VkvgDevice dev, const char* filePath);

void		vkvg_surface_destroy		(VkvgSurface surf);
VkImage		vkvg_surface_get_vk_image	(VkvgSurface surf);
VkImage		vkvg_surface_get_vkh_image	(VkvgSurface surf);

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
VkvgContext vkvg_create		(VkvgSurface surf);
void vkvg_destroy			(VkvgContext ctx);

void vkvg_flush				(VkvgContext ctx);

void vkvg_close_path		(VkvgContext ctx);
void vkvg_line_to			(VkvgContext ctx, float x, float y);
void vkvg_move_to			(VkvgContext ctx, float x, float y);
void vkvg_arc				(VkvgContext ctx, float xc, float yc, float radius, float a1, float a2);
void vkvg_curve_to          (VkvgContext ctx, float x1, float y1, float x2, float y2, float x3, float y3);
void vkvg_rectangle         (VkvgContext ctx, float x, float y, float w, float h);
void vkvg_stroke			(VkvgContext ctx);
void vkvg_stroke_preserve	(VkvgContext ctx);
void vkvg_fill				(VkvgContext ctx);
void vkvg_fill_preserve		(VkvgContext ctx);
void vkvg_paint             (VkvgContext ctx);
void vkvg_reset_clip        (VkvgContext ctx);
void vkvg_clip              (VkvgContext ctx);
void vkvg_clip_preserve     (VkvgContext ctx);
void vkvg_set_rgba			(VkvgContext ctx, float r, float g, float b, float a);
void vkvg_set_linewidth		(VkvgContext ctx, float width);
void vkvg_set_source_surface(VkvgContext ctx, VkvgSurface surf, float x, float y);

void vkvg_select_font_face	(VkvgContext ctx, const char* name);
void vkvg_set_font_size		(VkvgContext ctx, uint32_t size);
void vkvg_show_text			(VkvgContext ctx, const char* text);

void vkvg_save              (VkvgContext ctx);
void vkvg_restore           (VkvgContext ctx);

#endif

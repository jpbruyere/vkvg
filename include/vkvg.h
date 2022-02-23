/*
 * Copyright (c) 2018-2020 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
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

/*************************************************************************
 * Doxygen documentation
 *************************************************************************/
/** @mainpage VKVG: vulkan vector graphics
 *
 * VKVG is an open source 2d vector drawing library written in @b c and using [vulkan](https://www.khronos.org/vulkan/) for hardware acceleration.
 * Its api is modeled on the [cairo graphic library](https://www.cairographics.org/) with the following software components:
 *
 * - @ref surface
 * - @ref context
 * - @ref device
 * - @ref pattern
 */

/*! @file vkvg.h
 *	@brief The header of the VKVG library.
 *
 *	This is the header file of the VKVG library.  It defines all its types and
 *	declares all its functions.
 *
 *	For more information about how to use this file, see @ref build_include.
 */
/*! @defgroup surface Surface
 *	@brief Functions and types related to VKVG surface.
 *
 *	This is the reference documentation for creating, using and destroying VKVG
 *	Surfaces used as backend for drawing operations.
 */
/*! @defgroup context Context
 *	@brief Functions and types related to VKVG contexts.
 *
 *	This is the reference documentation for VKVG contexts used to draw on @ref surface.
 */
/*! @defgroup path Path creation and manipulation reference.
 *	@brief Functions and types related to path edition.
 */

#include <vulkan/vulkan.h>
#include <math.h>
#include <stdbool.h>

#ifndef vkvg_public
	#ifdef VKVG_SHARED_BUILD
		#if (defined(_WIN32) || defined(_WIN64))
			#define vkvg_public __declspec(dllimport)
		#else
			#define vkvg_public __attribute__((visibility("default")))
		#endif
	#else
		#define vkvg_public
	#endif
#endif


#define VKVG_LOG_ERR		0x00000001
#define VKVG_LOG_DEBUG		0x00000002

#define VKVG_LOG_INFO_PTS	0x00000004
#define VKVG_LOG_INFO_PATH	0x00000008
#define VKVG_LOG_INFO_CMD	0x00000010
#define VKVG_LOG_INFO_VBO	0x00000020
#define VKVG_LOG_INFO_IBO	0x00000040
#define VKVG_LOG_INFO_VAO	(VKVG_LOG_INFO_VBO|VKVG_LOG_INFO_IBO)
#define VKVG_LOG_DBG_ARRAYS	0x00001000
#define VKVG_LOG_STROKE		0x00010000
#define VKVG_LOG_FULL		0xffffffff

#define VKVG_LOG_INFO		0x00008000//(VKVG_LOG_INFO_PTS|VKVG_LOG_INFO_PATH|VKVG_LOG_INFO_CMD|VKVG_LOG_INFO_VAO)
#ifdef DEBUG
	extern uint32_t vkvg_log_level;
	#ifdef VKVG_WIRED_DEBUG
		typedef enum {
			vkvg_wired_debug_mode_normal	= 0x01,
			vkvg_wired_debug_mode_points	= 0x02,
			vkvg_wired_debug_mode_lines		= 0x04,
			vkvg_wired_debug_mode_both		= vkvg_wired_debug_mode_points|vkvg_wired_debug_mode_lines,
			vkvg_wired_debug_mode_all		= 0xFFFFFFFF
		}vkvg_wired_debug_mode;
		extern vkvg_wired_debug_mode vkvg_wired_debug;
	#endif
#endif

/**
 * @brief vkvg operation status.
 *
 * vkvg_status_t is used to indicates errors that can occur when using vkvg. Several vkvg function directely
 * return result, but when using a @ref context, the last error is stored in the context and can be accessed
 * with @ref vkvg_status().
 *
 * As soon as a status is not success, further operations will be canceled.
 */
typedef enum {
	VKVG_STATUS_SUCCESS = 0,		/*!< no error occurred.*/
	VKVG_STATUS_NO_MEMORY,			/*!< out of memory*/
	VKVG_STATUS_INVALID_RESTORE,		/*!< call to #vkvg_restore without matching call to #vkvg_save*/
	VKVG_STATUS_NO_CURRENT_POINT,		/*!< path command expecting a current point to be defined failed*/
	VKVG_STATUS_INVALID_MATRIX,			/*!< invalid matrix (not invertible)*/
	VKVG_STATUS_INVALID_STATUS,			/*!< */
	VKVG_STATUS_INVALID_INDEX,			/*!< */
	VKVG_STATUS_NULL_POINTER,			/*!< NULL pointer*/
	VKVG_STATUS_WRITE_ERROR,			/*!< */
	VKVG_STATUS_PATTERN_TYPE_MISMATCH,	/*!< */
	VKVG_STATUS_PATTERN_INVALID_GRADIENT,/*!< occurs when stops count is zero */
	VKVG_STATUS_INVALID_FORMAT,			/*!< */
	VKVG_STATUS_FILE_NOT_FOUND,			/*!< */
	VKVG_STATUS_INVALID_DASH,			/*!< invalid value for a dash setting */
	VKVG_STATUS_INVALID_RECT,			/*!< rectangle with height or width equal to 0. */
	VKVG_STATUS_TIMEOUT,				/*!< waiting for a vulkan operation to finish resulted in a fence timeout (5 seconds)*/
	VKVG_STATUS_DEVICE_ERROR,			/*!< vkvg device initialization error */
	VKVG_STATUS_INVALID_IMAGE,			/*!< */
	VKVG_STATUS_INVALID_SURFACE,		/*!< */
	VKVG_STATUS_INVALID_FONT,		/*!< unresolved font name */
	VKVG_STATUS_INVALID_POP_GROUP		/*!< the surface is the first element on the stack */
}vkvg_status_t;

typedef enum {
	VKVG_HORIZONTAL	= 0,
	VKVG_VERTICAL	= 1
}vkvg_direction_t;

typedef enum {
	VKVG_FORMAT_ARGB32,
	VKVG_FORMAT_RGB24,
	VKVG_FORMAT_A8,
	VKVG_FORMAT_A1
} vkvg_format_t;
/**
 * @brief pattern border policy
 *
 * when painting a pattern on a surface, if the input bounds are smaller than the target bounds,
 * the extend defines how the pattern will be rendered outside its original bounds.
 */
typedef enum {
	VKVG_EXTEND_NONE,			/*!< nothing will be outputed outside the pattern original bounds */
	VKVG_EXTEND_REPEAT,			/*!< pattern will be repeated to fill all the target bounds */
	VKVG_EXTEND_REFLECT,		/*!< pattern will be repeated but mirrored on each repeat */
	VKVG_EXTEND_PAD				/*!< the last pixels making the borders of the pattern will be extended to the whole target */
} vkvg_extend_t;


typedef enum {
	VKVG_FILTER_FAST,
	VKVG_FILTER_GOOD,
	VKVG_FILTER_BEST,
	VKVG_FILTER_NEAREST,
	VKVG_FILTER_BILINEAR,
	VKVG_FILTER_GAUSSIAN,
} vkvg_filter_t;

/**
 * @brief pattern types
 *
 *
 */
typedef enum {
	VKVG_PATTERN_TYPE_SOLID,			/*!< single color pattern */
	VKVG_PATTERN_TYPE_SURFACE,			/*!< vkvg surface pattern */
	VKVG_PATTERN_TYPE_LINEAR,			/*!< linear gradient pattern */
	VKVG_PATTERN_TYPE_RADIAL,			/*!< radial gradient pattern */
	VKVG_PATTERN_TYPE_MESH,				/*!< not implemented */
	VKVG_PATTERN_TYPE_RASTER_SOURCE,	/*!< not implemented */
} vkvg_pattern_type_t;

/**
 * @brief line caps
 *
 * define the path ends shapes which may be rounded or squared. The context hold the current line cap
 * configuration which may be accessed with #vkvg_set_line_cap and #vkvg_get_line_cap
 */
typedef enum {
	VKVG_LINE_CAP_BUTT,		/*!< normal line endings, this is the default. */
	VKVG_LINE_CAP_ROUND,	/*!< rounded line caps */
	VKVG_LINE_CAP_SQUARE	/*!< extend the caps with squared terminations having border equal to current line width. */
} vkvg_line_cap_t;
/**
 * @brief lines articulations
 *
 * define the joins shape for the stroke command between consecutive lines or curves. This setting is holded
 * in the context and may be accessed with #vkvg_set_line_join and #vkvg_get_line_join.
 */
typedef enum {
	VKVG_LINE_JOIN_MITER,	/*!< normal joins with sharp angles, this is the default. */
	VKVG_LINE_JOIN_ROUND,	/*!< joins are rounded on the exterior border of the line. */
	VKVG_LINE_JOIN_BEVEL	/*!< beveled line joins. */
} vkvg_line_join_t;

/**
 * @brief shape fill method
 *
 * define technique used to fill a path with the #vkvg_fill command. This setting can be accessed in the context
 * with #vkvg_set_fill_rule and #vkvg_get_fill_rule.
 */
typedef enum {
	VKVG_FILL_RULE_EVEN_ODD,	/*!< stencil even-odd technique */
	VKVG_FILL_RULE_NON_ZERO		/*!< ear clipping filling */
} vkvg_fill_rule_t;

typedef struct {
	float r;
	float g;
	float b;
	float a;
} vkvg_color_t;

/**
  * @brief font metrics
  *
  * structure defining global font metrics for a particular font. It can be retrieve by calling @ref vkvg_font_extents
  * on a valid context.
  */
typedef struct {
	float ascent;			/*!< the distance that the font extends above the baseline. */
	float descent;			/*!< the distance that the font extends below the baseline.*/
	float height;			/*!< the recommended vertical distance between baselines. */
	float max_x_advance;	/*!< the maximum distance in the X direction that the origin is advanced for any glyph in the font.*/
	float max_y_advance;	/*!< the maximum distance in the Y direction that the origin is advanced for any glyph in the font. This will be zero for normal fonts used for horizontal writing.*/
} vkvg_font_extents_t;
/**
  * @brief text metrics
  *
  * structure defining metrics for a single or a string of glyphs. To measure text, call @ref vkvg_text_extents
  * on a valid context.
  */
typedef struct {
	float x_bearing;		/*!< the horizontal distance from the origin to the leftmost part of the glyphs as drawn. Positive if the glyphs lie entirely to the right of the origin. */
	float y_bearing;		/*!< the vertical distance from the origin to the topmost part of the glyphs as drawn. Positive only if the glyphs lie completely below the origin; will usually be negative.*/
	float width;			/*!< width of the glyphs as drawn*/
	float height;			/*!< height of the glyphs as drawn*/
	float x_advance;		/*!< distance to advance in the X direction after drawing these glyphs*/
	float y_advance;		/*!< distance to advance in the Y direction after drawing these glyphs. Will typically be zero except for vertical text layout as found in East-Asian languages.*/
} vkvg_text_extents_t;

/**
  * @brief glyphs position in a @ref VkvgText
  *
  * structure defining glyph position as computed for rendering a text run.
  * the codepoint field is for internal use only.
  */
typedef struct _glyph_info_t {
	int32_t  x_advance;
	int32_t  y_advance;
	int32_t  x_offset;
	int32_t  y_offset;
	/* private */
	uint32_t codepoint;//should be named glyphIndex, but for harfbuzz compatibility...
} vkvg_glyph_info_t;

/**
 * @brief Opaque pointer on a vkvg text run.
 *
 * A #VkvgText is an intermediate representation
 * of a text to be drawn on a #VkvgSurface.
 * It contains the measurments computed for character poisitioning.
 *
 * This object is used to speed up the rendering of the same text with the same font multiple times
 * by storing typographic computations.
 *
 * Drawing text with #vkvg_show_text() implicitly create such intermediate structure
 * that is destroyed imediatly after the function call.
 */
typedef struct _vkvg_text_run_t* VkvgText;

/**
 * @brief The Vkvg drawing Context.
 * @ingroup context
 *
 * A #VkvgContext is the central object for drawing operations.
 * #vkvg_context_t structure internals this pointer point to are
 * hidden to the client application.
 */
typedef struct _vkvg_context_t* VkvgContext;
/**
 * @brief Opaque pointer on a Vkvg Surface structure.
 * @ingroup surface
 *
 * A #VkvgSurface represents an image, either as the destination
 * of a drawing operation or as source when drawing onto another
 * surface.	 To draw to a #VkvgSurface, create a vkvg context
 * with the surface as the target, using #vkvg_create().
 * hidden internals.
 *
 * #VkvgSurface are created with a @ref VkvgDevice which has to stay
 * active as long has the surface is in use.
 */
typedef struct _vkvg_surface_t* VkvgSurface;
/**
 * @brief Opaque pointer on a Vkvg device structure.
 * @ingroup device
 *
 * A #VkvgDevice is required for creating new surfaces.
 */
typedef struct _vkvg_device_t*	VkvgDevice;
/**
 * @brief Opaque pointer on a Vkvg pattern structure.
 * @ingroup pattern
 *
 * Patterns are images to be drawn on surface with several
 * configurable parameters such as the wrap mode, the filtering, etc...
 */
typedef struct _vkvg_pattern_t* VkvgPattern;

#if VKVG_DBG_STATS
/**
 * @brief vkvg memory and vulkan statistiques.
 *
 * @ingroup device
 */
typedef struct {
	uint32_t	sizePoints;	/**< maximum point array size				*/
	uint32_t	sizePathes;	/**< maximum path array size				*/
	uint32_t	sizeVertices;	/**< maximum size of host vertice cache			*/
	uint32_t	sizeIndices;	/**< maximum size of host index cache			*/
	uint32_t	sizeVBO;	/**< maximum size of vulkan vertex buffer		*/
	uint32_t	sizeIBO;	/**< maximum size of vulkan index buffer		*/
} vkvg_debug_stats_t;

vkvg_debug_stats_t vkvg_device_get_stats (VkvgDevice dev);
vkvg_debug_stats_t vkvg_device_reset_stats (VkvgDevice dev);
#endif


/**
 * @defgroup matrix Matrices
 * @brief Generic matrix operations
 *
 * This is the reference documentation for handling matrices to use as transformation in drawing operations.
 * Matrix computations in vkvg are taken from the cairo library.
 * @{ */
#define VKVG_IDENTITY_MATRIX (vkvg_matrix_t){1,0,0,1,0,0}/*!< The identity matrix*/
/**
 * @brief vkvg matrix structure
 *
 * A #vkvg_matrix_t holds an affine transformation, such as a scale,
 * rotation, shear, or a combination of those. The transformation of
 * a point (x, y) is given by:
 * @code
 * x_new = xx * x + xy * y + x0;
 * y_new = yx * x + yy * y + y0;
 * @endcode
 * @xx: xx component of the affine transformation
 * @yx: yx component of the affine transformation
 * @xy: xy component of the affine transformation
 * @yy: yy component of the affine transformation
 * @x0: X translation component of the affine transformation
 * @y0: Y translation component of the affine transformation
 */
typedef struct {
	float xx; float yx;
	float xy; float yy;
	float x0; float y0;
} vkvg_matrix_t;
/**
 * @brief Set matrix to identity
 *
 * Initialize members of the supplied #vkvg_matrix_t to make an identity matrix of it.
 * @param matrix a valid #vkvg_matrix_t pointer.
 */
vkvg_public
void vkvg_matrix_init_identity (vkvg_matrix_t *matrix);
/**
 * @brief Matrix initialization.
 *
 * Initialize members of the supplied matrix to the values passed as arguments.
 * Resulting matrix will hold an affine transformation defined by the parameters.
 * @param matrix a valid #vkvg_matrix_t pointer
 * @param xx xx component of the affine transformation
 * @param yx yx component of the affine transformation
 * @param xy xy component of the affine transformation
 * @param yy yy component of the affine transformation
 * @param x0 X translation component of the affine transformation
 * @param y0 Y translation component of the affine transformation
 */
vkvg_public
void vkvg_matrix_init (vkvg_matrix_t *matrix,
		   float xx, float yx,
		   float xy, float yy,
		   float x0, float y0);
/**
 * @brief Rotation matrix initialization
 *
 * Initialize members of the supplied matrix to create a translation matrix.
 * @param matrix a valid #vkvg_matrix_t pointer
 * @param tx translation in the X direction
 * @param ty translation in the Y direction
 */
vkvg_public
void vkvg_matrix_init_translate (vkvg_matrix_t *matrix, float tx, float ty);
/**
 * @brief scaling matrix initialization
 *
 * Initialize members of the supplied matrix to create a new scaling matrix
 * @param matrix a valid #vkvg_matrix_t pointer
 * @param sx scale in the x direction
 * @param sy Scale in the y direction
 */
vkvg_public
void vkvg_matrix_init_scale (vkvg_matrix_t *matrix, float sx, float sy);
/**
 * @brief rotation matrix initialization
 *
 * Initialize members of the supplied matrix to create a new rotation matrix
 * @param matrix a valid #vkvg_matrix_t pointer
 * @param radians angle of rotation, in radians. The direction of rotation
 * is defined such that positive angles rotate in the direction from
 * the positive X axis toward the positive Y axis. With the default
 * axis orientation of vkvg, positive angles rotate in a clockwise
 * direction.
 */
vkvg_public
void vkvg_matrix_init_rotate (vkvg_matrix_t *matrix, float radians);
/**
 * @brief apply translation on matrix
 *
 * Apply the translation defined by tx and ty on the supplied matrix.
 * The effect of the new transformation is to first translate the coordinates by tx and ty,
 * then apply the original transformation to the coordinates.
 * @param matrix a valid #vkvg_matrix_t pointer
 * @param tx translation in the x direction
 * @param ty translation in the y direction
 */
vkvg_public
void vkvg_matrix_translate (vkvg_matrix_t *matrix, float tx, float ty);
/**
 * @brief apply scale on matrix
 *
 * Apply scaling by sx and sy on the supplied transformation matrix.
 * The effect of the new transformation is to first scale the coordinates by sx and sy,
 * then apply the original transformation to the coordinates.
 * @param matrix a valid #vkvg_matrix_t pointer on which the scaling will be applied.
 * @param sx scale in the x direction
 * @param sy scale in the y direction
 */
vkvg_public
void vkvg_matrix_scale (vkvg_matrix_t *matrix, float sx, float sy);
/**
 * @brief apply rotation on matrix
 *
 * Apply a rotation of radians on the supplied matrix.
 * The effect of the new transformation is to first rotate the coordinates by radians,
 * then apply the original transformation to the coordinates.
 * @param matrix a valid #vkvg_matrix_t pointer on which the rotation will be applied
 * @param radians angle of rotation in radians. The direction of rotation is defined such that positive angles
 * rotate in the direction from the positive X axis toward the positive Y axis.
 * With the default axis orientation of cairo, positive angles rotate in a clockwise direction.
 */
vkvg_public
void vkvg_matrix_rotate (vkvg_matrix_t *matrix, float radians);
/**
 * @brief matrices multiplication
 *
 * compute the multiplication of two matrix.
 * @param result a valid #vkvg_matrix_t pointer to hold the resulting matrix
 * @param a first operand of the multiplication
 * @param b second operand of the multiplication
 */
vkvg_public
void vkvg_matrix_multiply (vkvg_matrix_t *result, const vkvg_matrix_t *a, const vkvg_matrix_t *b);
/**
 * @brief transform distances
 *
 * Transforms the distance vector (dx ,dy ) by matrix . This is similar to #cairo_matrix_transform_point() except that the translation
 * components of the transformation are ignored. The calculation of the returned vector is as follows:
 * @code
 * dx2 = dx1 * a + dy1 * c;
 * dy2 = dx1 * b + dy1 * d;
 * @endcode
 * Affine transformations are position invariant, so the same vector always transforms to the same vector. If (x1 ,y1 ) transforms to (x2 ,y2 )
 * then (x1 +dx1 ,y1 +dy1 ) will transform to (x1 +dx2 ,y1 +dy2 ) for all values of x1 and x2 .
 * @param matrix a valid #vkvg_matrix_t to use to transform distance
 * @param dx X component of a distance vector. An in/out parameter
 * @param dy Y component of a distance vector. An in/out parameter
 */
vkvg_public
void vkvg_matrix_transform_distance (const vkvg_matrix_t *matrix, float *dx, float *dy);
/**
 * @brief transform point
 *
 * Transforms the point (x , y ) by matrix .
 * @param matrix a valid #vkvg_matrix_t to use to transform point
 * @param x X position. An in/out parameter
 * @param y Y position. An in/out parameter
 */
vkvg_public
void vkvg_matrix_transform_point (const vkvg_matrix_t *matrix, float *x, float *y);
/**
 * @brief invert matrix
 *
 * Changes matrix to be the inverse of its original value. Not all transformation matrices have inverses;
 * if the matrix collapses points together (it is degenerate), then it has no inverse and this function will fail.
 * @param matrix the matrix to invert
 * @return If matrix has an inverse, modifies matrix to be the inverse matrix and returns VKVG_STATUS_SUCCESS.
 * Otherwise, returns VKVG_STATUS_INVALID_MATRIX.
 */
vkvg_public
vkvg_status_t vkvg_matrix_invert (vkvg_matrix_t *matrix);
vkvg_public
void vkvg_matrix_get_scale (const vkvg_matrix_t *matrix, float *sx, float *sy);
/** @}*/

/*!
 * @defgroup device Device
 * @brief create or use an existing vulkan context for vkvg.
 *
 * #VkvgDevice is the starting point of a vkvg rendering infrastructure. It connects an
 * existing vulkan context with vkvg, or may create a new one.
 *
 * Most of the vulkan rendering component (pipelines, renderpass, ..) are part of the VkvgDevice,
 * their are shared among drawing contexts.
 *
 * Antialiasing level is configured when creating the device by selecting the sample count.
 * @ref vkvg_device_create will create a non-antialiased dev by selecting VK_SAMPLE_COUNT_1_BIT as sample count.
 * To create antialiased rendering device, call @ref vkvg_device_create_multisample with VkSampleCountFlags
 * greater than one.
 *
 * vkvg use a single frame buffer format for now: VK_FORMAT_B8G8R8A8_UNORM.
 *
 * Device holds the font cache so that each time a context draws text, the same cache is used.
 *
 * @{ */

vkvg_public
void vkvg_device_set_thread_aware (VkvgDevice dev, uint32_t thread_awayre);

/**
 * @brief Create a new vkvg device.
 *
 * Create a new #VkvgDevice owning vulkan instance and device.
 *
 * On success, create a new vkvg device and set its reference count to 1.
 * On error, query the device status by calling @ref vkvg_device_status. Error could be
 * one of the following:
 * - VKVG_STATUS_INVALID_FORMAT: the combination of image format and tiling is not supported
 * - VKVG_STATUS_NULL_POINTER: vulkan function pointer fetching failed.
 *
 * @param samples The sample count that will be setup for the surfaces created by this device.
 * @param deferredResolve If true, the final simple sampled image of the surface will only be resolved on demand
 */
vkvg_public
VkvgDevice vkvg_device_create (VkSampleCountFlags samples, bool deferredResolve);
/**
 * @brief Create a new vkvg device from an existing vulkan logical device.
 *
 * Create a new #VkvgDevice connected to the vulkan context define by an instance,
 * a physical device, a logical device, a graphical queue family index and an its index.
 *
 * On success, create a new vkvg device and set its reference count to 1.
 * On error, query the device status by calling #vkvg_device_status(). Error could be
 * one of the following:
 * - VKVG_STATUS_INVALID_FORMAT: the combination of image format and tiling is not supported
 * - VKVG_STATUS_NULL_POINTER: vulkan function pointer fetching failed.
 *
 * @param inst a valid Vulkan instance to create the device from.
 * @param phy Vulkan physical device used to create the vkvg device.
 * @param vkdev Vulkan logical device to create the vkvg device for.
 * @param qFamIdx Queue family Index of the graphic queue used for drawing operations.
 * @param qIndex Index of the queue into the choosen familly, 0 in general.
 * @return The handle of the created vkvg device, or null if an error occured.
 */
vkvg_public
VkvgDevice vkvg_device_create_from_vk (VkInstance inst, VkPhysicalDevice phy, VkDevice vkdev, uint32_t qFamIdx, uint32_t qIndex);
/**
 * @brief Create a new multisampled vkvg device.
 *
 * This function allows to create vkvg device for working with multisampled surfaces.
 * Multisampling is used to smooth color transitions in drawings, making lines not pixelised and
 * diagonal edges not stepped. Multisampling has a performance cost.
 * The final image of the surface, accessible by the user will be a resolved single sampled image.
 *
 * @param inst Vulkan instance to create the device from.
 * @param phy Vulkan physical device used to create the vkvg device.
 * @param vkdev Vulkan logical device to create the vkvg device for.
 * @param qFamIdx Queue family Index of the graphic queue used for drawing operations.
 * @param qIndex Index of the queue into the choosen familly, 0 in general.
 * @param samples The sample count that will be setup for the surfaces created by this device.
 * @param deferredResolve If true, the final simple sampled image of the surface will only be resolved on demand
 * when calling @ref vkvg_surface_get_vk_image or by explicitly calling @ref vkvg_multisample_surface_resolve. If false, multisampled image is resolved on each draw operation.
 * @return The handle of the created vkvg device, or null if an error occured.
 */
vkvg_public
VkvgDevice vkvg_device_create_from_vk_multisample (VkInstance inst, VkPhysicalDevice phy, VkDevice vkdev, uint32_t qFamIdx, uint32_t qIndex, VkSampleCountFlags samples, bool deferredResolve);
/**
 * @brief Decrement the reference count of the device by 1. Release all it's ressources if count reach 0.
 *
 * If device is reference by several active surfaces, calling destroy will only decrement the reference count by 1.
 * When the reference count reach 0, the vkvg device is effectively destroyed, the device pointer is freed, and
 * vulkan objects are released.
 * Vkvg Devices has to stay active as long as surfaces created by it are used.
 * @param dev The vkvg device to destroy.
 */
vkvg_public
void vkvg_device_destroy (VkvgDevice dev);
/**
 * @brief Get the current status of the device.
 *
 * Query current status of device. See @ref vkvg_status_t for more informations.
 * @param dev a valid vkvg device pointer.
 * @return current state.
 */
vkvg_public
vkvg_status_t vkvg_device_status (VkvgDevice dev);
/**
 * @brief Increment the reference count on this device.
 *
 * Increment by one the reference count on the device.
 * @param The vkvg device pointer to increment the reference count for.
 * @return
 */
vkvg_public
VkvgDevice vkvg_device_reference (VkvgDevice dev);
/**
 * @brief Query the reference count of the device.
 *
 * Get the actual reference count on this device.
 * @param dev The vkvg device to get the reference count for.
 * @return The reference count on this device.
 */
vkvg_public
uint32_t vkvg_device_get_reference_count (VkvgDevice dev);
/**
 * @brief Set the screen dot per inch for this device.
 *
 * Set the current values for horizontal and vertical dot per inch of the screen for this device.
 * @param dev The vkvg device to set the dpy for.
 * @param hdpy Horizontal dot per inch.
 * @param vdpy Vertical dot per inch.
 */
vkvg_public
void vkvg_device_set_dpy (VkvgDevice dev, int hdpy, int vdpy);
/**
 * @brief Get the current dpy values.
 *
 * Get the current values for horizontal and vertical dot per inch of the screen for this device.
 * The default values for horizontal and vertical dpy on device creation is 96.
 * @param dev The vkvg device to get the dpy configuration for.
 * @param hdpy[out] The current horizontal dot per inch.
 * @param vdpy[out] The current vertical dot per inch.
 */
vkvg_public
void vkvg_device_get_dpy (VkvgDevice dev, int* hdpy, int* vdpy);

/**
 * @brief query required instance extensions for vkvg.
 *
 * @param pExtensions a valid pointer to the array of extension names to fill, the size may be queried
 * by calling this method with pExtension being a NULL pointer.
 * @param pExtCount a valid pointer to an integer that will be fill with the required extension count.
 */
vkvg_public
void vkvg_get_required_instance_extensions (const char** pExtensions, uint32_t* pExtCount);
/**
 * @brief query required device extensions for vkvg.
 * @param phy the vulkan physical device that will be used to create the @ref VkvgDevice.
 * @param pExtensions a valid pointer to the array of extension names to fill, the size may be queried
 * by calling this method with pExtension being a NULL pointer.
 * @param pExtCount a valid pointer to an integer that will be fill with the required extension count.
 */
vkvg_public
void vkvg_get_required_device_extensions (VkPhysicalDevice phy, const char** pExtensions, uint32_t* pExtCount);
/**
 * @brief get vulkan device creation requirement to fit vkvg needs.
 *
 * @param pEnabledFeatures a pointer to the feature structure to fill for the vulkan device creation.
 * @return the required pNext chain for the vulkan device creation. The first structure is guarantied to
 * be VkPhysicalDeviceVulkan12Features if vulkan version is >= 1.2
 */
vkvg_public
const void* vkvg_get_device_requirements (VkPhysicalDeviceFeatures* pEnabledFeatures);
/** @}*/

/** @addtogroup surface
 * @{ */
/**
 * @brief Create a new vkvg surface.
 * @param dev The vkvg device used for creating the surface.
 * @param width Width in pixel of the surface to create.
 * @param height Height in pixel of the surface to create.
 * @return The new vkvg surface pointer, or null if an error occured.
 */
vkvg_public
VkvgSurface vkvg_surface_create (VkvgDevice dev, uint32_t width, uint32_t height);
/**
 * @brief Create a new vkvg surface by loading an image file.
 * The resulting surface will have the same dimension as the supplied image.
 *
 * @param dev The vkvg device used for creating the surface.
 * @param filePath The path of the image to load for creating the surface.
 * @return The new vkvg surface with the loaded image as content, or null if an error occured.
 */
vkvg_public
VkvgSurface vkvg_surface_create_from_image (VkvgDevice dev, const char* filePath);
/**
 * @brief Create a new vkvg surface using an existing vulkan texture as backend.
 * @param dev The vkvg device used for creating the surface.
 * @param vkhImg The VkhImage to use as the backend texture for drawing operations.
 * @return A new surface, or null if an error occured.
 */
vkvg_public
VkvgSurface vkvg_surface_create_for_VkhImage (VkvgDevice dev, void* vkhImg);
// VkvgSurface vkvg_surface_create_from_bitmap (VkvgDevice dev, unsigned char* img, uint32_t width, uint32_t height);
/**
 * @brief Increment reference count on the surface by one.
 * @param The vkvg surface to increment the reference count for.
 * @return ?
 */
vkvg_public
VkvgSurface vkvg_surface_reference (VkvgSurface surf);
/**
 * @brief Get the current reference count on this surface.
 * @param The vkvg surface to get the reference count for.
 * @return The reference count on the surface.
 */
vkvg_public
uint32_t vkvg_surface_get_reference_count (VkvgSurface surf);
/**
 * @brief Decrement the reference count on the surface by one. Destroy it if count reach 0.
 * @param The vkvg surface to destroy.
 */
vkvg_public
void vkvg_surface_destroy (VkvgSurface surf);
/**
 * @brief Query the current status of the surface
 * @param The vkvg surface to query the status for.
 * @return The current surface status.
 */
vkvg_public
vkvg_status_t vkvg_surface_status (VkvgSurface surf);
/**
 * @brief Clear the surface content, alpha is also set to 0 resulting in a transparent image.
 *
 * @remark Internaly, the vulkan method used to clear the surface is the slowest, prefer using the @ref vkvg_clear
 * function of the context that will try to use the render pass load operations when possible.
 * @param The surface to clear.
 */
vkvg_public
void vkvg_surface_clear (VkvgSurface surf);
/**
 * @brief Get the final single sampled vulkan image of this surface.
 * @param The vkvg surface to get the vulkan texture of.
 * @return The VkImage object containing the result of the drawing operations on the surface.
 */
vkvg_public
VkImage	vkvg_surface_get_vk_image (VkvgSurface surf);
/**
 * @brief Get the vulkan format of the vulkan texture used as backend for this surface.
 * @param The surface to get the format for.
 * @return The VkFormat.
 */
vkvg_public
VkFormat vkvg_surface_get_vk_format (VkvgSurface surf);
/**
 * @brief Get the actual surface width.
 * @param The vkvg surface to get the width for.
 * @return The width in pixel of the surface.
 */
vkvg_public
uint32_t vkvg_surface_get_width (VkvgSurface surf);
/**
 * @brief Get the actual surface height.
 * @param The vkvg surface to get the height for.
 * @return The height in pixel of the surface.
 */
vkvg_public
uint32_t vkvg_surface_get_height (VkvgSurface surf);
/**
 * @brief Write surface content to a png file on disk.
 * @param The surface to save on disk.
 * @param The png file path.
 * @return SUCCESS or not.
 */
vkvg_public
vkvg_status_t vkvg_surface_write_to_png (VkvgSurface surf, const char* path);
/**
 * @brief Save surface to memory
 * @param The surface to save
 * @param A valid pointer on cpu memory large enough to contain surface pixels (stride * height)
 * @return SUCCESS or not.
 */
vkvg_public
vkvg_status_t vkvg_surface_write_to_memory (VkvgSurface surf, unsigned char* const bitmap);
/**
 * @brief Explicitly resolve a multisampled surface.
 *
 * When creating a multisampled vkvg device with deffered resolve set to true, this function has to be
 * called to have the multisampled texture used for drawing operations resolved on the single sampled accessible
 * texture. Note that a call to @ref vkvg_surface_get_vk_image will automaticaly call this method before returning
 * the backend image.
 *
 * @param The vkvg surface to resolve.
 */
vkvg_public
void vkvg_multisample_surface_resolve (VkvgSurface surf);
/** @}*/

//mimic from cairo, to facilitate usage of vkvg as cairo vulkan backend

/**
 * @brief compositing operators
 *
 * define the operation used to draw
 */
typedef enum _vkvg_operator {
	VKVG_OPERATOR_CLEAR,

	VKVG_OPERATOR_SOURCE,
	VKVG_OPERATOR_OVER,
/*	VKVG_OPERATOR_IN,
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
	*/VKVG_OPERATOR_DIFFERENCE,/*
	VKVG_OPERATOR_EXCLUSION,
	VKVG_OPERATOR_HSL_HUE,
	VKVG_OPERATOR_HSL_SATURATION,
	VKVG_OPERATOR_HSL_COLOR,
	VKVG_OPERATOR_HSL_LUMINOSITY,*/
	VKVG_OPERATOR_MAX,
} vkvg_operator_t;

/** @addtogroup context
 * @{ */

/**
 * @brief Create a new vkvg context used for drawing on surfaces.
 * @param surf The target surface of the drawing operations.
 * @return A new #VkvgContext or null if an error occured.
 */
vkvg_public
VkvgContext vkvg_create (VkvgSurface surf);
/**
 * @brief Decrement the reference count on the context by one. Destroy it if count reach 0.
 * @param ctx The vkvg context to destroy.
 */
vkvg_public
void vkvg_destroy (VkvgContext ctx);
/**
 * @brief Get context status
 *
 * Get the current context status.
 *
 * @param ctx The vkvg context to query the status for.
 */
vkvg_public
vkvg_status_t vkvg_status (VkvgContext ctx);
/**
 * vkvg_status_to_string:
 * @status: a vkvg status
 *
 * Provides a human-readable description of a #vkvg_status_t.
 *
 * Returns: a string representation of the status
 **/
vkvg_public
const char *vkvg_status_to_string (vkvg_status_t status);
/**
 * @brief Increment by one the reference count on this context.
 * @param ctx The context to increment the reference count for.
 * @return
 */
vkvg_public
VkvgContext vkvg_reference (VkvgContext ctx);
/**
 * @brief Get the current reference count of this context.
 * @param The vkvg context to query.
 * @return The current reference count for this context.
 */
vkvg_public
uint32_t vkvg_get_reference_count (VkvgContext ctx);
/**
 * @brief Perform all the pending drawing operations on a context.
 *
 * Drawing operations by a context on a surface are delayed in several cases
 * for optimisation purpose. Calling #vkvg_flush on a context ensure that
 * all possibly delayed drawing command are executed.
 * The context is automatically flushed on destruction.
 * @param ctx The vkvg context to flush.
 */
vkvg_public
void vkvg_flush (VkvgContext ctx);
/**
 * @brief Start a new empty path.
 *
 * Calling #vkvg_new_path is equivalent to a clear of the current path stored in the context
 * without drawing anything on the target surface.
 * The current position is reseted to (0,0).
 * @param ctx The vkvg context pointer.
 */
vkvg_public
void vkvg_new_path (VkvgContext ctx);
/**
 * @brief Close the current path.
 *
 * Close the last path emited for this context if its point count is greater than 2.
 * This function has no effect if the current path is empty or if its point count is less
 * than 2.
 * This function is automatically call when you try to perform a filling operation on an unclosed path.
 * @param ctx The vkvg context pointer.
 */
vkvg_public
void vkvg_close_path (VkvgContext ctx);
/**
 * @brief Start new sub path, no current point is defined
 *
 * If the current path is empty, this function has no effect.
 * @param ctx The vkvg context pointer.
 */
vkvg_public
void vkvg_new_sub_path (VkvgContext ctx);
/**
 * @brief vkvg_path_extents
 * @param ctx a valid @ref context
 * @param x1 left of the resulting extents
 * @param y1 top of the resulting extents
 * @param x2 right of the resulting extents
 * @param y2 bottom of the resulting extents
 */
vkvg_public
void vkvg_path_extents (VkvgContext ctx, float *x1, float *y1, float *x2, float *y2);
/**
 * @brief Get the current point of the context, return 0,0 if no point is defined.
 * @param ctx
 * @param x
 * @param y
 */
vkvg_public
void vkvg_get_current_point (VkvgContext ctx, float* x, float* y);
/**
 * @brief Add a line to the current path from the current point to the coordinate given in arguments.
 *
 * After this call, the current position will be (x,y).
 * If no current point is defined, this function call is equivalent to a
 * single #vkvg_move_to command.
 * @param ctx The vkvg context pointer.
 * @param x absolute x coordinate of second point
 * @param y aboslute y coordinate of second point
 */
vkvg_public
void vkvg_line_to (VkvgContext ctx, float x, float y);
/**
 * @brief Add a line to the current path from the current point to the coordinate relative to it.
 *
 * Given a current point of (x, y), after this call the current point will be  (x + dx, y + dy).
 * If no current point is defined, this function call is equivalent to a
 * single #move_to command to the absolute position given by the x and y arguments.
 * @param ctx The vkvg context pointer.
 * @param dx delta x
 * @param dy delta y
 */
vkvg_public
void vkvg_rel_line_to (VkvgContext ctx, float dx, float dy);
/**
 * @brief Move the context pen to the position given in argument.
 *
 * This command set the current pen position of the context to the coordinate
 * given by the x and y arguments.
 * If the current path is not empty, this command will start a new
 * subpath and set the current pen position to the coordinates given in
 * arguments.
 * @param ctx The vkvg context pointer.
 * @param x new x position of the pen
 * @param y new y position of the pen
 */
vkvg_public
void vkvg_move_to (VkvgContext ctx, float x, float y);
/**
 * @brief Move the context pen relative to the current point.
 *
 * This command will set the current pen position of the context to the
 * coordinate given by the x and y deltas in arguments.
 * If the current path is not empty, this command will start a new
 * subpath and set the current pen position.
 * @param ctx The vkvg context pointer.
 * @param x delta in the horizontal direction.
 * @param y delta in the vertical direction.
 */
vkvg_public
void vkvg_rel_move_to (VkvgContext ctx, float x, float y);
/**
 * @brief Adds a circular arc of the given radius to the current path.
 *
 * Adds a circular arc in clockwise order of the given radius to the current path following angles of a trigonometric circle.
 * After this call the current point will be the last computed point of the arc.
 * The arc is centered at (xc , yc ), begins at angle a1 and proceeds in the direction of increasing angles to end at angle a2.
 * If a2 is less than a1, it will be progressively increased by 2*PI until it is greater than a1.
 *
 * If there is a current point, an initial line segment will be added to the path to connect the current point to the beginning of the arc.
 * If this initial line is undesired, it can be avoided by calling vkvg_new_sub_path() before calling vkvg_arc().
 *
 * Angles are measured in radians. An angle of 0.0 is in the direction of the positive X axis.
 * An angle of PI/2 radians (90 degrees) is in the direction of the positive Y axis. Angles increase in the direction
 * from the positive X axis toward the positive Y axis. So with the default transformation matrix, angles increase in a clockwise direction.
 *
 * @remark To convert from degrees to radians, use degrees * (PI/180).
 * @param ctx The vkvg context pointer.
 * @param xc center x coordinate
 * @param xy center y coordinate
 * @param radius The radius of the arc.
 * @param a1 start angle in radians of the arc as if on a trigonometric circle.
 * @param a2 end angle in radians of the arc to draw.
 */
vkvg_public
void vkvg_arc (VkvgContext ctx, float xc, float yc, float radius, float a1, float a2);
/**
 * @brief Add a circular arc in counter clockwise order to the current path.
 *
 * Adds a circular arc in counter clockwise order of the given radius to the current path following angles of a trigonometric circle.
 * After this call the current point will be the last computed point of the arc.
 * The arc is centered at (xc , yc ), begins at angle a1 and proceeds in the direction of decreasing angles to end at angle a2.
 * If a2 is greater than a1, it will be progressively decreased by 2*PI until it is less than a1.
 *
 * See @ref vkvg_arc for more details. This function differs only in the direction of the arc between the two angles.
 * @param ctx The vkvg context pointer.
 * @param xc center x coordinate
 * @param xy center y coordinate
 * @param radius The radius of the arc.
 * @param a1 start angle in radians of the arc as if on a trigonometric circle.
 * @param a2 end angle in radians of the arc to draw.
 */
vkvg_public
void vkvg_arc_negative (VkvgContext ctx, float xc, float yc, float radius, float a1, float a2);
/**
 * @brief Adds a cubic Bézier spline to the current path.
 *
 * Adds a cubic Bézier spline to the path from the current point to position (x3, y3), using (x1, y1) and (x2, y2) as the control points.
 * After this call the current point will be (x3, y3).
 *
 * If there is no current point before the call to vkvg_curve_to() this function will behave as if preceded by a call to vkvg_move_to(ctx, x1, y1).
 * @param ctx The vkvg context pointer.
 * @param x1 The X coordinate of the first control point.
 * @param y1 The Y coordinate of the first control point.
 * @param x2 The X coordinate of the second control point.
 * @param y2 The Y coordinate of the second control point.
 * @param x3 The X coordinate of the end of the curve.
 * @param y3 The Y coordinate of the end of the curve.
 */
vkvg_public
void vkvg_curve_to (VkvgContext ctx, float x1, float y1, float x2, float y2, float x3, float y3);
/**
 * @brief Adds a cubic Bézier spline to the current path relative to the current point.
 *
 * Adds a cubic Bézier spline to the path from the current point to position (x3, y3) in relative coordinate to the current point,
 * using (x1, y1) and (x2, y2) as the control points relative to the current point.
 * After this call the current point will be (x3, y3).
 *
 * If there is no current point before the call to vkvg_rel_curve_to() => error:VKVG_STATUS_NO_CURRENT_POINT.
 * @param ctx The vkvg context pointer.
 * @param x1 The X coordinate of the first control point.
 * @param y1 The Y coordinate of the first control point.
 * @param x2 The X coordinate of the second control point.
 * @param y2 The Y coordinate of the second control point.
 * @param x3 The X coordinate of the end of the curve.
 * @param y3 The Y coordinate of the end of the curve.
 */
vkvg_public
void vkvg_rel_curve_to (VkvgContext ctx, float x1, float y1, float x2, float y2, float x3, float y3);
/**
 * @brief Add a quadratic Bezizer curve to the current path
 *
 * If there is no current point before the call to vkvg_quadratic_to() this function will behave as if preceded by a call to vkvg_move_to(ctx, x1, y1).
 * @param ctx The vkvg context pointer.
 * @param x1 The X coordinate of the control point.
 * @param y1 The Y coordinate of the control point.
 * @param x2 The X coordinate of the end point of the curve.
 * @param y2 The Y coordinate of the end point of the curve.
 */
vkvg_public
void vkvg_quadratic_to (VkvgContext ctx, float x1, float y1, float x2, float y2);
/**
 * @brief Add a quadratic Bezizer curve to the current path relative to the current point
 *
 * @param ctx The vkvg context pointer.
 * @param x1 The X coordinate of the control point relative to the current point.
 * @param y1 The Y coordinate of the control point relative to the current point.
 * @param x2 The X coordinate of the end point of the curve relative to the current point.
 * @param y2 The Y coordinate of the end point of the curve relative to the current point.
 */
vkvg_public
void vkvg_rel_quadratic_to (VkvgContext ctx, float x1, float y1, float x2, float y2);
/**
 * @brief Add an axis aligned rectangle subpath to the current path.
 *
 * Adds a closed sub-path rectangle of the given size to the current path at position (x, y).
 * @param ctx The vkvg context pointer.
 * @param x The x coordinate of the top left corner of the rectangle to emit.
 * @param y The y coordinate of the top left corner of the rectangle to emit.
 * @param w The width in pixel of the rectangle to draw.
 * @param h The height in pixel of the rectangle to draw.
 * @return VKVG_STATUS_SUCCESS or VKVG_STATUS_INVALID_RECT if width or height is equal to 0.
 */
vkvg_public
vkvg_status_t vkvg_rectangle(VkvgContext ctx, float x, float y, float w, float h);
/**
* @brief Add an axis aligned rectangle with rounded corners to the current path.
*
* Adds a closed sub-path rectangle of the given size to the current path at position (x, y).
* @param ctx The vkvg context pointer.
* @param x The x coordinate of the top left corner of the rectangle to emit.
* @param y The y coordinate of the top left corner of the rectangle to emit.
* @param w The width in pixel of the rectangle to draw.
* @param h The height in pixel of the rectangle to draw.
* @param radius The radius of the corners.
* @return VKVG_STATUS_SUCCESS or VKVG_STATUS_INVALID_RECT if width or height is equal to 0.
*/
vkvg_public
vkvg_status_t vkvg_rounded_rectangle (VkvgContext ctx, float x, float y, float w, float h, float radius);
/**
* @brief Add an axis aligned rectangle with rounded corners defined in both axis to the current path.
*
* Adds a closed sub-path rectangle of the given size to the current path at position (x, y).
* @param ctx The vkvg context pointer.
* @param x The x coordinate of the top left corner of the rectangle to emit.
* @param y The y coordinate of the top left corner of the rectangle to emit.
* @param w The width in pixel of the rectangle to draw.
* @param h The height in pixel of the rectangle to draw.
* @param rx The horizontal radius of the corners.
* @param ry The vertical radius of the corners.
* @return VKVG_STATUS_SUCCESS or VKVG_STATUS_INVALID_RECT if width or height is equal to 0.
*/
vkvg_public
void vkvg_rounded_rectangle2 (VkvgContext ctx, float x, float y, float w, float h, float rx, float ry);
vkvg_public
void vkvg_ellipse (VkvgContext ctx, float radiusX, float radiusY, float x, float y, float rotationAngle);
vkvg_public
void vkvg_elliptic_arc_to (VkvgContext ctx, float x2, float y2, bool largeArc, bool counterClockWise, float rx, float ry, float phi);
vkvg_public
void vkvg_rel_elliptic_arc_to (VkvgContext ctx, float x2, float y2, bool largeArc, bool counterClockWise, float rx, float ry, float phi);
/**
 * @brief Stroke command
 *
 * Perform a stroke of the current path and reset it.
 *
 * @param ctx a valid vkvg @ref context
 */
vkvg_public
void vkvg_stroke (VkvgContext ctx);
/**
 * @brief Stroke command that preserve current path.
 *
 * Perform a stroke of the current path and preserve it after the operation.
 *
 * @param ctx a valid vkvg @ref context
 */
vkvg_public
void vkvg_stroke_preserve (VkvgContext ctx);
/**
 * @brief Fill command
 *
 * Perform the filling of the current path. If no path is defined, this command has no effect.
 * The current path is reseted after this operation.
 *
 * @param ctx a valid vkvg @ref context
 */
vkvg_public
void vkvg_fill (VkvgContext ctx);
/**
 * @brief Fill command that preserve current path.
 *
 * Same as @ref vkvg_fill, but don't reset the current path after the operation.
 *
 * @param ctx a valid vkvg @ref context
 */
vkvg_public
void vkvg_fill_preserve (VkvgContext ctx);
/**
 * @brief Paint command.
 *
 * perform a fill operation on the current path or on the full surface if no path is defined with
 * the currently active pattern.
 *
 * @param ctx a valid vkvg @ref context
 */
vkvg_public
void vkvg_paint (VkvgContext ctx);
/**
 * @brief clear surface
 *
 * Color and clipping are reset to 0.
 *
 * @param ctx a valid vkvg @ref context
 */
vkvg_public
void vkvg_clear (VkvgContext ctx);//use vkClearAttachment to speed up clearing surf
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 */
vkvg_public
void vkvg_reset_clip (VkvgContext ctx);
/**
 * @brief reset clip
 *
 * Reset current context clip regions.
 * @param ctx a valid vkvg @ref context
 */
vkvg_public
void vkvg_clip (VkvgContext ctx);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 */
vkvg_public
void vkvg_clip_preserve (VkvgContext ctx);
vkvg_public
void vkvg_set_opacity (VkvgContext ctx, float opacity);
vkvg_public
float vkvg_get_opacity (VkvgContext ctx);
/**
 * @brief Set current source for drawing to the solid color defined by the supplied 32bit integer.
 * @param ctx a valid vkvg @ref context
 * @param rgba color coded in 32bit integer.
 */
vkvg_public
void vkvg_set_source_color (VkvgContext ctx, uint32_t c);
/**
 * @brief set color with alpha.
 *
 * Set current source for drawing to the solid color defined by the rgba components with 'a' for transparency.
 * @param ctx a valid vkvg @ref context
 * @param r the red component of the color.
 * @param g the green component of the color.
 * @param b the blue component of the color.
 * @param a the alpha component holding the transparency for the current color.
 */
vkvg_public
void vkvg_set_source_rgba (VkvgContext ctx, float r, float g, float b, float a);
/**
 * @brief set color as new source.
 *
 * Set current source to the solid color defined by the rgb components.
 *
 * @param ctx a valid vkvg @ref context
 * @param r the red component of the color.
 * @param g the green component of the color.
 * @param b the blue component of the color.
 * @param a the alpha component holding the transparency for the current color.
 */
vkvg_public
void vkvg_set_source_rgb (VkvgContext ctx, float r, float g, float b);
/**
 * @brief set line width for the next draw command.
 *
 * Set the current line width for the targeted context. All further calls to #vkvg_stroke on this context
 * will use this new width.
 *
 * @param ctx a valid vkvg @ref context
 * @param width new current line width for the context.
 */
vkvg_public
void vkvg_set_line_width (VkvgContext ctx, float width);
/**
 * @brief set line join miter size limit.
 *
 * If the current line join style is set to VKVG_LINE_JOIN_MITER (see vkvg_set_line_join()), the miter limit is used to determine whether the lines should be
 * joined with a bevel instead of a miter. Vkvg divides the length of the miter by the line width. If the result is greater than the miter limit, the style is converted to a bevel.
 *
 * The default miter limit value is 10.0, which will convert joins with interior angles less than 11 degrees to bevels instead of miters.
 * For reference, a miter limit of 2.0 makes the miter cutoff at 60 degrees, and a miter limit of 1.414 makes the cutoff at 90 degrees.
 *
 * A miter limit for a desired angle can be computed as: miter limit = 1/sin(angle/2)
 *
 * @param ctx a valid vkvg @ref context
 * @param limit new current miter limit value for the context.
 */
vkvg_public
void vkvg_set_miter_limit (VkvgContext ctx, float limit);
/**
 * @brief Gets the current miter limit, as set by @ref vkvg_set_miter_limit().
 * @param ctx a valid vkvg @ref context
 * @return the current miter limit for the context.
 */
vkvg_public
float vkvg_get_miter_limit (VkvgContext ctx);
/**
 * @brief set line terminations for the next draw command.
 *
 * Configure the line terminations to output for further path stroke commands.
 * @param ctx a valid vkvg @ref context
 * @param cap new line termination, may be one of the value of #vkvg_line_cap_t.
 */
vkvg_public
void vkvg_set_line_cap (VkvgContext ctx, vkvg_line_cap_t cap);
/**
 * @brief set line joins for the next draw command.
 *
 * Configure the line join to output for further path stroke commands.
 * @param ctx a valid vkvg @ref context
 * @param join new line join as defined in #vkvg_line_joint_t.
 */
vkvg_public
void vkvg_set_line_join (VkvgContext ctx, vkvg_line_join_t join);
/**
 * @brief use supplied surface as current pattern.
 *
 * set #VkvgSurface as the current context source.
 * @param ctx a valid vkvg @ref context
 * @param surf the vkvg surface to use as source.
 * @param x an x offset to apply for drawing operations using this surface.
 * @param y an y offset to apply for drawing operations using this surface.
 */
vkvg_public
void vkvg_set_source_surface (VkvgContext ctx, VkvgSurface surf, float x, float y);
/**
 * @brief set supplied pattern as current source.
 *
 * set #VkvgPattern as the new source for the targeted context.
 * @param ctx a valid vkvg @ref context
 * @param pat the new pattern to use as source for further drawing operations.
 */
vkvg_public
void vkvg_set_source (VkvgContext ctx, VkvgPattern pat);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @param op
 */
vkvg_public
void vkvg_set_operator (VkvgContext ctx, vkvg_operator_t op);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @param fr
 */
vkvg_public
void vkvg_set_fill_rule (VkvgContext ctx, vkvg_fill_rule_t fr);
/**
 * @brief set the dash configuration for strokes
 *
 * Sets the dash pattern to be used by the next #vkvg_stroke().
 * A dash pattern is specified by dashes, an array of positive values.
 * Each value provides the length of alternate "on" and "off" portions of the stroke.
 * The offset specifies an offset into the pattern at which the stroke begins.
 * @param ctx a valid vkvg @ref context
 * @param dashes a pointer on an array of float values defining alternate lengths of on and off stroke portions.
 * @param num_dashes the length of the dash array.
 * @param offset an offset into the dash pattern at which the stroke should start.
 */
vkvg_public
void vkvg_set_dash (VkvgContext ctx, const float* dashes, uint32_t num_dashes, float offset);
/**
 * @brief get current dash settings.
 *
 * Get the current dash configuration for the supplied @ref context.
 * If dashes pointer is NULL, only count and offset are returned, useful to query dash array dimension first.
 * @param ctx a valid vkvg @ref context
 * @param dashes[out] return value for the dash array. If count is 0, this pointer stay untouched.
 * If NULL, only count and offset are returned.
 * @param num_dashes[out] return length of dash array or 0 if dash is not set.
 * @param offset[out] return value for the current dash offset
 */
vkvg_public
void vkvg_get_dash (VkvgContext ctx, const float *dashes, uint32_t* num_dashes, float* offset);

/**
 * @brief get current line width
 *
 * This function return the current line width to use by vkvg_stroke() as set by #vkvg_set_line_width().
 * @param ctx a valid vkvg @ref context
 * @return current line width.
 */
vkvg_public
float vkvg_get_line_width (VkvgContext ctx);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @return vkvg_line_cap_t
 */
vkvg_public
vkvg_line_cap_t vkvg_get_line_cap (VkvgContext ctx);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @return vkvg_line_join_t
 */
vkvg_public
vkvg_line_join_t vkvg_get_line_join (VkvgContext ctx);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @return vkvg_operator_t
 */
vkvg_public
vkvg_operator_t vkvg_get_operator (VkvgContext ctx);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @return vkvg_fill_rule_t
 */
vkvg_public
vkvg_fill_rule_t vkvg_get_fill_rule (VkvgContext ctx);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @return VkvgPattern
 */
vkvg_public
VkvgPattern vkvg_get_source (VkvgContext ctx);

vkvg_public
VkvgSurface vkvg_get_target (VkvgContext ctx);

/**
 * @brief Returns whether a current point is defined on the current path.
 * See @ref vkvg_get_current_point() for details on the current point.
 *
 * @param ctx a valig vkvg @ref context
 * @return bool whether a current point is defined
 **/
vkvg_public
bool vkvg_has_current_point (VkvgContext ctx);

/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 */
vkvg_public
void vkvg_save (VkvgContext ctx);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 */
vkvg_public
void vkvg_restore (VkvgContext ctx);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @param dx
 * @param dy
 */
vkvg_public
void vkvg_translate (VkvgContext ctx, float dx, float dy);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @param sx
 * @param sy
 */
vkvg_public
void vkvg_scale (VkvgContext ctx, float sx, float sy);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @param radians
 */
vkvg_public
void vkvg_rotate (VkvgContext ctx, float radians);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @param matrix
 */
vkvg_public
void vkvg_transform (VkvgContext ctx, const vkvg_matrix_t* matrix);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @param matrix
 */
vkvg_public
void vkvg_set_matrix (VkvgContext ctx, const vkvg_matrix_t* matrix);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @param matrix
 */
vkvg_public
void vkvg_get_matrix (VkvgContext ctx, const vkvg_matrix_t* matrix);
/**
 * @brief Reset the current transformation matrix of the provided context to the identity matrix.
 *
 * @param ctx a valid vkvg @ref context
 */
vkvg_public
void vkvg_identity_matrix (VkvgContext ctx);

/**
 * @brief Try find font with the specified name using the FontConfig library.
 *
 * @param ctx a valid vkvg @ref context
 * @param name A name to be recognized by the FontConfig library
 */
vkvg_public
void vkvg_select_font_face (VkvgContext ctx, const char* name);
/**
 * @brief Select a new font by providing its file path.
 *
 * @param ctx a valid vkvg @ref context
 * @param path A valid font file path.
 * @param name A short name to select this font afteward
 */
vkvg_public
void vkvg_load_font_from_path (VkvgContext ctx, const char* path, const char *name);
/**
 * @brief Select a new font by providing a pointer on the font file loaded in memory and its size in byte.
 *
 * @param ctx a valid vkvg @ref context
 * @param fontBuffer a pointer to a raw font file loaded in memory.
 * @param fontBufferByteSize the size of the font buffer in bytes.
 * @param name A short name to select this font afteward
 */
vkvg_public
void vkvg_load_font_from_memory (VkvgContext ctx, unsigned char* fontBuffer, long fontBufferByteSize, const char* name);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @param size
 */
vkvg_public
void vkvg_set_font_size (VkvgContext ctx, uint32_t size);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @param text
 */
vkvg_public
void vkvg_show_text (VkvgContext ctx, const char* text);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @param text
 * @param extents
 */
vkvg_public
void vkvg_text_extents (VkvgContext ctx, const char* text, vkvg_text_extents_t* extents);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @param extents
 */
vkvg_public
void vkvg_font_extents (VkvgContext ctx, vkvg_font_extents_t* extents);

//text run holds harfbuz datas, and prevent recreating them multiple times for the same line of text.
/**
 * @brief Create a new text run.
 *
 * @param ctx a valid vkvg @ref context
 * @param text Null terminated utf8 string.
 * @return VkvgText
 */
vkvg_public
VkvgText vkvg_text_run_create (VkvgContext ctx, const char* text);
/**
 * @brief Create a new text run for a non null terminated string.
 *
 * @param ctx a valid vkvg @ref context
 * @param text non null terminated utf8 string.
 * @param length glyphs count, not to be confused with byte length.
 * @return VkvgText
 */
vkvg_public
VkvgText vkvg_text_run_create_with_length (VkvgContext ctx, const char* text, uint32_t length);
/**
 * @brief Release ressources holded by the text run.
 *
 * @param VkvgtextRun A valid VkvgText pointer.
 */
vkvg_public
void vkvg_text_run_destroy (VkvgText textRun);
/**
 * @brief
 *
 * @param ctx a valid vkvg @ref context
 * @param textRun
 */
vkvg_public
void vkvg_show_text_run (VkvgContext ctx, VkvgText textRun);
/**
 * @brief
 *
 * @param textRun
 * @param extents
 */
vkvg_public
void vkvg_text_run_get_extents (VkvgText textRun, vkvg_text_extents_t* extents);
/**
 * @brief Get glyph count of text run.
 *
 * @return glyph count
 */
vkvg_public
uint32_t vkvg_text_run_get_glyph_count (VkvgText textRun);
/**
 * @brief retrieve glyph positions.
 *
 */
vkvg_public
void vkvg_text_run_get_glyph_position (VkvgText textRun,
									   uint32_t index,
									   vkvg_glyph_info_t* pGlyphInfo);
/** @}*/

/**
 * @defgroup pattern Pattern
 * @brief special sources for drawing
 *
 * A Pattern is a special source for drawing operations that can be an image, a gradient
 * and which may have special configuration for filtering and border repeat.
 *
 * @{ */

/**
 * @brief add reference
 *
 * increment reference count by one for the supplied #VkvgPattern.
 * @param pat a valid #VkvgPattern pointer
 * @return VkvgPattern
 */
vkvg_public
VkvgPattern vkvg_pattern_reference (VkvgPattern pat);
/**
 * @brief get reference count
 *
 * return the current reference count for the supplied #VkvgPattern
 * @param pat a valid #VkvgPattern to query for its reference count
 * @return uint32_t the current reference count for this instance.
 */
vkvg_public
uint32_t vkvg_pattern_get_reference_count (VkvgPattern pat);
/**
 * @brief create a surface pattern
 *
 * Create a new pattern from the supplied #surface. The advantage of having
 * a #VkvgPattern to paint an image resides in the hability to set filtering options
 * as well as repeat behaviour on borders. Reference count of the supplied surface will
 * be incremented so that it will not be destroyed before this pattern release it.
 * @param surf a valid #VkvgSurface to use for pattern
 * @return VkvgPattern a newly created pattern
 */
vkvg_public
VkvgPattern vkvg_pattern_create_for_surface (VkvgSurface surf);
/**
 * @brief create a new linear gradient.
 *
 * Create a new linear gradient along the line defined by (x0, y0) and (x1, y1).
 * Before using the gradient pattern, a number of color stops should be defined using @ref vkvg_pattern_add_color_stop.
 *
 * @param x0 x coordinate of the start point
 * @param y0 y coordinate of the start point
 * @param x1 x coordinate of the end point
 * @param y1 y coordinate of the end point
 * @return VkvgPattern the newly created pattern, call @ref vkvg_pattern_destroy when finished with it.
 */
vkvg_public
VkvgPattern vkvg_pattern_create_linear (float x0, float y0, float x1, float y1);
/**
 * @brief edit an existing linear gradient.
 *
 * edit control points of an existing linear gradient.
 *
 * @param x0 x coordinate of the start point
 * @param y0 y coordinate of the start point
 * @param x1 x coordinate of the end point
 * @param y1 y coordinate of the end point
 * @return VKVG_STATUS_SUCCESS, or VKVG_STATUS_PATTERN_TYPE_MISMATCH if the pattern is not a linear gradient.
 */
vkvg_public
vkvg_status_t vkvg_pattern_edit_linear(VkvgPattern pat, float x0, float y0, float x1, float y1);
/**
 * @brief get the gradient end points for a linear gradient
 *
 * If supplied pattern is not a linear gradient, @ref VKVG_STATUS_PATTERN_TYPE_MISMATCH is set for pattern.
 *
 * @param x0 x coordinate of the start point
 * @param y0 y coordinate of the start point
 * @param x1 x coordinate of the end point
 * @param y1 y coordinate of the end point
 * @return VKVG_STATUS_SUCCESS, or VKVG_STATUS_PATTERN_TYPE_MISMATCH if the pattern is not a linear gradient.
 */
vkvg_public
vkvg_status_t vkvg_pattern_get_linear_points(VkvgPattern pat, float* x0, float* y0, float* x1, float* y1);
/**
 * @brief create a new radial gradient.
 *
 * Creates a new radial gradient between the two circles defined by (cx0, cy0, radius0) and (cx1, cy1, radius1).
 * Before using the gradient pattern, a number of color stops should be defined using vkvg_pattern_add_color_stop.
 *
 * @param cx0 x coordinate for the center of the start circle, the inner circle. Must stand inside outer circle.
 * @param cy0 y coordinate for the center of the start circle, the inner circle. Must stand inside outer circle.
 * @param radius0 radius for the center of the start circle, the inner circle. Can't be greater than radius1
 * @param cx1 x coordinate for the center of the end circle, the outer circle.
 * @param cy1 y coordinate for the center of the end circle, the outer circle.
 * @param radius1 radius for the center of the end circle, the outer circle.
 * @return VkvgPattern the newly created pattern to be disposed when finished by calling @ref vkvg_pattern_destroy.
 */
vkvg_public
VkvgPattern vkvg_pattern_create_radial (float cx0, float cy0, float radius0,
										float cx1, float cy1, float radius1);
/**
 * @brief edit an existing radial gradient.
 *
 * Edit control points of an existing radial gradient
 *
 * @param pat the pattern to edit
 * @param cx0 x coordinate for the center of the start circle, the inner circle. Must stand inside outer circle.
 * @param cy0 y coordinate for the center of the start circle, the inner circle. Must stand inside outer circle.
 * @param radius0 radius for the center of the start circle, the inner circle. Can't be greater than radius1
 * @param cx1 x coordinate for the center of the end circle, the outer circle.
 * @param cy1 y coordinate for the center of the end circle, the outer circle.
 * @param radius1 radius for the center of the end circle, the outer circle.
 * @return VKVG_STATUS_SUCCESS, or VKVG_STATUS_PATTERN_TYPE_MISMATCH if the pattern is not a radial gradient.
 */
vkvg_public
vkvg_status_t vkvg_pattern_edit_radial(VkvgPattern pat,
								float cx0, float cy0, float radius0,
								float cx1, float cy1, float radius1);
/**
 * @brief get color stop count.
 *
 * Retrieve the color stop count of a gradient pattern.
 *
 * @param pat a valid pattern pointer.
 * @param count a valid integer pointer to old the current stop count returned.
 * @return VKVG_STATUS_SUCCESS, or VKVG_STATUS_PATTERN_TYPE_MISMATCH if the pattern is not a gradient.
 */
vkvg_public
vkvg_status_t vkvg_pattern_get_color_stop_count (VkvgPattern pat, uint32_t* count);
/**
 * @brief get color stop.
 *
 * Gets the color and offset information at the given index for a gradient pattern. Values of index range from 0 to n-1 where n is the number
 * returned by @ref vkvg_pattern_get_color_stop_count().
 *
 * @param pat a valid pattern pointer.
 * @param index index of the stop to return data for.
 * @param offset a valid float pointer to old the stop offset.
 * @param r a valid float pointer to old the red component.
 * @param g a valid float pointer to old the green component.
 * @param b a valid float pointer to old the blue component.
 * @param a a valid float pointer to old the alpha component.
 * @return VKVG_STATUS_SUCCESS, VKVG_STATUS_PATTERN_TYPE_MISMATCH if the pattern is not a gradient, VKVG_STATUS_INVALID_INDEX if index is out of bounds.
 */
vkvg_public
vkvg_status_t vkvg_pattern_get_color_stop_rgba (VkvgPattern pat, uint32_t index,
												float* offset, float* r, float* g, float* b, float* a);

/**
 * @brief dispose pattern.
 *
 * When you have finished using a pattern, free its ressources by calling this method.
 *
 * @param pat the pattern to destroy.
 */
vkvg_public
void vkvg_pattern_destroy (VkvgPattern pat);
/**
 * @brief add colors to gradients
 *
 * for each color step in the gradient, call this method and provide an absolute position between 0 and 1
 * and a color.
 *
 * @param pat the gradient pattern to add a color step.
 * @param offset location along the gradient's control vector, value ranging from zero (start of the gradient) to one.
 * @param r the red component of the color step
 * @param g the green component of the color stop
 * @param b the blue component of the color stop
 * @param a the alpha chanel of the color stop
 */
vkvg_public
vkvg_status_t vkvg_pattern_add_color_stop(VkvgPattern pat, float offset, float r, float g, float b, float a);
/**
 * @brief control the extend of the pattern
 *
 * control whether the pattern has to be repeated or extended when painted on a surface.
 *
 * @param pat the pattern to set extend for.
 * @param extend one value of the @ref vkvg_extend_t enumeration.
 * @return VKVG_STATUS_SUCCESS, or VKVG_STATUS_PATTERN_TYPE_MISMATCH if the pattern is not a gradient.
 */
vkvg_public
void vkvg_pattern_set_extend (VkvgPattern pat, vkvg_extend_t extend);
/**
 * @brief control the filtering when using this pattern on a surface.
 *
 * @param pat pat the pattern to set filter for.
 * @param filter one value of the @ref vkvg_filter_t enumeration.
 */
vkvg_public
void vkvg_pattern_set_filter (VkvgPattern pat, vkvg_filter_t filter);
/**
 * @brief query the current extend value for a pa
 *
 * @param pat
 * @return vkvg_extend_t
 */
vkvg_public
vkvg_extend_t vkvg_pattern_get_extend (VkvgPattern pat);
/**
 * @brief
 *
 * @param pat
 * @return vkvg_filter_t
 */
vkvg_public
vkvg_filter_t vkvg_pattern_get_filter (VkvgPattern pat);
/**
 * @brief get pattern type
 *
 * may be one of the @ref vkvg_pattern_type_t enumeration
 *
 * @param pat the pattern to query
 * @return vkvg_pattern_type_t
 */
vkvg_public
vkvg_pattern_type_t vkvg_pattern_get_type (VkvgPattern pat);

vkvg_public
void vkvg_pattern_set_matrix (VkvgPattern pat, const vkvg_matrix_t* matrix);

vkvg_public
void vkvg_pattern_get_matrix (VkvgPattern pat, vkvg_matrix_t* matrix);

vkvg_public
void vkvg_push_group (VkvgContext ctx);

vkvg_public
VkvgPattern vkvg_pop_group (VkvgContext ctx);

vkvg_public
void vkvg_pop_group_to_source (VkvgContext ctx);

/** @}*/

/********* EXPERIMENTAL **************/
vkvg_public
void vkvg_set_source_color_name (VkvgContext ctx, const char* color);

#ifdef VKVG_RECORDING
typedef struct _vkvg_recording_t* VkvgRecording;

vkvg_public
void			vkvg_start_recording	(VkvgContext ctx);
vkvg_public
VkvgRecording	vkvg_stop_recording		(VkvgContext ctx);
vkvg_public
void			vkvg_replay				(VkvgContext ctx, VkvgRecording rec);
vkvg_public
void			vkvg_replay_command		(VkvgContext ctx, VkvgRecording rec, uint32_t cmdIndex);
vkvg_public
void			vkvg_recording_get_command (VkvgRecording rec, uint32_t cmdIndex, uint32_t* cmd, void** dataOffset);
vkvg_public
uint32_t		vkvg_recording_get_count(VkvgRecording rec);
vkvg_public
void*			vkvg_recording_get_data (VkvgRecording rec);
vkvg_public
void			vkvg_recording_destroy	(VkvgRecording rec);
/*************************************/
#endif

#ifdef __cplusplus
}
#endif

#endif

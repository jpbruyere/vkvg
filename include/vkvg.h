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
 * Documentation of all members: vkvg.h
 *
 * VKVG is an open source 2d vector drawing library written in @b c and using [vulkan](https://www.khronos.org/vulkan/) for hardware acceleration.
 * Its api is modeled on the [cairo graphic library](https://www.cairographics.org/) with the following software components:
 *
 * - @ref surface
 */

/*! @file vkvg.h
 *  @brief The header of the VKVG library.
 *
 *  This is the header file of the VKVG library.  It defines all its types and
 *  declares all its functions.
 *
 *  For more information about how to use this file, see @ref build_include.
 */
/*! @defgroup surface Surface
 *  @brief Functions and types related to VKVG surface.
 *
 *  This is the reference documentation for creating, using and destroying VKVG
 *  Surfaces used as backend for drawing operations.
 */
/*! @defgroup context Context
 *  @brief Functions and types related to VKVG contexts.
 *
 *  This is the reference documentation for VKVG contexts used to draw on @ref surface.
 */
/*! @defgroup path Path creation and manipulation reference.
 *  @brief Functions and types related to path edition.
 */
/*! @defgroup pattern Pattern
 *  @brief Functions and types related to VKVG patterns.
 *
 *  This is the reference documentation for VKVG patters used as source for drawing operations.
 */

#include <vulkan/vulkan.h>
#include <math.h>
#include <stdbool.h>

/*! @defgroup VKVG version macros
 *	@brief Compile-time and run-time version checks.
 *
 *	vkvg library versioning follows the [Semantic Versioning 2.0.0](https://semver.org/)
 *
 *  @{ */
/*! @brief Major version number of the VKVG library.
 *
 *  This is incremented when the API is changed in non-compatible ways.
 *  @ingroup init
 */
#define VKVG_VERSION_MAJOR          0
/*! @brief The minor version number of the VKVG library.
 *
 *  This is incremented when features are added to the API but it remains
 *  backward-compatible.
 *  @ingroup init
 */
#define VKVG_VERSION_MINOR          1
/*! @brief The revision number of the VKVG library.
 *
 *  This is incremented when a bug fix release is made that does not contain any
 *  API changes.
 *  @ingroup init
 */
#define VKVG_VERSION_REVISION       1
/*! @} */

#ifdef DEBUG
extern uint8_t vkvg_log_level;
#endif

/**
 * @brief vkvg operation status.
 *
 * vkvg_status_t is used to indicates errors that can occur when using vkvg. Several vkvg function directely
 * return result, but when using a #VkvgContext, the last error is stored in the context and can be accessed
 * with #vkvg_status.
 */
typedef enum {
	VKVG_STATUS_SUCCESS = 0,			/*!< no error occurred.*/
	VKVG_STATUS_NO_MEMORY,				/*!< out of memory*/
	VKVG_STATUS_INVALID_RESTORE,		/*!< call to #vkvg_restore without matching call to #vkvg_save*/
	VKVG_STATUS_NO_CURRENT_POINT,		/*!< path command expecting a current point to be defined failed*/
	VKVG_STATUS_INVALID_MATRIX,			/*!< invalid matrix (not invertible)*/
	VKVG_STATUS_INVALID_STATUS,			/*!< */
	VKVG_STATUS_NULL_POINTER,			/*!< NULL pointer*/
	VKVG_STATUS_INVALID_STRING,			/*!< */
	VKVG_STATUS_INVALID_PATH_DATA,		/*!< */
	VKVG_STATUS_READ_ERROR,				/*!< */
	VKVG_STATUS_WRITE_ERROR,			/*!< */
	VKVG_STATUS_SURFACE_FINISHED,		/*!< */
	VKVG_STATUS_SURFACE_TYPE_MISMATCH,	/*!< */
	VKVG_STATUS_PATTERN_TYPE_MISMATCH,	/*!< */
	VKVG_STATUS_INVALID_CONTENT,		/*!< */
	VKVG_STATUS_INVALID_FORMAT,			/*!< */
	VKVG_STATUS_INVALID_VISUAL,			/*!< */
	VKVG_STATUS_FILE_NOT_FOUND,			/*!< */
	VKVG_STATUS_INVALID_DASH,			/*!< invalid value for a dash setting */
	VKVG_STAtUS_NOT_ENOUGH_POINTS_TO_CLOSE_PATH/*!< trying to close path with less than 3 points defined*/
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

/**
 * @brief Opaque pointer on a vkvg text run.
 * A #VkvgText is an intermediate representation
 * of a text to be drawn on a #VkvgSurface.
 * It contains the measurments computed for character poisitioning.
 *
 * This object is used to speed up the rendering of the same
 * text with the same font multiple times.
 *
 * Drawing text with @ref vkvg_show_text implicitly create such intermediate structure
 * that is destroyed imediatly after the function call.
 */
typedef struct _vkvg_text_run_t* VkvgText;

/**
 * @brief The Vkvg drawing Context.
 *
 * A #VkvgContext is the central object for drawing operations.
 * #vkvg_context_t structure internals this pointer point to are
 * hidden to the client application.
 */
typedef struct _vkvg_context_t* VkvgContext;
/**
 * @brief Opaque pointer on a Vkvg Surface structure.
 *
 * A #VkvgSurface represents an image, either as the destination
 * of a drawing operation or as source when drawing onto another
 * surface.  To draw to a #VkvgSurface, create a vkvg context
 * with the surface as the target, using #vkvg_create().
 * hidden internals.
 *
 * #VkvgSurface are created with a @ref VkvgDevice which has to stay
 * active as long has the surface is in use.
 */
typedef struct _vkvg_surface_t* VkvgSurface;
/**
 * @brief Opaque pointer on a Vkvg device structure.
 *
 * A #VkvgDevice is required for creating new surfaces.
 */
typedef struct _vkvg_device_t*  VkvgDevice;
/**
 * @brief Opaque pointer on a Vkvg pattern structure.
 *
 * Patterns are images to be drawn on surface with several
 * configurable parameters such as the wrap mode, the filtering, etc...
 */
typedef struct _vkvg_pattern_t* VkvgPattern;

/*!	@defgroup matrix Matrices
 *	@brief Generic matrix operations
 *
 *	This is the reference documentation for handling matrices to use as transformation in drawing operations.
 *	Matrix computations in vkvg are taken from the cairo library.
 * @{ */
#define VKVG_IDENTITY_MATRIX {1,0,0,1,0,0}/*!< The identity matrix*/
/**
 * @xx: xx component of the affine transformation
 * @yx: yx component of the affine transformation
 * @xy: xy component of the affine transformation
 * @yy: yy component of the affine transformation
 * @x0: X translation component of the affine transformation
 * @y0: Y translation component of the affine transformation
 *
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
void vkvg_matrix_init_translate     (vkvg_matrix_t *matrix, float tx, float ty);
/**
 * @brief scaling matrix initialization
 *
 * Initialize members of the supplied matrix to create a new scaling matrix
 * @param matrix a valid #vkvg_matrix_t pointer
 * @param sx scale in the x direction
 * @param sy Scale in the y direction
 */
void vkvg_matrix_init_scale         (vkvg_matrix_t *matrix, float sx, float sy);
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
void vkvg_matrix_init_rotate        (vkvg_matrix_t *matrix, float radians);
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
void vkvg_matrix_translate          (vkvg_matrix_t *matrix, float tx, float ty);
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
void vkvg_matrix_scale              (vkvg_matrix_t *matrix, float sx, float sy);
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
void vkvg_matrix_rotate             (vkvg_matrix_t *matrix, float radians);
/**
 * @brief matrices multiplication
 *
 * compute the multiplication of two matrix.
 * @param result a valid #vkvg_matrix_t pointer to hold the resulting matrix
 * @param a first operand of the multiplication
 * @param b second operand of the multiplication
 */
void vkvg_matrix_multiply           (vkvg_matrix_t *result, const vkvg_matrix_t *a, const vkvg_matrix_t *b);
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
void vkvg_matrix_transform_distance (const vkvg_matrix_t *matrix, float *dx, float *dy);
/**
 * @brief transform point
 *
 * Transforms the point (x , y ) by matrix .
 * @param matrix a valid #vkvg_matrix_t to use to transform point
 * @param x X position. An in/out parameter
 * @param y Y position. An in/out parameter
 */
void vkvg_matrix_transform_point    (const vkvg_matrix_t *matrix, float *x, float *y);
/**
 * @brief invert matrix
 *
 * Changes matrix to be the inverse of its original value. Not all transformation matrices have inverses;
 * if the matrix collapses points together (it is degenerate), then it has no inverse and this function will fail.
 * @param matrix the matrix to invert
 * @return If matrix has an inverse, modifies matrix to be the inverse matrix and returns VKVG_STATUS_SUCCESS.
 * Otherwise, returns VKVG_STATUS_INVALID_MATRIX.
 */
vkvg_status_t vkvg_matrix_invert(vkvg_matrix_t *matrix);
/** @}*/



/*! @defgroup device Device
 *  @brief Functions and types related to VKVG device.
 *
 *  This is the reference documentation for creating, using and destroying VKVG
 *  Devices used to connect to vulkan context.
 * @{ */

/**
 * @brief Create a new vkvg device.
 *
 * On success, create a new vkvg device set its reference count to 1.
 *
 * @param inst Vulkan instance to create the device from.
 * @param phy Vulkan physical device used to create the vkvg device.
 * @param vkdev Vulkan logical device to create the vkvg device for.
 * @param qFamIdx Queue family Index of the graphic queue used for drawing operations.
 * @param qIndex Index of the queue into the choosen familly, 0 in general.
 * @return The handle of the created vkvg device, or null if an error occured.
 */
VkvgDevice vkvg_device_create (VkInstance inst, VkPhysicalDevice phy, VkDevice vkdev, uint32_t qFamIdx, uint32_t qIndex);
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
VkvgDevice vkvg_device_create_multisample (VkInstance inst, VkPhysicalDevice phy, VkDevice vkdev, uint32_t qFamIdx, uint32_t qIndex, VkSampleCountFlags samples, bool deferredResolve);
/**
 * @brief Decrement the reference count of the device by 1. Release all it's ressources if count reach 0.
 *
 * If device is reference by several active surfaces, calling destroy will only decrement the reference count by 1.
 * When the reference count reach 0, the vkvg device is effectively destroyed, the device pointer is freed, and
 * vulkan objects are released.
 * Vkvg Devices has to stay active as long as surfaces created by it are used.
 * @param dev The vkvg device to destroy.
 */
void vkvg_device_destroy (VkvgDevice dev);
/**
 * @brief Increment by one the reference count on the device.
 * @param The vkvg device pointer to increment reference for.
 * @return ?
 */
VkvgDevice vkvg_device_reference (VkvgDevice dev);
/**
 * @brief Get the actual reference count on this device.
 * @param dev The vkvg device to get the reference of.
 * @return The reference count on this device.
 */
uint32_t vkvg_device_get_reference_count (VkvgDevice dev);
/**
 * @brief Set the screen dot per inch for this device.
 *
 * Set the current values for horizontal and vertical dot per inch of the screen for this device.
 * @param dev The vkvg device to set the dpy for.
 * @param hdpy Horizontal dot per inch.
 * @param vdpy Vertical dot per inch.
 */
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
void vkvg_device_get_dpy (VkvgDevice dev, int* hdpy, int* vdpy);
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
VkvgSurface vkvg_surface_create             (VkvgDevice dev, uint32_t width, uint32_t height);
/**
 * @brief Create a new vkvg surface by loading an image file. The resulting surface will have the same dimension as the supplied image.
 *
 * @param dev The vkvg device used for creating the surface.
 * @param filePath The path of the image to load for creating the surface.
 * @return The new vkvg surface with the loaded image as content, or null if an error occured.
 */
VkvgSurface vkvg_surface_create_from_image  (VkvgDevice dev, const char* filePath);
/**
 * @brief Create a new vkvg surface by loading a SVG file.
 * @param dev The vkvg device used for creating the surface.
 * @param filePath The path of the SVG file to load.
 * @return The new vkvg surface with the loaded SVG drawing as content, or null if an error occured.
 */
VkvgSurface vkvg_surface_create_from_svg    (VkvgDevice dev, const char* filePath);
/**
 * @brief Create a new vkvg surface by parsing a string with a valid SVG fragment passed as argument.
 * @param dev The vkvg device used for creating the surface.
 * @param fragment The SVG fragment to parse.
 * @return The new vkvg surface with the parsed SVG fragment as content, or null if an error occured.
 */
VkvgSurface vkvg_surface_create_from_svg_fragment (VkvgDevice dev, char *fragment);
/**
 * @brief Create a new vkvg surface that will used an existing vulkan texture as backend.
 * @param dev The vkvg device used for creating the surface.
 * @param vkhImg The VkhImage to use as the backend texture for drawing operations.
 * @return A new surface, or null if an error occured.
 */
VkvgSurface vkvg_surface_create_for_VkhImage(VkvgDevice dev, void* vkhImg);
// VkvgSurface vkvg_surface_create_from_bitmap (VkvgDevice dev, unsigned char* img, uint32_t width, uint32_t height);
/**
 * @brief Increment reference count on the surface by one
 * @param The vkvg surface to increment the reference count for.
 * @return ?
 */
VkvgSurface vkvg_surface_reference          (VkvgSurface surf);
/**
 * @brief Get the current reference count on this surface.
 * @param The vkvg surface to get the reference count for.
 * @return The reference count on the surface.
 */
uint32_t    vkvg_surface_get_reference_count(VkvgSurface surf);
/**
 * @brief Decrement the reference count on the surface by one. Destroy it if count reach 0.
 * @param The vkvg surface to destroy.
 */
void		vkvg_surface_destroy		(VkvgSurface surf);
/**
 * @brief Clear the surface content, alpha is also set to 0 resulting in a transparent image.
 *
 * @remark Internaly, the vulkan method used to clear the surface is the slowest, prefer using the @ref vkvg_clear
 * function of the context that will try to use the render pass load operations when possible.
 * @param The surface to clear.
 */
void        vkvg_surface_clear          (VkvgSurface surf);
/**
 * @brief Get the final single sampled vulkan image of this surface.
 * @param The vkvg surface to get the vulkan texture of.
 * @return The VkImage object containing the result of the drawing operations on the surface.
 */
VkImage		vkvg_surface_get_vk_image	(VkvgSurface surf);
/**
 * @brief Get the vulkan format of the vulkan texture used as backend for this surface.
 * @param The surface to get the format for.
 * @return The VkFormat.
 */
VkFormat	vkvg_surface_get_vk_format	(VkvgSurface surf);
/**
 * @brief Get the actual surface width.
 * @param The vkvg surface to get the width for.
 * @return The width in pixel of the surface.
 */
uint32_t   	vkvg_surface_get_width      (VkvgSurface surf);
/**
 * @brief Get the actual surface height.
 * @param The vkvg surface to get the height for.
 * @return The height in pixel of the surface.
 */
uint32_t	vkvg_surface_get_height     (VkvgSurface surf);
/**
 * @brief Write surface content to a png file on disk.
 * @param The surface to save on disk.
 * @param The png file path.
 */
void        vkvg_surface_write_to_png   (VkvgSurface surf, const char* path);
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
void        vkvg_multisample_surface_resolve (VkvgSurface surf);
/** @}*/

//nsvg interface for easy svg drawing
typedef struct NSVGimage NSVGimage;

NSVGimage*  nsvg_load_file   (VkvgDevice dev, const char* filePath);
NSVGimage*  nsvg_load        (VkvgDevice dev, char* fragment);
void        nsvg_destroy     (NSVGimage* svg);
void        nsvg_get_size    (NSVGimage* svg, int* width, int* height);
void        vkvg_render_svg  (VkvgContext ctx, NSVGimage* svg, char* subId);


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

/** @addtogroup context
 * @{ */

/**
 * @brief Create a new vkvg context used for drawing on surfaces.
 * @param surf The target surface of the drawing operations.
 * @return A new #VkvgContext or null if an error occured.
 */
VkvgContext vkvg_create (VkvgSurface surf);
/**
 * @brief Decrement the reference count on the context by one. Destroy it if count reach 0.
 * @param ctx The vkvg context to destroy.
 */
void vkvg_destroy (VkvgContext ctx);
/**
 * @brief Increment by one the reference count on this context.
 * @param ctx The context to increment the reference count for.
 * @return
 */
VkvgContext vkvg_reference (VkvgContext ctx);
/**
 * @brief Get the current reference count of this context.
 * @param The vkvg context to query.
 * @return The current reference count for this context.
 */
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
void vkvg_flush (VkvgContext ctx);
/**
 * @brief Start a new empty path.
 *
 * Clear current path stored in the context without drawing anything on the target surface.
 * The current position is reseted.
 * @param ctx The vkvg context pointer.
 */
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
void vkvg_close_path (VkvgContext ctx);
/**
 * @brief Start new sub path, no current point is defined
 *
 * If the current path is empty, this function has no effect.
 * @param ctx The vkvg context pointer.
 */
void vkvg_new_sub_path (VkvgContext ctx);
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
void vkvg_curve_to (VkvgContext ctx, float x1, float y1, float x2, float y2, float x3, float y3);
/**
 * @brief Add an axis aligned rectangle subpath to the current path.
 *
 * Adds a closed sub-path rectangle of the given size to the current path at position (x, y).
 * @param ctx The vkvg context pointer.
 * @param x The x coordinate of the top left corner of the rectangle to emit.
 * @param y The y coordinate of the top left corner of the rectangle to emit.
 * @param w The width in pixel of the rectangle to draw.
 * @param h The height in pixel of the rectangle to draw.
 */
void vkvg_rectangle         (VkvgContext ctx, float x, float y, float w, float h);

void vkvg_fill_rectangle    (VkvgContext ctx, float x, float y, float w, float h);
void vkvg_stroke			(VkvgContext ctx);
void vkvg_stroke_preserve	(VkvgContext ctx);
void vkvg_fill				(VkvgContext ctx);
void vkvg_fill_preserve		(VkvgContext ctx);
void vkvg_paint             (VkvgContext ctx);
void vkvg_clear             (VkvgContext ctx);//use vkClearAttachment to speed up clearing surf
void vkvg_reset_clip        (VkvgContext ctx);
void vkvg_clip              (VkvgContext ctx);
void vkvg_clip_preserve     (VkvgContext ctx);
/**
 * @brief set color as new source with an alpha component.
 *
 * Set current source to the solid color defined by the rgba components with 'a' for transparency.
 *
 * @param ctx vkvg context pointer
 * @param r the red component of the color.
 * @param g the green component of the color.
 * @param b the blue component of the color.
 * @param a the alpha component holding the transparency for the current color.
 */
void vkvg_set_source_rgba	(VkvgContext ctx, float r, float g, float b, float a);
/**
 * @brief set color as new source.
 *
 * Set current source to the solid color defined by the rgb components.
 *
 * @param ctx vkvg context pointer
 * @param r the red component of the color.
 * @param g the green component of the color.
 * @param b the blue component of the color.
 * @param a the alpha component holding the transparency for the current color.
 */
void vkvg_set_source_rgb    (VkvgContext ctx, float r, float g, float b);
/**
 * @brief set line width.
 *
 * Set the current line width for the targeted context. All further calls to #vkvg_stroke on this context
 * will use this new width.
 *
 * @param ctx vkvg context pointer.
 * @param width new current line width for the context.
 */
void vkvg_set_line_width	(VkvgContext ctx, float width);
/**
 * @brief set line terminations
 *
 * Configure the line terminations to output for further path stroke commands.
 * @param ctx vkvg context pointer.
 * @param cap new line termination, may be one of the value of #vkvg_line_cap_t.
 */

void vkvg_set_line_cap      (VkvgContext ctx, vkvg_line_cap_t cap);
/**
 * @brief set line joins
 *
 * Configure the line join to output for further path stroke commands.
 * @param ctx vkvg context pointer.
 * @param join new line join as defined in #vkvg_line_joint_t.
 */
void vkvg_set_line_join     (VkvgContext ctx, vkvg_line_join_t join);
/**
 * @brief use supplied surface as current pattern
 *
 * set #VkvgSurface as the current context source.
 * @param ctx vkvg context pointer
 * @param surf the vkvg surface to use as source.
 * @param x an x offset to apply for drawing operations using this surface.
 * @param y an y offset to apply for drawing operations using this surface.
 */
void vkvg_set_source_surface(VkvgContext ctx, VkvgSurface surf, float x, float y);
/**
 * @brief set supplied pattern as current source.
 *
 * set #VkvgPattern as the new source for the targeted context.
 * @param ctx vkvg context pointer
 * @param pat the new pattern to use as source for further drawing operations.
 */
void vkvg_set_source        (VkvgContext ctx, VkvgPattern pat);
void vkvg_set_operator      (VkvgContext ctx, vkvg_operator_t op);
void vkvg_set_fill_rule     (VkvgContext ctx, vkvg_fill_rule_t fr);

void vkvg_set_dash          (VkvgContext ctx, const float* dashes, uint32_t num_dashes, float offset);
void vkvg_get_dash          (VkvgContext ctx, const float *dashes, uint32_t* num_dashes, float* offset);

float               vkvg_get_line_width     (VkvgContext ctx);
vkvg_line_cap_t     vkvg_get_line_cap       (VkvgContext ctx);
vkvg_line_join_t    vkvg_get_line_join      (VkvgContext ctx);
vkvg_operator_t     vkvg_get_operator       (VkvgContext ctx);
vkvg_fill_rule_t    vkvg_get_fill_rule      (VkvgContext ctx);
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
void        vkvg_text_run_get_extents(VkvgText textRun, vkvg_text_extents_t* extents);
/** @}*/

/** @addtogroup pattern
 * @{ */
VkvgPattern vkvg_pattern_reference          (VkvgPattern pat);
uint32_t    vkvg_pattern_get_reference_count(VkvgPattern pat);
VkvgPattern vkvg_pattern_create_for_surface (VkvgSurface surf);
VkvgPattern vkvg_pattern_create_linear      (float x0, float y0, float x1, float y1);
VkvgPattern vkvg_pattern_create_radial      (float cx0, float cy0, float radius0,
											 float cx1, float cy1, float radius1);
void        vkvg_pattern_destroy            (VkvgPattern pat);

void vkvg_pattern_add_color_stop(VkvgPattern pat, float offset, float r, float g, float b, float a);
void vkvg_pattern_set_extend    (VkvgPattern pat, vkvg_extend_t extend);
void vkvg_pattern_set_filter    (VkvgPattern pat, vkvg_filter_t filter);

vkvg_extend_t   vkvg_pattern_get_extend (VkvgPattern pat);
vkvg_filter_t   vkvg_pattern_get_filter (VkvgPattern pat);
/** @}*/


#ifdef __cplusplus
}
#endif

#endif

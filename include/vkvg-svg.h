// Copyright (c) 2018-2026 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

#ifndef VKVG_SVG_H
#define VKVG_SVG_H

#include "vkvg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _svg_placeholder* VkvgSvg;

/**
 * @brief load svg file into @ref surface
 *
 * Create a new vkvg surface by loading a SVG file.
 * @param dev The vkvg @ref device used for creating the surface.
 * @param svgFilePath The path of the SVG file to load.
 * @param width force the rendering width, if 0 autosize from svg.
 * @param height force the rendering height, if 0 autosize from svg.
 * @return The new vkvg surface with the loaded SVG drawing as content, or null if an error occured.
 */
vkvg_public VkvgSurface vkvg_surface_create_from_svg(VkvgDevice dev, uint32_t width, uint32_t height,
                                                     const char* svgFilePath);
/**
 * @brief create surface from svg fragment
 *
 * Create a new vkvg surface by parsing a string with a valid SVG fragment passed as argument.
 * @param dev The vkvg device used for creating the surface.
 * @param width force the rendering width, if 0 autosize from svg.
 * @param height force the rendering height, if 0 autosize from svg.
 * @param svgFragment The SVG fragment to parse.
 * @return The new vkvg surface with the parsed SVG fragment as content, or null if an error occured.
 */
vkvg_public VkvgSurface vkvg_surface_create_from_svg_fragment(VkvgDevice dev, uint32_t width, uint32_t height,
                                                              char* svgFragment);
/**
 * @brief get svg dimensions.
 *
 * retrieve dimensions from svg parsing.
 * @param svg a valid VkvgSvg handle.
 * @param width pointer to a valid integer to receive the svg width.
 * @param height pointer to a valid integer to receive the svg height.
 */
vkvg_public void vkvg_svg_get_dimensions(VkvgSvg svg, uint32_t* width, uint32_t* height);

/**
 * @brief Load svg file in memory.
 *
 * Open a svg file and store source in memory, head is parsed to query the svg dimensions.
 * @param svgFilePath a valid file path to the svg to load.
 * @return a VkvgSvg pointer.
 */
vkvg_public VkvgSvg vkvg_svg_load(const char* svgFilePath);

/**
 * @brief Load svg from an svg source fragment.
 * @param svgFragment A valid svg code fragment.
 * @return a VkvgSvg pointer.
 */
vkvg_public VkvgSvg vkvg_svg_load_fragment(char* svgFragment);

/**
 * @brief render svg on a context.
 * @param svg a valid VkvgSvg handle.
 * @param ctx a valid vkvg context.
 * @param id an optional id to limit rendering to specific `<g>` elemnt.
 */
vkvg_public void vkvg_svg_render(VkvgSvg svg, VkvgContext ctx, const char* id);

/**
 * @brief release VkvgSvg pointer resources.
 * @param svg a valid VkvgSvg handle to free.
 */
vkvg_public void vkvg_svg_destroy(VkvgSvg svg);

#ifdef __cplusplus
}
#endif
#endif

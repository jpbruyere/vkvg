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
#ifndef VKVG_SVG_H
#define VKVG_SVG_H

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief load svg file into @ref surface
 *
 * Create a new vkvg surface by loading a SVG file.
 * @param dev The vkvg @ref device used for creating the surface.
 * @param filePath The path of the SVG file to load.
 * @param width force the rendering width, if 0 autosize from svg.
 * @param height force the rendering height, if 0 autosize from svg.
 * @return The new vkvg surface with the loaded SVG drawing as content, or null if an error occured.
 */
vkvg_public
VkvgSurface vkvg_surface_create_from_svg(VkvgDevice dev, uint32_t width, uint32_t height, const char* filePath);
/**
 * @brief create surface from svg fragment
 *
 * Create a new vkvg surface by parsing a string with a valid SVG fragment passed as argument.
 * @param dev The vkvg device used for creating the surface.
 * @param width force the rendering width, if 0 autosize from svg.
 * @param height force the rendering height, if 0 autosize from svg.
 * @param fragment The SVG fragment to parse.
 * @return The new vkvg surface with the parsed SVG fragment as content, or null if an error occured.
 */
vkvg_public
VkvgSurface vkvg_surface_create_from_svg_fragment(VkvgDevice dev, uint32_t width, uint32_t height, char* svgFragment);

#ifndef VKVG_SVG
/*! @defgroup nsvg Nano SVG
 * @brief Render SVG drawings
 *
 * [NanoSVG](https://github.com/memononen/nanosvg) is a simple svg parser.
 * nanosvg does not suit production needs, but is implemented
 * in vkvg for quickly presenting some outputs with the library.
 *
 * First load svg file with #nsvg_load_file() then render it with vkvg by calling #vkvg_render_svg().
 * The #NSVGimage pointer has to be destroyed by calling #nsvg_destroy(). @ref context use is guarded by
 * a save/restore couple that will restore context at its state before the svg draw.
 * @{ */

/**
 * @brief nanosvg image structure
 *
 */
typedef struct NSVGimage NSVGimage;
/**
 * @brief load svg file
 *
 * Load basic svg file and store datas in a #NSVGimage structure. This structure
 * has to be destroyed by a call to #nsvg_destroy() when no longuer in use.
 * @param dev VkvgDevice to use for loading the svg file, needed for dpy.
 * @param filePath the path of the svg file to load
 * @return NSVGimage* a new pointer on a NSVGimage structure
 */
vkvg_public
NSVGimage* nsvg_load_file (VkvgDevice dev, const char* filePath);
/**
 * @brief load svg fragment
 *
 * create a new #NSVGimage by parsing an svg fragment passed as string. The
 * vkvg @ref device is required to get the device dot per inch configuration.
 * @param dev the vkvg @ref device
 * @param fragment a valid svg expression
 * @return NSVGimage* a new pointer on the resulting nanovg structure
 */
vkvg_public
NSVGimage* nsvg_load (VkvgDevice dev, char* fragment);
/**
 * @brief release #NSVGimage pointer
 *
 * destroy the #NSVGimage passed as argument.
 * @param svg the nanovg structure to release
 */
vkvg_public
void nsvg_destroy (NSVGimage* svg);
/**
 * @brief get svg dimensions
 *
 * query an #NSVGimage for its width and height.
 * @param svg a valid #NSVGimage pointer
 * @param width[out] a valid integer pointer to hold the svg width
 * @param height[out] a valid integer pointer to hold the svg height
 */
vkvg_public
void nsvg_get_size (NSVGimage* svg, int* width, int* height);
/**
 * @brief render #NSVGimage with vkvg context
 *
 * Render a svg drawing loaded with #nsvg_load_file() or #nsvg_load() using
 * a valid #VkvgContext. Svg rendering is guarded with a save/restore so that the supplied context
 * is not affected by the rendering and will be restored to it's previous state.
 *
 * @param ctx the #VkvgContext to use for rendering.
 * @param svg the #NSVGimage to render
 * @param subId Render only a subpart of the svg identified by a named svg group or NULL for
 * rendering the whole svg.
 */
vkvg_public
void vkvg_render_svg (VkvgContext ctx, NSVGimage* svg, char* subId);
/** @}*/

#ifdef __cplusplus
}
#endif

#endif
#endif

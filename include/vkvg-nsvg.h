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
#ifndef VKVG_NSVG_H
#define VKVG_NSVG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vkvg.h"

/*! @defgroup nsvg Nano SVG
 * @brief Render SVG drawings
 *
 * [NanoSVG](https://github.com/memononen/nanosvg) does not suit production needs, but is implemented
 * in vkvg for quickly presenting some outputs with the library.
 *
 * First load svg file with #nsvg_load_file() then render it with vkvg by calling #vkvg_render_svg().
 * The #NSVGimage pointer has to be destroyed by calling #nsvg_destroy()
 * @{ */

/**
 * @brief nanosvg image structure
 *
 */
typedef struct NSVGimage NSVGimage;
/**
 * @brief load svg file
 *
 * Load basic svg file and store datas in a #NSVGimage structure.
 * @param dev VkvgDevice to use for loading the svg file, needed for dpy.
 * @param filePath the path of the svg file to load
 * @return NSVGimage* a pointer on a NSVGimage structure
 */
vkvg_public
NSVGimage* nsvg_load_file (VkvgDevice dev, const char* filePath);
/**
 * @brief
 *
 * @param dev
 * @param fragment
 * @return NSVGimage*
 */
vkvg_public
NSVGimage* nsvg_load (VkvgDevice dev, char* fragment);
/**
 * @brief
 *
 * @param svg
 */
vkvg_public
void nsvg_destroy (NSVGimage* svg);
/**
 * @brief
 *
 * @param svg
 * @param width
 * @param height
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

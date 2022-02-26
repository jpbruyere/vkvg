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

#ifdef VKVG_SVG
typedef struct _vkvg_svg_t* VkvgSvg;
#else
typedef struct NSVGimage* VkvgSvg;
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
VkvgSurface vkvg_surface_create_from_svg(VkvgDevice dev, uint32_t width, uint32_t height, const char* svgFilePath);
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

vkvg_public
void vkvg_svg_get_dimensions (VkvgSvg svg, uint32_t* width, uint32_t* height);

vkvg_public
VkvgSvg vkvg_svg_load (const char* svgFilePath);

vkvg_public
VkvgSvg vkvg_svg_load_fragment (char* svgFragment);

vkvg_public
void vkvg_svg_render (VkvgSvg svg, VkvgContext ctx, const char* id);

vkvg_public
void vkvg_svg_destroy (VkvgSvg svg);

#ifdef __cplusplus
}
#endif
#endif

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
#include "vkvg_surface_internal.h"
#include "vkvg_device_internal.h"
#include "vkvg_context_internal.h"

#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "nanosvg.h"
#include "vkvg-svg.h"

void _svg_set_color (VkvgContext ctx, uint32_t c, float alpha) {
	float a = (c >> 24 & 255) / 255.f;
	float b = (c >> 16 & 255) / 255.f;
	float g = (c >> 8 & 255) / 255.f;
	float r = (c & 255) / 255.f;
	vkvg_set_source_rgba(ctx,r,g,b,a*alpha);
}

VkvgSurface _svg_load (VkvgDevice dev, NSVGimage* svg) {
	if (svg == NULL) {
		LOG(VKVG_LOG_ERR, "nsvg error");
		return NULL;
	}
	VkvgSurface surf = _create_surface(dev, FB_COLOR_FORMAT);
	if (!surf)
		return NULL;

	surf->width = (uint32_t)svg->width;
	surf->height = (uint32_t)svg->height;
	surf->new = true;

	_create_surface_images (surf);

	VkvgContext ctx = vkvg_create(surf);
	vkvg_svg_render(svg, ctx, NULL);
	vkvg_destroy(ctx);

	nsvgDelete(svg);

	surf->references = 1;
	vkvg_device_reference (surf->dev);

	return surf;
}

VkvgSurface vkvg_surface_create_from_svg (VkvgDevice dev, uint32_t width, uint32_t height, const char* filePath) {
	return _svg_load(dev, nsvgParseFromFile(filePath, "px", (float)dev->hdpi));
}
VkvgSurface vkvg_surface_create_from_svg_fragment (VkvgDevice dev, uint32_t width, uint32_t height,const char* fragment) {
	return _svg_load(dev, nsvgParse(fragment, "px", (float)dev->hdpi));
}
VkvgSvg vkvg_svg_load (const char* svgFilePath) {
	return nsvgParseFromFile(svgFilePath, "px", 96.0f);
}
VkvgSvg vkvg_svg_load_fragment (const char* svgFragment) {
	return nsvgParse (svgFragment, "px", 96.0f);
}
void vkvg_svg_destroy (VkvgSvg svg) {
	nsvgDelete(svg);
}
void vkvg_svg_get_dimensions (VkvgSvg svg, uint32_t* width, uint32_t* height) {
	*width = (uint32_t)svg->width;
	*height = (uint32_t)svg->height;
}

void vkvg_svg_render (VkvgSvg svg, VkvgContext ctx, const char* subId){
	NSVGshape* shape;
	NSVGpath* path;
	vkvg_save (ctx);

	vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);

	vkvg_set_source_rgba(ctx,0.0,0.0,0.0,1);

	for (shape = svg->shapes; shape != NULL; shape = shape->next) {
		if (subId != NULL) {
			if (strcmp(shape->id, subId)!=0)
				continue;
		}

		vkvg_new_path(ctx);

		float o = shape->opacity;

		vkvg_set_line_width(ctx, shape->strokeWidth);

		for (path = shape->paths; path != NULL; path = path->next) {
			float* p = path->pts;
			vkvg_move_to(ctx, p[0],p[1]);
			for (int i = 1; i < path->npts; i += 3) {
				p = &path->pts[i*2];
				vkvg_curve_to(ctx, p[0],p[1], p[2],p[3], p[4],p[5]);
			}
			if (path->closed)
				vkvg_close_path(ctx);
		}

		if (shape->fill.type == NSVG_PAINT_COLOR)
			_svg_set_color(ctx, shape->fill.color, o);
		else if (shape->fill.type == NSVG_PAINT_LINEAR_GRADIENT){
			NSVGgradient* g = shape->fill.gradient;
			_svg_set_color(ctx, g->stops[0].color, o);
		}

		if (shape->fill.type != NSVG_PAINT_NONE){
			if (shape->stroke.type == NSVG_PAINT_NONE){
				vkvg_fill(ctx);
				continue;
			}
			vkvg_fill_preserve (ctx);
		}

		if (shape->stroke.type == NSVG_PAINT_COLOR)
			_svg_set_color(ctx, shape->stroke.color, o);
		else if (shape->stroke.type == NSVG_PAINT_LINEAR_GRADIENT){
			NSVGgradient* g = shape->stroke.gradient;
			_svg_set_color(ctx, g->stops[0].color, o);
		}

		vkvg_stroke(ctx);
	}
	vkvg_restore (ctx);
}

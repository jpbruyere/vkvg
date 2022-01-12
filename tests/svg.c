#include "test.h"

#include "vkvg-svg.h"

static float rotation = 0.f;
static const char* path = "data/tiger.svg";

void svg_surface() {
	VkvgSurface svgSurf = vkvg_surface_create_from_svg (device, test_width, test_height, path);

	VkvgContext ctx = _initCtx(surf);

	vkvg_set_source_rgb(ctx,1,1,1);
	vkvg_paint(ctx);

	vkvg_set_source_surface(ctx, svgSurf, 0,0);
	vkvg_paint(ctx);

	vkvg_destroy(ctx);
	vkvg_surface_destroy(svgSurf);
}
void vkvg_svg () {
	VkvgSvg svg = vkvg_svg_load (path);
	uint32_t w, h;
	vkvg_svg_get_dimensions(svg, &w, &h);
	VkvgContext ctx = _initCtx(surf);
	vkvg_clear(ctx);

	vkvg_svg_render (svg, ctx, NULL);

	vkvg_destroy(ctx);
	vkvg_svg_destroy (svg);
}

int main(int argc, char *argv[]) {
	no_test_size = true;

	PERFORM_TEST (svg_surface, argc, argv);
	PERFORM_TEST (vkvg_svg, argc, argv);
	return 0;
}

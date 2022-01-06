#include "test.h"

#include "vkvg-svg.h"

static float rotation = 0.f;
static const char* path = "data/tiger.svg";

void svg_surface() {
	VkvgSurface svgSurf = vkvg_surface_create_from_svg(device, test_width, test_height, path);

	VkvgContext ctx = vkvg_create(surf);

	vkvg_set_source_rgb(ctx,1,1,1);
	vkvg_paint(ctx);

	vkvg_set_source_surface(ctx, svgSurf, 0,0);
	vkvg_paint(ctx);

	vkvg_destroy(ctx);
	vkvg_surface_destroy(svgSurf);
}
#ifndef VKVG_SVG
void nsvg() {
	NSVGimage* svg = nsvg_load_file(device, path);
	if (svg == NULL) {
		fprintf (stderr, "svg file not found: %s", path);
		return;
	}

	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
	vkvg_set_source_rgba(ctx,0.9f,1.0,1.0f,1);
	vkvg_paint(ctx);

	vkvg_scale(ctx,1.0f,1.0f);
	//vkvg_render_svg(ctx, svg, "wq");

	vkvg_render_svg(ctx, svg, NULL);

	vkvg_destroy(ctx);

	nsvg_destroy(svg);
}
#endif

int main(int argc, char *argv[]) {
	no_test_size = true;
	PERFORM_TEST (svg_surface, argc, argv);
#ifndef VKVG_SVG
	PERFORM_TEST (nsvg, argc, argv);
#endif
	return 0;
}

#include "test.h"

#include "string.h" //for nanosvg
//#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "nanosvg.h"

static float rotation = 0.f;
static const char* path = "data/tiger.svg";
//static const char* path = "data/vkvg.svg";
//static const char* path = "data/testPiece.svg";

void test_svg_surface() {
    VkvgSurface svgSurf = vkvg_surface_create_from_svg(device, path);

    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgb(ctx,0,0,0);
    vkvg_paint(ctx);

    vkvg_set_source_surface(ctx, svgSurf, 0,0);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
    vkvg_surface_destroy(svgSurf);
}

void test_nsvg() {
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

int main() {
    PERFORM_TEST (test_nsvg);
    return 0;
}

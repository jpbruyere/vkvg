#include "VkvgTest.hpp"

#include "vkvg-svg.h"

static float       rotation   = 0.f;

TEST(svg_surface_create_from_svg) {
    VkvgSurface svgSurf = vkvg_surface_create_from_svg(test->device, test->app->width, test->app->height, GET_PATH("tiger.svg"));
    VkvgContext ctx = vkvg_create(test->surf);

    vkvg_set_source_rgb(ctx, 1, 1, 1);
    vkvg_paint(ctx);

    vkvg_set_source_surface(ctx, svgSurf, 0, 0);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
    vkvg_surface_destroy(svgSurf);
}
TEST(svg_load_render) {
    VkvgSvg  svg = vkvg_svg_load(GET_PATH("tiger.svg"));
    uint32_t w, h;
    vkvg_svg_get_dimensions(svg, &w, &h);
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    vkvg_svg_render(svg, ctx, NULL);

    vkvg_destroy(ctx);
    vkvg_svg_destroy(svg);
}
TEST(svg_load_render_sub) {
    VkvgSvg  svg = vkvg_svg_load(GET_PATH("checkbox.svg"));
    uint32_t w, h;
    vkvg_svg_get_dimensions(svg, &w, &h);
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    vkvg_svg_render(svg, ctx, "#True");
    vkvg_translate(ctx, 200, 0);
    vkvg_svg_render(svg, ctx, "#False");

    vkvg_destroy(ctx);
    vkvg_svg_destroy(svg);
}

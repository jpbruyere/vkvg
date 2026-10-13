#include "test.h"
#include "vkvg-svg.h"

static float       rotation   = 0.f;
static const char* path       = TESTS_DATA_ROOT "tiger.svg";
static const char* svgSubPath = TESTS_DATA_ROOT "checkbox.svg";
static const char* svgText    = TESTS_DATA_ROOT "text.svg";

void svg_surface() {
    VkvgSurface svgSurf = vkvg_surface_create_from_svg(device, test_width, test_height, path);
    VkvgContext ctx = _initCtx();

    vkvg_set_source_rgb(ctx, 1, 1, 1);
    vkvg_paint(ctx);

    vkvg_set_source_surface(ctx, svgSurf, 0, 0);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
    vkvg_surface_destroy(svgSurf);
}
void vkvg_svg() {
    VkvgSvg  svg = vkvg_svg_load(path);
    uint32_t w, h;
    vkvg_svg_get_dimensions(svg, &w, &h);
    VkvgContext ctx = _initCtx();
    vkvg_clear(ctx);

    vkvg_svg_render(svg, ctx, NULL);

    vkvg_destroy(ctx);
    vkvg_svg_destroy(svg);
}
void vkvg_svg_sub() {
    VkvgSvg  svg = vkvg_svg_load(svgSubPath);
    uint32_t w, h;
    vkvg_svg_get_dimensions(svg, &w, &h);
    VkvgContext ctx = _initCtx();
    vkvg_clear(ctx);

    vkvg_svg_render(svg, ctx, "#True");
    vkvg_translate(ctx, 200, 0);
    vkvg_svg_render(svg, ctx, "#False");

    vkvg_destroy(ctx);
    vkvg_svg_destroy(svg);
}
void svg_text() {
    VkvgSurface svgSurf = vkvg_surface_create_from_svg(device, test_width, test_height, svgText);
    VkvgContext ctx = _initCtx();

    vkvg_set_source_rgb(ctx, 1, 1, 1);
    vkvg_paint(ctx);

    vkvg_set_source_surface(ctx, svgSurf, 0, 0);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
    vkvg_surface_destroy(svgSurf);
}
void svg_text_frag() {
    const char* txt = {"<svg><text id='txt' font-size='50' font-family='sans' x='50' y='50'>The crow jumed on the fox</text></svg>"};
    VkvgSurface svgSurf = vkvg_surface_create_from_svg_fragment(device, test_width, test_height, txt);
    VkvgContext ctx = _initCtx();

    vkvg_set_source_rgb(ctx, 1, 1, 1);
    vkvg_paint(ctx);

    vkvg_set_source_surface(ctx, svgSurf, 0, 0);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
    vkvg_surface_destroy(svgSurf);
}
int main(int argc, char* argv[]) {
    no_test_size = true;

    /*PERFORM_TEST(svg_surface, argc, argv);
    PERFORM_TEST(vkvg_svg, argc, argv);
    PERFORM_TEST(vkvg_svg_sub, argc, argv);*/
    PERFORM_TEST(svg_text_frag, argc, argv);
    //PERFORM_TEST(svg_text, argc, argv);
    return 0;
}

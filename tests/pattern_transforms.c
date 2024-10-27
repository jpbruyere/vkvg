#include "test.h"

float       lineWidth = 10.f;
const char *imgPath   = "data/miroir.jpg";

VkvgPattern create_grad(VkvgContext ctx) {
    VkvgPattern pat = vkvg_pattern_create_linear(0, 0, 200, 0);
    vkvg_pattern_add_color_stop(pat, 0, 1, 0, 0, 1);
    vkvg_pattern_add_color_stop(pat, 0.5, 0, 1, 0, 1);
    vkvg_pattern_add_color_stop(pat, 1.0, 0, 0, 1, 1);
    return pat;
}

VkvgSurface createSurf(uint32_t width, uint32_t height) {
    VkvgSurface s   = vkvg_surface_create(device, width, height);
    VkvgContext ctx = _initCtx(surf);
    vkvg_set_line_width(ctx, lineWidth);
    float hlw = lineWidth / 2.f;
    /*
    vkvg_set_source_rgba(ctx,0,1,0,0.5);
    vkvg_fill_preserve(ctx);*/
    vkvg_set_source_rgba(ctx, 1, 0, 0, 0.5);
    vkvg_paint(ctx);
    vkvg_set_source_rgba(ctx, 0, 0, 1, 0.5);
    vkvg_rectangle(ctx, hlw, hlw, (float)width - lineWidth, (float)height - lineWidth);
    vkvg_stroke(ctx);
    vkvg_destroy(ctx);
    return s;
}

void img_scale() {
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);
    VkvgPattern pat     = vkvg_pattern_create_for_surface(imgSurf);
    VkvgContext ctx     = _initCtx(surf);

    vkvg_translate(ctx, 100, 100);

    vkvg_matrix_t mat;
    vkvg_matrix_init_scale(&mat, 2.0, 2.0);
    vkvg_pattern_set_matrix(pat, &mat);

    vkvg_set_source(ctx, pat);
    vkvg_rectangle(ctx, 0, 0, 200, 140);
    vkvg_fill(ctx);
    vkvg_destroy(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_surface_destroy(imgSurf);
}

void pat_scale() {

    VkvgContext ctx = _initCtx(surf);
    VkvgPattern pat = create_grad(ctx);

    vkvg_translate(ctx, 100, 100);

    vkvg_matrix_t mat;
    vkvg_matrix_init_scale(&mat, 0.5, 0.5);
    vkvg_pattern_set_matrix(pat, &mat);

    vkvg_set_source(ctx, pat);
    vkvg_rectangle(ctx, 0, 0, 200, 140);
    vkvg_fill(ctx);
    vkvg_destroy(ctx);
    vkvg_pattern_destroy(pat);
}
static float angle = 0;
void         pat_rotate() {
    angle += 0.001f;

    VkvgContext ctx = _initCtx(surf);
    VkvgPattern pat = create_grad(ctx);

    // vkvg_translate(ctx, 100,100);

    vkvg_matrix_t mat;
    vkvg_matrix_init_translate(&mat, 100, 70);
    vkvg_matrix_rotate(&mat, angle);
    vkvg_pattern_set_matrix(pat, &mat);

    vkvg_set_source(ctx, pat);
    vkvg_rectangle(ctx, 0, 0, 200, 140);
    vkvg_fill(ctx);
    vkvg_destroy(ctx);
    vkvg_pattern_destroy(pat);
}
int main(int argc, char *argv[]) {
    no_test_size = true;
    PERFORM_TEST(pat_scale, argc, argv);
    PERFORM_TEST(pat_rotate, argc, argv);
    PERFORM_TEST(img_scale, argc, argv);
    return 0;
}

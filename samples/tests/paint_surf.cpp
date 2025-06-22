#include "VkvgTest.hpp"

float lineWidth = 10.f;

VkvgSurface createSurf(VkvgDevice dev, uint32_t width, uint32_t height) {
    VkvgSurface s   = vkvg_surface_create(dev, width, height);
    VkvgContext ctx = vkvg_create(s);
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

TEST(paint_surf) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    VkvgSurface src = createSurf(test->device, 256, 256);
    vkvg_set_source_surface(ctx, src, 0, 0);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
    vkvg_surface_destroy(src);
}

TEST(paint_surf_with_offset) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    VkvgSurface src = createSurf(test->device, 256, 256);
    vkvg_set_source_rgba(ctx, 0, 1, 0, 0.5);
    vkvg_paint(ctx);
    vkvg_set_source_surface(ctx, src, 100, 100);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
    vkvg_surface_destroy(src);
}
TEST(paint_surf_multiple) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    VkvgSurface src = createSurf(test->device, 256, 256);
    for (int i = 0; i < 10; i++) {
        vkvg_set_source_surface(ctx, src, i * 20, i * 20);
        vkvg_paint(ctx);
    }
    vkvg_destroy(ctx);
    vkvg_surface_destroy(src);
}
TEST(paint_surf_with_rotation) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    VkvgSurface src = createSurf(test->device, 256, 256);
    vkvg_set_source_surface(ctx, src, 0, 0);
    vkvg_rotate(ctx, 45);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
    vkvg_surface_destroy(src);
}
TEST(paint_surf_with_scale) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    VkvgSurface src = createSurf(test->device, 256, 256);
    vkvg_set_source_surface(ctx, src, 0, 0);
    vkvg_scale(ctx, 0.2f, 0.2f);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
    vkvg_surface_destroy(src);
}
TEST(paint_rect) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    VkvgSurface src = createSurf(test->device, 256, 256);
    vkvg_set_source_surface(ctx, src, 0, 0);
    vkvg_rectangle(ctx, 100, 100, 300, 200);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
    vkvg_surface_destroy(src);
}
// TODO:test failed: full screen paint instead of rotated rect
TEST(paint_rect_with_rotation) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    vkvg_rotate(ctx, 45);
    vkvg_set_source_rgba(ctx, 1, 0, 0, 1);
    vkvg_rectangle(ctx, 100, 100, 300, 200);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
}
TEST(paint_rect_with_scale) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    vkvg_scale(ctx, 0.2f, 0.2f);
    vkvg_set_source_rgba(ctx, 1, 0, 0, 1);
    vkvg_rectangle(ctx, 100, 100, 300, 200);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
}

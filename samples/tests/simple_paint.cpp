#include "VkvgTest.hpp"

TEST(paint) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    vkvg_set_source_rgba(ctx, 1, 0, 0, 0.5f);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
}
TEST(paint_with_rotation) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    vkvg_rotate(ctx, 45);
    vkvg_set_source_rgba(ctx, 1, 0, 0, 1);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
}
TEST(paint_with_scale) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    vkvg_scale(ctx, 0.2f, 0.2f);
    vkvg_set_source_rgba(ctx, 1, 0, 0, 1);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
}
TEST(paint_rect) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    vkvg_set_source_rgba(ctx, 1, 0, 0, 1);
    vkvg_rectangle(ctx, 100, 100, 300, 200);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
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

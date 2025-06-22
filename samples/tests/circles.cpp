#include "VkvgTest.hpp"

TEST(circles_scaled_up) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    vkvg_set_source_rgb(ctx, 1, 1, 1);

    vkvg_scale(ctx, 100, 100);
    vkvg_arc(ctx, 2, 2, 0.5f, 0, M_PIF * 2);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}
TEST(circles_fill_and_stroke) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    vkvg_set_source_rgba(ctx, 0, 0.1f, 0.8f, 0.5f);
    vkvg_set_line_width(ctx, 10);

    vkvg_arc(ctx, 300, 300, 150.f, 0, M_PIF * 2);
    vkvg_fill_preserve(ctx);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}
TEST(circles_sizes) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    vkvg_set_source_rgb(ctx, 1, 1, 1);

    draw_growing_circles(ctx, 100, 40);
    vkvg_fill(ctx);

    draw_growing_circles(ctx, 200, 40);
    vkvg_stroke(ctx);

    vkvg_set_source_rgba(ctx, 0, 0, 1, 0.4F);
    draw_growing_circles(ctx, 300, 40);
    vkvg_fill_preserve(ctx);
    vkvg_set_line_width(ctx, 5);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

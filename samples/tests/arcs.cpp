#include "VkvgTest.hpp"

TEST(arc_scaled_up) {
    VkvgContext ctx = vkvg_create(test->surf);

    vkvg_set_source_rgb(ctx, 1, 1, 1);
    vkvg_paint(ctx);
    vkvg_set_source_rgb(ctx, 0, 0, 0);

    vkvg_scale(ctx, 10, 10);
    vkvg_arc(ctx, 20, 20, 2.0f, 0, M_PIF / 2.f);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}
TEST(arc_sizes) {
    VkvgContext ctx = vkvg_create(test->surf);

    vkvg_set_source_rgb(ctx, 1, 1, 1);
    vkvg_paint(ctx);
    vkvg_set_source_rgb(ctx, 0, 0, 0);

    draw_growing_circles(ctx, 100, 40);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}
TEST(arc_test) {
    VkvgContext ctx = vkvg_create(test->surf);

    vkvg_set_source_rgb(ctx, 1, 1, 1);
    vkvg_paint(ctx);
    vkvg_set_source_rgb(ctx, 0, 0, 0);

    vkvg_set_source_rgb(ctx, 1, 0, 1);
    vkvg_set_line_width(ctx, 5.0);
    vkvg_arc(ctx, 100, 100, 20, 0, M_PIF / 2);
    vkvg_stroke(ctx);

    vkvg_set_source_rgb(ctx, 0, 1, 1);
    vkvg_arc_negative(ctx, 100, 100, 20, 0, M_PIF / 2);
    vkvg_stroke(ctx);

    vkvg_set_source_rgb(ctx, 1, 0, 1);
    vkvg_arc(ctx, 100, 200, 20, M_PIF / 2, 0);
    vkvg_stroke(ctx);

    vkvg_set_source_rgb(ctx, 0, 1, 1);
    vkvg_arc_negative(ctx, 100, 200, 20, M_PIF / 2, 0);
    vkvg_stroke(ctx);

    vkvg_set_source_rgb(ctx, 0, 0, 1);
    vkvg_set_line_width(ctx, 10.0);
    vkvg_arc(ctx, 350, 100, 40, 0, M_PIF * 2);
    vkvg_stroke(ctx);

    vkvg_set_source_rgb(ctx, 0, 1, 0);
    vkvg_set_line_width(ctx, 1.0);
    vkvg_arc(ctx, 150, 100, 3.5, 0, M_PIF * 2);
    vkvg_stroke(ctx);
    vkvg_arc(ctx, 200, 200, 10, 0, M_PIF * 2);
    vkvg_fill(ctx);

    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_scale(ctx, 3, 3);
    vkvg_arc(ctx, 150, 100, 3.5, 0, M_PIF * 2);
    vkvg_stroke(ctx);
    vkvg_arc(ctx, 200, 200, 10, 0, M_PIF * 2);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}

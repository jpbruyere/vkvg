#include "VkvgTest.hpp"
TEST(fill_and_stroke) {
    VkvgContext ctx = vkvg_create(test->surf);

    vkvg_move_to(ctx, 100, 100);
    vkvg_rel_line_to(ctx, 50, -80);
    vkvg_rel_line_to(ctx, 50, 80);
    vkvg_close_path(ctx);

    vkvg_move_to(ctx, 300, 100);
    vkvg_rel_line_to(ctx, 50, -80);
    vkvg_rel_line_to(ctx, 50, 80);
    vkvg_close_path(ctx);

    vkvg_set_line_width(ctx, 10.0);
    vkvg_set_source_rgb(ctx, 0, 0, 1);
    vkvg_fill_preserve(ctx);
    // vkvg_fill(ctx);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}
TEST(fill_non_zero) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_save(ctx);
    vkvg_set_line_width(ctx, 30);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);

    vkvg_set_source_rgba(ctx, 0.1f, 0.9f, 0.1f, 1.0f);
    vkvg_move_to(ctx, 100, 100);
    vkvg_rel_line_to(ctx, 200, 0);
    vkvg_rel_line_to(ctx, 0, 150);
    vkvg_rel_line_to(ctx, -200, 0);
    vkvg_close_path(ctx);

    vkvg_move_to(ctx, 150, 150);
    vkvg_rel_line_to(ctx, 0, 50);
    vkvg_rel_line_to(ctx, 100, 0);
    vkvg_rel_line_to(ctx, 0, -50);
    vkvg_close_path(ctx);

    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}


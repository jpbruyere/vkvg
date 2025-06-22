#include "VkvgTest.hpp"

void draw(VkvgContext ctx) {
    vkvg_set_source_rgba(ctx, 0, 0, 1, 0.5);
    vkvg_rectangle(ctx, 100, 100, 200, 200);
    vkvg_fill(ctx);

    vkvg_rectangle(ctx, 200, 200, 200, 200);
    vkvg_set_source_rgba(ctx, 1, 0, 0, 0.5);
    vkvg_fill(ctx);
}
TEST(rect_fill_nz) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);

    draw(ctx);

    vkvg_destroy(ctx);
}
TEST(rect_fill_eo) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);

    draw(ctx);

    vkvg_destroy(ctx);
}

TEST(fill_nz) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);

    vkvg_set_source_rgba(ctx, 0.1f, 0.9f, 0.1f, 1.0f);
    vkvg_move_to(ctx, 100, 100);
    vkvg_rel_line_to(ctx, 50, 200);
    vkvg_rel_line_to(ctx, 150, -100);
    vkvg_rel_line_to(ctx, 100, 200);
    vkvg_rel_line_to(ctx, -100, 100);
    vkvg_rel_line_to(ctx, -10, -100);
    vkvg_rel_line_to(ctx, -190, -50);
    vkvg_close_path(ctx);

    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}
TEST(fill_eo) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);

    vkvg_set_source_rgba(ctx, 0.1f, 0.9f, 0.1f, 1.0f);
    vkvg_move_to(ctx, 100, 100);
    vkvg_rel_line_to(ctx, 50, 200);
    vkvg_rel_line_to(ctx, 150, -100);
    vkvg_rel_line_to(ctx, 100, 200);
    vkvg_rel_line_to(ctx, -100, 100);
    vkvg_rel_line_to(ctx, -10, -100);
    vkvg_rel_line_to(ctx, -190, -50);
    vkvg_close_path(ctx);

    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}

TEST(fill_with_hole_nz) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

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
TEST(fill_with_hole_eo) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    vkvg_set_line_width(ctx, 30);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);

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

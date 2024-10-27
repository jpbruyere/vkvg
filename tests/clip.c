#include "test.h"

void clipped_paint_ec() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_paint(ctx);
    vkvg_set_source_rgb(ctx, 0, 1, 0);
    vkvg_rectangle(ctx, 100, 100, 300, 200);
    vkvg_clip(ctx);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
}
void clipped_paint_eo() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_paint(ctx);
    vkvg_set_source_rgb(ctx, 0, 1, 0);
    vkvg_rectangle(ctx, 100, 100, 300, 200);
    vkvg_clip(ctx);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
}
void clipped_paint2_ec() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_paint(ctx);
    vkvg_set_source_rgb(ctx, 0, 1, 0);
    vkvg_rectangle(ctx, 100, 100, 300, 200);
    vkvg_clip(ctx);
    vkvg_paint(ctx);

    vkvg_reset_clip(ctx);
    vkvg_rectangle(ctx, 200, 200, 300, 200);
    vkvg_set_source_rgb(ctx, 0, 0, 1);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
}
void clipped_paint2_eo() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_paint(ctx);
    vkvg_set_source_rgb(ctx, 0, 1, 0);
    vkvg_rectangle(ctx, 100, 100, 300, 200);
    vkvg_clip(ctx);
    vkvg_paint(ctx);
    vkvg_reset_clip(ctx);
    vkvg_rectangle(ctx, 200, 200, 300, 200);
    vkvg_set_source_rgb(ctx, 0, 0, 1);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
}
void clipped_transformed_ec() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_paint(ctx);
    vkvg_rotate(ctx, 0.2f);
    vkvg_set_source_rgb(ctx, 0, 1, 0);
    vkvg_rectangle(ctx, 100, 100, 300, 200);
    vkvg_clip(ctx);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
}
void clip_transformed_ec() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_paint(ctx);
    vkvg_rotate(ctx, 0.1f);
    vkvg_set_source_rgb(ctx, 0, 1, 0);
    vkvg_rectangle(ctx, 100, 100, 300, 200);
    vkvg_clip(ctx);
    vkvg_paint(ctx);
    vkvg_translate(ctx, 200, 100);
    vkvg_set_source_rgb(ctx, 0, 0, 1);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
}
void clipped_transformed_eo() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_paint(ctx);
    vkvg_rotate(ctx, 0.2f);
    vkvg_set_source_rgb(ctx, 0, 1, 0);
    vkvg_rectangle(ctx, 100, 100, 300, 200);
    vkvg_clip(ctx);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
}

void test_clip() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_clear(ctx);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
    vkvg_move_to(ctx, 10, 10);
    vkvg_line_to(ctx, 400, 150);
    vkvg_line_to(ctx, 900, 10);
    vkvg_line_to(ctx, 700, 450);
    vkvg_line_to(ctx, 900, 750);
    vkvg_line_to(ctx, 500, 650);
    vkvg_line_to(ctx, 100, 800);
    vkvg_line_to(ctx, 150, 400);
    vkvg_clip_preserve(ctx);
    vkvg_set_operator(ctx, VKVG_OPERATOR_CLEAR);
    vkvg_fill_preserve(ctx);
    vkvg_clip(ctx);
    vkvg_set_operator(ctx, VKVG_OPERATOR_OVER);

    vkvg_set_source_rgb(ctx, 1, 0, 0);
    // vkvg_set_line_width(ctx,10);
    // vkvg_stroke(ctx);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
}
void test_clip2() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_clear(ctx);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);
    vkvg_rectangle(ctx, 50, 50, 500, 500);
    vkvg_clip(ctx);

    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_paint(ctx);

    vkvg_save(ctx);
    vkvg_rectangle(ctx, 100, 100, 350, 350);
    vkvg_clip(ctx);
    vkvg_save(ctx);

    vkvg_set_source_rgb(ctx, 1, 1, 0);
    vkvg_paint(ctx);

    vkvg_rectangle(ctx, 200, 200, 200, 200);
    vkvg_clip(ctx);

    vkvg_set_source_rgb(ctx, 0, 1, 0);
    vkvg_paint(ctx);

    vkvg_restore(ctx);

    vkvg_rectangle(ctx, 350, 350, 420, 420);
    vkvg_set_source_rgb(ctx, 0, 0, 1);
    vkvg_fill(ctx);

    vkvg_restore(ctx);

    // vkvg_clip(ctx);
    //
    /*vkvg_clip_preserve(ctx);
    vkvg_set_operator(ctx, VKVG_OPERATOR_CLEAR);
    vkvg_fill_preserve(ctx);*/
    // vkvg_clip(ctx);
    // vkvg_set_operator(ctx, VKVG_OPERATOR_OVER);
    /*vkvg_rectangle(ctx, 200,200,220,220);
    vkvg_set_source_rgb(ctx,1,0,0);
    vkvg_paint(ctx);*/

    vkvg_destroy(ctx);
}
int main(int argc, char *argv[]) {
    no_test_size = true;
    PERFORM_TEST(clip_transformed_ec, argc, argv);
    PERFORM_TEST(clipped_paint_ec, argc, argv);
    PERFORM_TEST(clipped_paint_eo, argc, argv);
    PERFORM_TEST(clipped_transformed_ec, argc, argv);
    PERFORM_TEST(clipped_transformed_eo, argc, argv);
    PERFORM_TEST(clipped_paint2_ec, argc, argv);
    PERFORM_TEST(clipped_paint2_eo, argc, argv);
    PERFORM_TEST(test_clip, argc, argv);
    PERFORM_TEST(test_clip2, argc, argv);
    return 0;
}

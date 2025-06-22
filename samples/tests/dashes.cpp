#include "VkvgTest.hpp"

static float dash_offset = 0;

TEST(dashes0) {
    dash_offset += 0.2f;

    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    vkvg_set_source_rgb(ctx, 1, 1, 1);
    vkvg_paint(ctx);
    // const float dashes[] = {160.0f, 80};
    // float dashes[] = {7.0f, 3};
    float dashes[] = {100, 20, 20, 20};
    vkvg_set_line_cap(ctx, VKVG_LINE_CAP_ROUND);
    vkvg_set_line_join(ctx, VKVG_LINE_JOIN_ROUND);
    vkvg_set_dash(ctx, dashes, 4, dash_offset);
    vkvg_set_line_width(ctx, 4);
    vkvg_set_source_rgb(ctx, 0, 0, 1);

    vkvg_move_to(ctx, 50, 50);
    vkvg_rel_line_to(ctx, 500, 0);
    vkvg_rel_line_to(ctx, 0, 200);
    vkvg_rel_line_to(ctx, 200, 0);
    vkvg_rel_line_to(ctx, 0, 500);
    vkvg_rel_line_to(ctx, -700, 0);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);

    dashes[0] = 0;
    dashes[1] = 20;
    vkvg_set_dash(ctx, dashes, 2, dash_offset);

    vkvg_set_source_rgb(ctx, 0, 1, 0);

    vkvg_move_to(ctx, 100, 100);
    vkvg_rel_line_to(ctx, 400, 0);
    vkvg_rel_line_to(ctx, 0, 200);
    vkvg_rel_line_to(ctx, 200, 0);
    vkvg_rel_line_to(ctx, 0, 400);
    vkvg_rel_line_to(ctx, -600, 0);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);

    dashes[0] = 80;
    dashes[1] = 20;

    vkvg_set_line_width(ctx, 10);
    vkvg_set_source_rgb(ctx, 1, 0, 0);

    vkvg_set_dash(ctx, dashes, 2, dash_offset);

    vkvg_rectangle(ctx, 200, 300, 200, 200);
    /*vkvg_move_to(ctx, 200,300);
    vkvg_rel_line_to(ctx,200,0);*/
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

TEST(dashes_long_curve) {
    float w = (float)test->app->width;
    float h = (float)test->app->height;

    VkvgContext ctx = vkvg_create(test->surf);

    randomize_color(ctx);

    float x1 = w * rand() / RAND_MAX;
    float y1 = h * rand() / RAND_MAX;
    vkvg_move_to(ctx, x1, y1);

    for (uint32_t i = 0; i < test->app->testSize; i++)
        test->draw_random_curve(ctx);

    vkvg_stroke(ctx);
    vkvg_destroy(ctx);
}
TEST(dashes_long_path) {
    float w = (float)test->app->width;
    float h = (float)test->app->height;

    VkvgContext ctx = vkvg_create(test->surf);

    randomize_color(ctx);
    float x1 = w * rndf();
    float y1 = h * rndf();
    vkvg_move_to(ctx, x1, y1);
    for (uint32_t i = 0; i < test->app->testSize; i++) {
        x1 = w * rndf();
        y1 = h * rndf();
        vkvg_line_to(ctx, x1, y1);
    }
    vkvg_stroke(ctx);
    vkvg_destroy(ctx);
}

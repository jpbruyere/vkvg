#include "VkvgTest.hpp"

TEST(lines_horizontal) {
    float w = (float)test->app->width;
    float h = (float)test->app->height;

    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    for (uint32_t i = 0; i < test->app->testSize; i++) {
        randomize_color(ctx);
        float x1 = w * rndf();
        float y1 = h * rndf();
        float v  = 500.f * rndf();

        vkvg_move_to(ctx, x1, y1);
        vkvg_line_to(ctx, x1 + v, y1);
        vkvg_stroke(ctx);
    }
    vkvg_destroy(ctx);
}
TEST(lines_vertical) {
    float w = (float)test->app->width;
    float h = (float)test->app->height;

    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    for (uint32_t i = 0; i < test->app->testSize; i++) {
        randomize_color(ctx);
        float x1 = w * rndf();
        float y1 = h * rndf();
        float v  = 500.f * rndf();

        vkvg_move_to(ctx, x1, y1);
        vkvg_line_to(ctx, x1, y1 + v);
        vkvg_stroke(ctx);
    }
    vkvg_destroy(ctx);
}
TEST(lines_horzAndVert) {
    float w = (float)test->app->width;
    float h = (float)test->app->height;

    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    for (uint32_t i = 0; i < test->app->testSize; i++) {
        randomize_color(ctx);

        float x1 = w * rndf();
        float y1 = h * rndf();
        float x2 = (w * rndf()) + 1;
        float y2 = (h * rndf()) + 1;

        vkvg_move_to(ctx, x1, y1);
        vkvg_line_to(ctx, x2, y2);
        vkvg_stroke(ctx);
    }
    vkvg_destroy(ctx);
}
TEST(lines_multilines) {
    float w = (float)test->app->width;
    float h = (float)test->app->height;

    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    randomize_color(ctx);

    for (uint32_t i = 0; i < test->app->testSize; i++) {

        float x1 = w * rndf();
        float y1 = h * rndf();
        float x2 = (w * rndf()) + 1;
        float y2 = (h * rndf()) + 1;

        vkvg_move_to(ctx, x1, y1);
        vkvg_line_to(ctx, x2, y2);
    }
    vkvg_stroke(ctx);
    vkvg_destroy(ctx);
}
TEST(lines_multi_segments) {
    float w = (float)test->app->width;
    float h = (float)test->app->height;

    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

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

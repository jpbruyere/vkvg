#include "VkvgTest.hpp"


static float shape_size = 0.2f;

void _shape_fill(VkvgTest* test, shape_t shape) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    for (uint32_t i = 0; i < test->app->testSize; i++) {
        test->draw_random_shape(ctx, shape, shape_size);
        vkvg_fill(ctx);
    }
    vkvg_destroy(ctx);
}
void _shape_stroke(VkvgTest* test, shape_t shape) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    for (uint32_t i = 0; i < test->app->testSize; i++) {
        test->draw_random_shape(ctx, shape, shape_size);
        vkvg_stroke(ctx);
    }
    vkvg_destroy(ctx);
}
void _shape_fill_stroke(VkvgTest* test, shape_t shape) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    for (uint32_t i = 0; i < test->app->testSize; i++) {
        test->draw_random_shape(ctx, shape, shape_size);
        vkvg_fill_preserve(ctx);
        vkvg_stroke(ctx);
    }
    vkvg_destroy(ctx);
}

TEST(rnd_rectangles_fill) { _shape_fill(test, SHAPE_RECTANGLE); }
TEST(rnd_rectangles_stroke) { _shape_stroke(test, SHAPE_RECTANGLE); }
TEST(rnd_rectangles_fill_stroke) { _shape_fill_stroke(test, SHAPE_RECTANGLE); }
TEST(rnd_rounded_rects_fill) { _shape_fill(test, SHAPE_ROUNDED_RECTANGLE); }
TEST(rnd_rounded_rects_stroke) { _shape_stroke(test, SHAPE_ROUNDED_RECTANGLE); }
TEST(rnd_rounded_rects_fill_stroke) { _shape_fill_stroke(test, SHAPE_ROUNDED_RECTANGLE); }
TEST(rnd_circles_fill) { _shape_fill(test, SHAPE_CIRCLE); }
TEST(rnd_circles_stroke) { _shape_stroke(test, SHAPE_CIRCLE); }
TEST(rnd_circles_fill_stroke) { _shape_fill_stroke(test, SHAPE_CIRCLE); }
TEST(rnd_stars_fill) { _shape_fill(test, SHAPE_STAR); }
TEST(rnd_stars_stroke) { _shape_stroke(test, SHAPE_STAR); }
TEST(rnd_stars_fill_stroke) { _shape_fill_stroke(test, SHAPE_STAR); }
TEST(rnd_random_fill) { _shape_fill(test, SHAPE_RANDOM); }
TEST(rnd_random_stroke) { _shape_stroke(test, SHAPE_RANDOM); }
TEST(rnd_random_fill_stroke) { _shape_fill_stroke(test, SHAPE_RANDOM); }

TEST(rnd_fixed_size_square_fill) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    for (uint32_t i = 0; i < test->app->testSize; i++) {
        test->draw_random_square(ctx, 50.0f);
        vkvg_fill(ctx);
    }
    vkvg_destroy(ctx);
}

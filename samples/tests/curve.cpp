#include "VkvgTest.hpp"


//"M80 170   C100 170 160 170 180 170 lZ"
/*TEST(curve2) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    vkvg_set_line_width(ctx, 20);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_move_to(ctx, 80, 170);
    vkvg_curve_to(ctx, 100, 170, 160, 171, 180, 170);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);
    vkvg_destroy(ctx);
}*/
TEST(curve) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    vkvg_set_line_width(ctx, 20);

    vkvg_scale(ctx, 2, 2);
    vkvg_set_line_join(ctx, VKVG_LINE_JOIN_BEVEL);

    // vkvg_arc (ctx, 200, 500, 100, 0, M_PI);

    vkvg_set_source_rgb(ctx, 0.5f, 0, 0);

    /*vkvg_move_to(ctx,100,100);
    vkvg_line_to(ctx,300,100);
    vkvg_line_to(ctx,500,300);
    vkvg_line_to(ctx,300,500);
    //vkvg_arc (ctx, 200, 500, 100, 0, M_PI);
    vkvg_line_to(ctx,300,700);
    vkvg_line_to(ctx,100,500);*/

    /*vkvg_arc(ctx, 300, 300, 100, 0, M_PI);
    vkvg_line_to(ctx,100,200);
    vkvg_line_to(ctx,200,100);
    vkvg_arc(ctx, 250, 100, 50, M_PI, M_PI * 1.5f);
    vkvg_line_to(ctx,350,50);
    vkvg_arc(ctx, 350, 100, 50, M_PI*1.5f, M_PI * 2.0f);

    vkvg_stroke(ctx);
    vkvg_translate(ctx,400,30);

    */
    // vkvg_set_fill_rule(ctx,VKVG_FILL_RULE_EVEN_ODD);
    vkvg_translate(ctx, 200, 30);
    vkvg_arc(ctx, 200, 200, 20, 0, M_PIF * 2);
    // vkvg_stroke(ctx);

    vkvg_set_source_rgba(ctx, 0.5f, 0.0f, 1.0f, 0.5f);
    vkvg_move_to(ctx, 100, 100);
    vkvg_line_to(ctx, 200, 100);
    // vkvg_move_to(ctx,200,100);
    vkvg_curve_to(ctx, 250, 100, 300, 150, 300, 200);
    vkvg_line_to(ctx, 300, 300);
    vkvg_curve_to(ctx, 300, 350, 250, 400, 200, 400);
    vkvg_line_to(ctx, 100, 400);
    vkvg_curve_to(ctx, 50, 400, 10, 350, 10, 300);
    vkvg_line_to(ctx, 10, 200);
    vkvg_curve_to(ctx, 10, 150, 50, 100, 100, 100);
    // vkvg_close_path(ctx);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgba(ctx, 0.1f, 0.3f, 0.7f, 0.5f);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

TEST(curved_rect) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    float x = 50, y = 50, width = 150, height = 140, radius = 30;

    vkvg_scale(ctx, 2, 2);
    // vkvg_rotate(ctx,0.5f);

    vkvg_set_line_width(ctx, 15);
    vkvg_set_source_rgba(ctx, 0, 0.5f, 0.4f, 1);

    if ((radius > height / 2) || (radius > width / 2))
        radius = MIN(height / 2, width / 2);

    vkvg_move_to(ctx, x, y + radius);
    vkvg_arc(ctx, x + radius, y + radius, radius, M_PIF, (float)-M_PI_2);
    vkvg_line_to(ctx, x + width - radius, y);
    vkvg_arc(ctx, x + width - radius, y + radius, radius, (float)-M_PI_2, 0);
    vkvg_line_to(ctx, x + width, y + height - radius);
    vkvg_arc(ctx, x + width - radius, y + height - radius, radius, 0, (float)M_PI_2);
    vkvg_line_to(ctx, x + radius, y + height);
    vkvg_arc(ctx, x + radius, y + height - radius, radius, (float)M_PI_2, M_PIF);
    vkvg_line_to(ctx, x, y + radius);
    vkvg_close_path(ctx);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgba(ctx, 0.5f, 0, 0, 0.5f);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

TEST(curve3) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    vkvg_move_to(ctx, 100, 400);
    vkvg_curve_to(ctx, 100, 100, 600, 700, 600, 400);
    vkvg_curve_to(ctx, 1000, 100, 100, 800, 1000, 800);
    vkvg_curve_to(ctx, 1000, 500, 700, 500, 700, 100);
    vkvg_close_path(ctx);

    // vkvg_set_source_rgba   (ctx, 0.5,0.0,1.0,0.5);
    // vkvg_fill_preserve(ctx);

    vkvg_set_source_rgba(ctx, 1, 0, 0, 1);
    vkvg_set_line_width(ctx, 40);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

static bool fillAndStroke = true;

TEST(curves_stroke_random) {
    float w = (float)test->app->width;
    float h = (float)test->app->height;

    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    for (uint32_t i = 0; i < test->app->testSize; i++) {
        randomize_color(ctx);
        float x1 = rndf() * w;
        float y1 = rndf() * h;

        vkvg_move_to(ctx, x1, y1);
        test->draw_random_curve(ctx);

        vkvg_stroke(ctx);
    }
    vkvg_destroy(ctx);
}

void _long_curv(VkvgTest* test, vkvg_fill_rule_t fill_rule) {
    float w = (float)test->app->width;
    float h = (float)test->app->height;

    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    randomize_color(ctx);
    float x1 = rndf() * w;
    float y1 = rndf() * h;
    vkvg_move_to(ctx, x1, y1);

    for (uint32_t i = 0; i < test->app->testSize; i++) {
        test->draw_random_curve(ctx);
    }

    if (fillAndStroke) {
        vkvg_fill_preserve(ctx);
        randomize_color(ctx);
        vkvg_stroke(ctx);
    } else
        vkvg_fill(ctx);

    vkvg_destroy(ctx);
}

TEST(long_curv_fill_nz) {
    fillAndStroke = false;    
    _long_curv(test,VKVG_FILL_RULE_NON_ZERO);
}
TEST(long_curv_fill_eo) {
    fillAndStroke = false;
    _long_curv(test, VKVG_FILL_RULE_EVEN_ODD);
}
TEST(long_curv_fill_stroke_nz) {
    fillAndStroke = true;
    _long_curv(test, VKVG_FILL_RULE_NON_ZERO);
}
TEST(long_curv_fill_stroke_eo) {
    fillAndStroke = true;
    _long_curv(test, VKVG_FILL_RULE_EVEN_ODD);
}

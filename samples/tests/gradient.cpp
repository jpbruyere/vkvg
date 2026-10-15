#include "VkvgTest.hpp"

VkvgPattern create_grad(VkvgContext ctx, float x) {
    VkvgPattern pat = vkvg_pattern_create_linear(x, 0, 300, 0);
    vkvg_pattern_add_color_stop(pat, 0, 1, 0, 0, 1);
    vkvg_pattern_add_color_stop(pat, 0.5f, 0, 1, 0, 1);
    vkvg_pattern_add_color_stop(pat, 1, 0, 0, 1, 1);
    return pat;
}


void draw_gradient1(VkvgContext cr)
{
    VkvgPattern pat1;
    pat1 = vkvg_pattern_create_linear(0.0, 0.0,  350.0, 350.0);

    double j;
    int count = 1;
    for ( j = 0.1; j < 1; j += 0.1 ) {
        if (( count % 2 ))  {
            vkvg_pattern_add_color_stop(pat1, j, 0, 0, 0, 1);
        } else {
            vkvg_pattern_add_color_stop(pat1, j, 1, 0, 0, 1);
        }
        count++;
    }

    vkvg_rectangle(cr, 20, 20, 300, 100);
    vkvg_set_source(cr, pat1);
    vkvg_fill(cr);

    vkvg_pattern_destroy(pat1);
}
void draw_gradient2(VkvgContext cr)
{
    VkvgPattern pat2;
    pat2 = vkvg_pattern_create_linear(0.0, 0.0,  350.0, 0.0);

    double i;
    int count = 1;
    for ( i = 0.05; i < 0.95; i += 0.075 ) {
        if (( count % 2 ))  {
            vkvg_pattern_add_color_stop(pat2, i, 0, 0, 0, 1);
        } else {
            vkvg_pattern_add_color_stop(pat2, i, 0, 0, 1, 1);
        }
        count++;
    }

    vkvg_rectangle(cr, 20, 140, 300, 100);
    vkvg_set_source(cr, pat2);
    vkvg_fill(cr);

    vkvg_pattern_destroy(pat2);
}
void draw_gradient3(VkvgContext cr)
{
    VkvgPattern pat3;
    pat3 = vkvg_pattern_create_linear(20.0, 260.0, 20.0, 360.0);

    vkvg_pattern_add_color_stop(pat3, 0.1, 0, 0, 0, 1);
    vkvg_pattern_add_color_stop(pat3, 0.5, 1, 1, 0, 1);
    vkvg_pattern_add_color_stop(pat3, 0.9, 0, 0, 0, 1);

    vkvg_rectangle(cr, 20, 260, 300, 100);
    vkvg_set_source(cr, pat3);
    vkvg_fill(cr);

    vkvg_pattern_destroy(pat3);
}

TEST(gradient_paint) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    // vkvg_translate(ctx,100,100);
    VkvgPattern pat = create_grad(ctx, 0);
    // vkvg_pattern_set_extend(pat,VKVG_EXTEND_NONE);
    vkvg_rectangle(ctx, 0, 0, 400, 460);
    vkvg_set_source(ctx, pat);
    vkvg_pattern_destroy(pat);
    vkvg_fill(ctx);

    float x = 100;
    pat     = create_grad(ctx, x);
    // vkvg_pattern_set_extend(pat,VKVG_EXTEND_NONE);
    vkvg_rectangle(ctx, x, 200, 50, 50);
    vkvg_set_source(ctx, pat);
    vkvg_pattern_destroy(pat);
    vkvg_fill(ctx);

    x += 100;

    pat = vkvg_pattern_create_linear(10, 0, 300, 0);
    vkvg_pattern_add_color_stop(pat, 0, 0, 0, 1, 0);
    vkvg_pattern_add_color_stop(pat, 1, 1, 0, 0, 1);
    // vkvg_pattern_set_extend(pat,VKVG_EXTEND_NONE);
    vkvg_rectangle(ctx, 10, 10, 300, 50);
    vkvg_set_source(ctx, pat);
    vkvg_pattern_destroy(pat);
    vkvg_fill(ctx);

    x += 200;

    pat = create_grad(ctx, x);
    // vkvg_pattern_set_extend(pat,VKVG_EXTEND_NONE);
    vkvg_rectangle(ctx, x, 200, 50, 50);
    vkvg_set_source(ctx, pat);
    vkvg_pattern_destroy(pat);
    vkvg_fill(ctx);

    x += 100;

    pat = create_grad(ctx, x);
    // vkvg_pattern_set_extend(pat,VKVG_EXTEND_NONE);
    vkvg_rectangle(ctx, x, 200, 50, 50);
    vkvg_set_source(ctx, pat);
    vkvg_pattern_destroy(pat);
    vkvg_fill(ctx);

    /*vkvg_set_source_rgb(ctx, 0,1,0);
    vkvg_rectangle(ctx, 100,100,200,160);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb(ctx, 0,0,0);
    vkvg_set_line_width(ctx,1.0f);
    vkvg_stroke(ctx);*/

    vkvg_destroy(ctx);
}

TEST(gradient_paint_repeat) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    VkvgPattern pat = create_grad(ctx, 0);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_REPEAT);
    vkvg_set_source(ctx, pat);
    vkvg_pattern_destroy(pat);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
}

TEST(gradient_test0) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);
    VkvgPattern pat = create_grad(ctx, 0);
    vkvg_set_source(ctx, pat);
    vkvg_rectangle(ctx, 100, 100, 200, 200);
    vkvg_set_line_width(ctx, 20);
    // vkvg_fill (ctx);
    // vkvg_paint(ctx);
    vkvg_stroke(ctx);
    vkvg_pattern_destroy(pat);

    vkvg_destroy(ctx);
}
TEST(gradient_test1) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_paint(ctx);

    VkvgPattern pat = vkvg_pattern_create_linear(100, 0, 300, 0);
    vkvg_set_line_width(ctx, 20);
    vkvg_pattern_add_color_stop(pat, 0, 1, 1, 1, 1);
    vkvg_pattern_add_color_stop(pat, 1, 1, 1, 0, 0);
    vkvg_set_source(ctx, pat);
    vkvg_rectangle(ctx, 100, 100, 200, 200);
    vkvg_fill(ctx);
    // vkvg_stroke (ctx);
    vkvg_pattern_destroy(pat);

    vkvg_destroy(ctx);
}

TEST(gradient_transform) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    // vkvg_translate(ctx,-100,-100);

    vkvg_translate(ctx, 200, 100);
    vkvg_rotate(ctx, 0.5f);

    // vkvg_scale(ctx,2,2);
    VkvgPattern pat = vkvg_pattern_create_linear(0, 0, 400, 0);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_NONE);
    vkvg_set_line_width(ctx, 20);
    vkvg_pattern_add_color_stop(pat, 0, 1, 0, 0, 1);
    vkvg_pattern_add_color_stop(pat, 0.5f, 0, 1, 0, 1);
    vkvg_pattern_add_color_stop(pat, 1, 0, 0, 1, 1);
    vkvg_set_source(ctx, pat);
    vkvg_rectangle(ctx, 0, 0, 400, 200);
    // vkvg_fill (ctx);
    vkvg_stroke(ctx);
    // vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);

    vkvg_destroy(ctx);
}

TEST(gradient_alpha) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    vkvg_set_source_rgb(ctx, 0, 1, 0);
    vkvg_paint(ctx);

    VkvgPattern pat = vkvg_pattern_create_linear(10, 0, 300, 0);
    vkvg_pattern_add_color_stop(pat, 1, 1, 0, 0, 0.5);
    vkvg_pattern_add_color_stop(pat, 1, 1, 0, 0, 0.5);

    vkvg_set_source(ctx, pat);
    vkvg_pattern_destroy(pat);
    vkvg_rectangle(ctx, 10, 10, 300, 200);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}

TEST(gradient_radial) {
    VkvgContext ctx = vkvg_create(test->surf);

    VkvgPattern radial = vkvg_pattern_create_radial(250.0f, 250.0f, 0, 300, 300, 260.0f);
    vkvg_pattern_add_color_stop(radial, 0.0, 1, 0, 0, 1);
    vkvg_pattern_add_color_stop(radial, 0.5, 0, 1, 0, 1);
    vkvg_pattern_add_color_stop(radial, 1.0, 0, 0, 0, 1);
    vkvg_set_source(ctx, radial);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(radial);

    vkvg_destroy(ctx);
}

TEST(gradient_cairo) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_set_source_rgb(ctx, 0.7f, 0.7f, 0.7f);
    vkvg_paint(ctx);

    draw_gradient1(ctx);
    draw_gradient2(ctx);
    draw_gradient3(ctx);
    vkvg_destroy(ctx);
}
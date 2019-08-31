#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);
    vkvg_set_line_width(ctx, 10);

    //vkvg_move_to(ctx,400,100);
    vkvg_arc(ctx, 300, 300, 100, 0, M_PI);
    vkvg_line_to(ctx,200,100);
    vkvg_arc(ctx, 250, 100, 50, M_PI, M_PI * 1.5f);
    vkvg_line_to(ctx,350,50);
    vkvg_arc(ctx, 350, 100, 50, M_PI*1.5f, M_PI * 2.0f);

    vkvg_set_source_rgb   (ctx, 1,0,0);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb   (ctx, 1,1,0);
    vkvg_stroke_preserve(ctx);
    vkvg_translate(ctx,50,50);
    vkvg_set_source_rgb   (ctx, 1,0,0);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb   (ctx, 1,1,0);
    vkvg_stroke_preserve(ctx);
    vkvg_translate(ctx,50,50);
    vkvg_set_source_rgb   (ctx, 1,0,0);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb   (ctx, 1,1,0);
    vkvg_stroke_preserve(ctx);
    vkvg_translate(ctx,50,50);
    vkvg_set_source_rgb   (ctx, 1,0,0);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb   (ctx, 1,1,0);
    vkvg_stroke_preserve(ctx);
    vkvg_translate(ctx,50,50);
    vkvg_set_source_rgb   (ctx, 1,0,0);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb   (ctx, 1,1,0);
    vkvg_stroke_preserve(ctx);
    vkvg_translate(ctx,50,50);
    vkvg_set_source_rgb   (ctx, 1,0,0);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb   (ctx, 1,1,0);
    vkvg_stroke_preserve(ctx);
    vkvg_translate(ctx,50,50);
    vkvg_set_source_rgb   (ctx, 1,0,0);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb   (ctx, 1,1,0);
    vkvg_stroke_preserve(ctx);
    vkvg_translate(ctx,100,100);

    /*vkvg_arc(ctx, 200, 200, 10, 0, M_PI*2);


    vkvg_set_source_rgba   (ctx, 0.5,0.0,1.0,0.5);
    vkvg_move_to(ctx,100,100);
    vkvg_line_to(ctx,200,100);
    vkvg_curve_to(ctx,250,100,300,150,300,200);
    vkvg_line_to(ctx,300,300);
    vkvg_curve_to(ctx,300,350,250,400,200,400);
    vkvg_line_to(ctx,100,400);
    vkvg_curve_to(ctx,50,400,10,350,10,300);
    vkvg_line_to(ctx,10,200);
    vkvg_curve_to(ctx,10,150,50,100,100,100);
    vkvg_close_path(ctx);
    //vkvg_curve_to(ctx, 150,100,200,150,200,200);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgba   (ctx, 1,1,1.0,0.5);*/
    //vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

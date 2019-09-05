#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);
    vkvg_set_line_width(ctx, 20);

    vkvg_scale(ctx,2,2);
    vkvg_set_line_join(ctx, VKVG_LINE_JOIN_BEVEL);

    //vkvg_arc (ctx, 200, 500, 100, 0, M_PI);

    vkvg_set_source_rgb   (ctx, 0.5,0,0);


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
    vkvg_translate(ctx,200,30);
    vkvg_arc(ctx, 200, 200, 20, 0, M_PI*2);
    //vkvg_stroke(ctx);

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
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgba   (ctx, 0.1f,0.3,0.7,0.5);
    vkvg_stroke(ctx);


    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

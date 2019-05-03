#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    VkvgPattern pat = vkvg_pattern_create_linear(100,0,300,0);
    vkvg_set_line_width(ctx, 20);
    vkvg_pattern_add_color_stop(pat, 0, 1, 0, 0, 1);
    vkvg_pattern_add_color_stop(pat, 0.5, 0, 1, 0, 1);
    vkvg_pattern_add_color_stop(pat, 1, 0, 0, 1, 1);
    vkvg_set_source (ctx, pat);
    vkvg_rectangle(ctx,100,100,200,200);
    vkvg_fill (ctx);
    //vkvg_stroke (ctx);
    vkvg_pattern_destroy (pat);

    vkvg_destroy(ctx);
}
void test2(){
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgb(ctx,1,0,0);
    vkvg_paint(ctx);

    VkvgPattern pat = vkvg_pattern_create_linear(100,0,300,0);
    vkvg_set_line_width(ctx, 20);
    vkvg_pattern_add_color_stop(pat, 0, 1, 1, 1, 1);
    vkvg_pattern_add_color_stop(pat, 1, 1, 1, 0, 0);
    vkvg_set_source (ctx, pat);
    vkvg_rectangle(ctx,100,100,200,200);
    vkvg_fill (ctx);
    //vkvg_stroke (ctx);
    vkvg_pattern_destroy (pat);

    vkvg_destroy(ctx);
}
int main(int argc, char *argv[]) {

    perform_test (test2, 1024, 768);

    return 0;
}

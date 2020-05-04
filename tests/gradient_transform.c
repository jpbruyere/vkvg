#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    //vkvg_translate(ctx,-100,-100);

    vkvg_translate(ctx,200,100);
    vkvg_rotate(ctx,0.5);

    //vkvg_scale(ctx,2,2);
    VkvgPattern pat = vkvg_pattern_create_linear(0,0,400,0);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_NONE);
    vkvg_set_line_width(ctx, 20);
    vkvg_pattern_add_color_stop(pat, 0, 1, 0, 0, 1);
    vkvg_pattern_add_color_stop(pat, 0.5, 0, 1, 0, 1);
    vkvg_pattern_add_color_stop(pat, 1, 0, 0, 1, 1);
    vkvg_set_source (ctx, pat);
    vkvg_rectangle(ctx,0,0,400,200);
    //vkvg_fill (ctx);
    vkvg_stroke (ctx);
    //vkvg_paint(ctx);
    vkvg_pattern_destroy (pat);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
    PERFORM_TEST (test);
    return 0;
}

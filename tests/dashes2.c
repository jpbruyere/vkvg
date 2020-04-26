#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    const float dashes[] = {0, 8};
    vkvg_set_line_cap(ctx, VKVG_LINE_CAP_ROUND);
    vkvg_set_dash(ctx, dashes, 2, 0);
    vkvg_set_line_width(ctx, 4);

    vkvg_move_to    (ctx, 100, 400);
    vkvg_curve_to   (ctx, 100, 100, 600, 700, 600, 400);
    vkvg_curve_to   (ctx, 1000, 100, 100, 800, 1000, 800);
    vkvg_curve_to   (ctx, 1000, 500, 700, 500, 700, 100);
    //vkvg_close_path(ctx);

    //vkvg_set_source_rgba   (ctx, 0.5,0.0,1.0,0.5);
    //vkvg_fill_preserve(ctx);

    vkvg_set_source_rgba   (ctx, 1,0,0,1);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

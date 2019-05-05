#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_line_width(ctx, 0.5);
    vkvg_set_source_rgb   (ctx, 1,1,1);
    vkvg_paint(ctx);
    vkvg_set_source_rgb   (ctx, 0,0,0);

    vkvg_scale(ctx,3,3);
    vkvg_arc(ctx, 150, 100, 3.5, 0, M_PI*2);
    vkvg_stroke(ctx);
    vkvg_arc(ctx, 200, 200, 10, 0, M_PI*2);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

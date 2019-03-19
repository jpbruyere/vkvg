#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgba(ctx,0.7,0.7,0.7,1);
    vkvg_paint(ctx);

    vkvg_set_source_rgba(ctx,0,1,0,1);
    vkvg_set_line_width(ctx,10);

    vkvg_move_to(ctx,100,100);
    vkvg_line_to(ctx,100,200);
    vkvg_line_to(ctx,100,100);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

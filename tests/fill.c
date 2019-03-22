#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgba(ctx,0.1,0.9,0.1,1.0);
    vkvg_move_to(ctx,100,100);
    vkvg_rel_line_to(ctx,100,100);
    vkvg_rel_line_to(ctx,100,-100);
    vkvg_rel_line_to(ctx,100,200);
    vkvg_rel_line_to(ctx,-100,100);
    vkvg_rel_line_to(ctx,-100,-100);
    vkvg_rel_line_to(ctx,-100,-50);
    //vkvg_close_path(ctx);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

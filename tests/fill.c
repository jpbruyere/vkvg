#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgba(ctx,0.1,0.9,0.1,1.0);
    vkvg_move_to(ctx,10,10);
    vkvg_rel_line_to(ctx,100,100);
    vkvg_rel_line_to(ctx,100,-100);
    vkvg_rel_line_to(ctx,100,400);
//    vkvg_line_to(ctx,400,350);
//    vkvg_line_to(ctx,900,150);
//    vkvg_line_to(ctx,700,450);
//    vkvg_line_to(ctx,900,750);
//    vkvg_line_to(ctx,500,650);
//    vkvg_line_to(ctx,100,800);
//    vkvg_line_to(ctx,150,400);
    vkvg_close_path(ctx);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

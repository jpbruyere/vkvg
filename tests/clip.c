#include "test.h"

void test_clip(){
    VkvgContext ctx = vkvg_create(surf);

    vkvg_move_to(ctx,10,10);
    vkvg_line_to(ctx,400,150);
    vkvg_line_to(ctx,900,10);
    vkvg_line_to(ctx,700,450);
    vkvg_line_to(ctx,900,750);
    vkvg_line_to(ctx,500,650);
    vkvg_line_to(ctx,100,800);
    vkvg_line_to(ctx,150,400);
    vkvg_clip(ctx);

    vkvg_set_source_rgb(ctx,1,0,0);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test_clip, 1024, 768);

    return 0;
}

#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_set_line_width(ctx,10);
    vkvg_rectangle(ctx,100,100,200,200);
    vkvg_stroke(ctx);
    vkvg_set_source_rgba(ctx,0,1,0,1);

    vkvg_save(ctx);

    vkvg_set_source_rgba(ctx,0,1,1,1);
    vkvg_set_line_width(ctx,1);
    vkvg_rectangle(ctx,200,200,200,200);
    vkvg_fill(ctx);

    vkvg_restore(ctx);

    vkvg_rectangle(ctx,100,100,200,200);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 600, 800);

    return 0;
}

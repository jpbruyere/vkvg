#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_paint(ctx);

    /*vkvg_set_line_width(ctx, 5);
    vkvg_set_source_rgba(ctx,0,1,0,1);
    vkvg_rectangle(ctx,100,100,300,200);
    vkvg_fill(ctx);

    vkvg_translate(ctx,300,100);

    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_rectangle(ctx,100,100,300,200);
    vkvg_stroke(ctx);*/
    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

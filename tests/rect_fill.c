#include "test.h"

void test(){
    //vkvg_surface_clear(surf);

    VkvgSurface surf2 = vkvg_surface_create (device,400,400);

    VkvgContext ctx = vkvg_create (surf2);

    vkvg_set_source_rgba(ctx,1.0,0.,0.,0.5f);
    vkvg_paint (ctx);
    vkvg_destroy(ctx);


    //vkvg_surface_write_to_png(surf2, "/home/jp/test.png");

    ctx = vkvg_create(surf);

    /*vkvg_set_source_rgba(ctx,0,0,1,0.5);

    */

    vkvg_set_source_surface(ctx, surf2,0,0);
    //vkvg_rectangle(ctx,10,10,300,300);
    //vkvg_fill(ctx);
    vkvg_paint(ctx);

    //vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);

    /*vkvg_rotate(ctx,0.1f);

    vkvg_set_source_rgba(ctx,0,0,1,0.5);
    vkvg_rectangle(ctx,100,100,200,200);
    vkvg_fill(ctx);


    vkvg_set_source_rgba(ctx,1,0,0,0.5);
    vkvg_rectangle(ctx,200,200,200,200);
    vkvg_fill(ctx);

    vkvg_set_source_rgba(ctx,0,1,0,0.5);
    vkvg_rectangle(ctx,500,500,200,200);
    vkvg_fill(ctx);*/


    vkvg_destroy(ctx);

    vkvg_surface_destroy(surf2);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

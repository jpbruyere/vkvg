#include "test.h"

void test(){

    VkvgSurface surf2 = vkvg_surface_create (device,400,400);;
    VkvgContext ctx = vkvg_create (surf2);

    vkvg_set_source_rgba(ctx,1.0,0.,0.,1.0);
    vkvg_paint (ctx);

    vkvg_destroy (ctx);
    ctx = vkvg_create (surf);

    vkvg_set_source_rgba(ctx,0.1,0.1,0.3,1.0);
    vkvg_paint (ctx);

    //vkvg_set_source_surface(ctx,surf2,0,0);

    //VkvgPattern pat = vkvg_get_source (ctx);
    VkvgPattern pat = vkvg_pattern_create_for_surface(surf2);
    vkvg_pattern_set_extend (pat,VKVG_EXTEND_REFLECT);
    vkvg_set_source(ctx,pat);
    //vkvg_paint (ctx);
    //vkvg_set_source_rgba(ctx,0,1,0,1.0);
    vkvg_rectangle(ctx,100,100,200,200);
    vkvg_fill(ctx);

    vkvg_destroy (ctx);
    vkvg_surface_destroy (surf2);
    vkvg_pattern_destroy (pat);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

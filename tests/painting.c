#include "test.h"

static VkvgSurface surf2;

void test(){

    VkvgContext ctx  = vkvg_create (surf);

    /*vkvg_set_source_surface(ctx,surf2,0,0);
    vkvg_paint (ctx);
    vkvg_destroy (ctx);

    return;*/

    //vkvg_set_source_rgba(ctx,0.1,0.1,0.3,1.0);
    //vkvg_paint (ctx);

    for (int i=0; i<10; i++) {
        vkvg_translate(ctx,50,50);

        vkvg_save(ctx);
/*
        vkvg_rectangle(ctx,0,0,200,200);
        vkvg_clip_preserve(ctx);
        vkvg_set_operator(ctx,VKVG_OPERATOR_CLEAR);
        vkvg_fill(ctx);
        vkvg_set_operator(ctx,VKVG_OPERATOR_OVER);*/

        vkvg_set_source_rgba(ctx,0.0,0.0,0.0,0.3f);
        vkvg_move_to(ctx,0,0);
        vkvg_line_to(ctx,0,200);
        vkvg_set_line_width(ctx,10);
        vkvg_stroke(ctx);
        vkvg_set_source_surface(ctx,surf2,0,0);
        vkvg_paint (ctx);

        vkvg_restore(ctx);
    }

    //VkvgPattern pat = vkvg_get_source (ctx);
    /*VkvgPattern pat = vkvg_pattern_create_for_surface(surf2);
    vkvg_pattern_set_extend (pat,VKVG_EXTEND_REFLECT);
    vkvg_set_source(ctx,pat);*/
    //vkvg_set_source_rgba(ctx,0,1,0,1.0);
    //vkvg_rectangle(ctx,100,100,200,200);
    //vkvg_fill(ctx);

    vkvg_destroy (ctx);

    //vkvg_pattern_destroy (pat);
}

int main(int argc, char *argv[]) {

    init_test(1024, 768);

    surf2 = vkvg_surface_create (device,400,400);

    VkvgContext ctx = vkvg_create (surf2);

    vkvg_set_source_rgba(ctx,1.0,0.,0.,0.2f);
    vkvg_paint (ctx);
    vkvg_set_source_rgba(ctx,1.0,1.0,0.,0.5f);
    vkvg_move_to(ctx,10,10);
    vkvg_line_to(ctx,200,200);
    vkvg_set_line_width(ctx,10);
    vkvg_stroke(ctx);
    vkvg_set_source_rgba(ctx,1.0,1.0,1.0,0.6f);
    vkvg_rectangle(ctx,0,0,400,400);
    vkvg_stroke(ctx);

    vkvg_destroy (ctx);

    run_test_func(test, 1024, 768);

    vkvg_surface_destroy (surf2);

    clear_test();
    return 0;
}

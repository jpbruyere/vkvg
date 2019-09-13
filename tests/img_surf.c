#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx,VKVG_FILL_RULE_NON_ZERO);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "data/miroir.jpg");

    vkvg_set_operator(ctx, VKVG_OPERATOR_CLEAR);
    vkvg_paint(ctx);
    vkvg_set_operator(ctx, VKVG_OPERATOR_OVER);
    /*vkvg_translate(ctx,200,200);



    vkvg_set_line_width(ctx,20.f);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_arc(ctx,200,200,200,0,2.f*M_PI);
    vkvg_new_sub_path(ctx);
    vkvg_arc(ctx,200,200,100,0,2.f*M_PI);

    vkvg_set_source_surface(ctx, imgSurf, 00, 00);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgba(ctx,0.2,0.3,0.8,1);

    vkvg_stroke(ctx);*/

    //vkvg_translate(ctx,200,200);
    vkvg_rotate(ctx,0.1f);

    //vkvg_scale(ctx,0.5,0.5);
    vkvg_set_source_rgb(ctx,0,0,1);
    vkvg_paint(ctx);

    vkvg_set_line_width(ctx,20.f);
    vkvg_set_source_surface(ctx, imgSurf, 100, 100);
    vkvg_rectangle(ctx,100,100,200,200);
    vkvg_stroke(ctx);
    //vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

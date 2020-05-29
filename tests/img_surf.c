#include "test.h"
void paint () {
    VkvgContext ctx = vkvg_create(surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "data/miroir.jpg");

    vkvg_set_source_surface(ctx, imgSurf, 00, 00);
    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void paint_offset () {
    VkvgContext ctx = vkvg_create(surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "data/miroir.jpg");

    vkvg_set_source_surface(ctx, imgSurf, 100, 100);
    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void paint_pattern () {
    VkvgContext ctx = vkvg_create(surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "data/miroir.jpg");
    VkvgPattern pat = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void paint_pattern_repeat () {
    VkvgContext ctx = vkvg_create(surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "data/miroir.jpg");
    VkvgPattern pat = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_pattern_set_extend(pat,VKVG_EXTEND_REPEAT);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void paint_pattern_pad () {
    VkvgContext ctx = vkvg_create(surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "data/miroir.jpg");
    VkvgPattern pat = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_pattern_set_extend(pat,VKVG_EXTEND_PAD);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}

void test(){
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx,VKVG_FILL_RULE_EVEN_ODD);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "data/miroir.jpg");

    vkvg_translate(ctx,200,200);
    //vkvg_rotate(ctx,M_PI_4);

    vkvg_set_line_width(ctx,20.f);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_arc(ctx,200,200,200,0,2.f*M_PIF);
    vkvg_new_sub_path(ctx);
    vkvg_arc(ctx,200,200,100,0,2.f*M_PIF);

    vkvg_set_source_surface(ctx, imgSurf, 00, 00);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgba(ctx,0.2f,0.3f,0.8f,1);

    vkvg_stroke(ctx);

    vkvg_surface_destroy(imgSurf);

    vkvg_destroy(ctx);
}

int main() {
    PERFORM_TEST (paint);
    PERFORM_TEST (paint_offset);
    PERFORM_TEST (paint_pattern);
    PERFORM_TEST (paint_pattern_repeat);
    PERFORM_TEST (paint_pattern_pad);
    PERFORM_TEST (test);

    return 0;
}

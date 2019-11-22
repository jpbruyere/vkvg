#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    float x = 50, y = 50, width = 150, height = 140, radius = 30;

    vkvg_scale(ctx,2,2);

    vkvg_set_line_width(ctx,15);
    vkvg_set_source_rgba(ctx, 0, 0.5f, 0.4f, 1);


    if ((radius > height / 2) || (radius > width / 2))
        radius = MIN(height / 2, width / 2);

    vkvg_move_to(ctx, x, y + radius);
    //vkvg_arc(ctx, x + radius, y + radius, radius, M_PIF, (float)-M_PI_2);
    vkvg_line_to(ctx, x + width - radius, y);
    //vkvg_arc(ctx, x + width - radius, y + radius, radius, (float)-M_PI_2, 0);
    /*vkvg_line_to(ctx, x + width, y + height - radius);
    vkvg_arc(ctx, x + width - radius, y + height - radius, radius, 0, (float)M_PI_2);
    vkvg_line_to(ctx, x + radius, y + height);
    vkvg_arc(ctx, x + radius, y + height - radius, radius, (float)M_PI_2, M_PIF);
    vkvg_line_to(ctx, x, y + radius);
    vkvg_close_path(ctx);
    vkvg_fill_preserve(ctx);*/
    vkvg_set_source_rgba(ctx,0.5,0,0,0.5);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

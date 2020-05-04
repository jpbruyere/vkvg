#include "test.h"

void draw_growing_circles (VkvgContext ctx, float y, int count) {
    float x = 2;
    for (int i=1; i<count; i++) {
        x += 0.5*i;
        //vkvg_set_source_rgb   (ctx, 1,0,1);
        vkvg_arc(ctx, x + 2, y, 0.5 * i, 0, M_PI*1.5);
        //vkvg_set_source_rgb   (ctx, 0,1,1);
        //vkvg_arc_negative(ctx, x + 2, y, 0.5 * i, M_PI/2,0);
        x += 0.5*i + 5;
    }
}

void scaled_up() {
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgb   (ctx, 1,1,1);
    vkvg_paint(ctx);
    vkvg_set_source_rgb   (ctx, 0,0,0);

    vkvg_scale(ctx,100,100);
    vkvg_arc(ctx, 2, 2, 0.5, 0, M_PI/2);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}
void sizes() {
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgb   (ctx, 1,1,1);
    vkvg_paint(ctx);
    vkvg_set_source_rgb   (ctx, 0,0,0);

    draw_growing_circles (ctx, 100, 40);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}
void test(){
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgb   (ctx, 1,1,1);
    vkvg_paint(ctx);
    vkvg_set_source_rgb   (ctx, 0,0,0);

    vkvg_set_source_rgb   (ctx, 1,0,1);
    vkvg_set_line_width(ctx, 5.0);
    vkvg_arc(ctx, 100, 100, 20, 0, M_PI/2);
    vkvg_stroke(ctx);

    vkvg_set_source_rgb   (ctx, 0,1,1);
    vkvg_arc_negative(ctx, 100, 100, 20, 0, M_PI/2);
    vkvg_stroke(ctx);

    vkvg_set_source_rgb   (ctx, 1,0,1);
    vkvg_arc(ctx, 100, 200, 20, M_PI/2, 0);
    vkvg_stroke(ctx);

    vkvg_set_source_rgb   (ctx, 0,1,1);
    vkvg_arc_negative(ctx, 100, 200, 20, M_PI/2, 0);
    vkvg_stroke(ctx);

    vkvg_set_source_rgb   (ctx, 0,0,1);
    vkvg_set_line_width(ctx, 10.0);
    vkvg_arc(ctx, 350, 100, 40, 0, M_PI*2);
    vkvg_stroke(ctx);

    vkvg_set_source_rgb   (ctx, 0,1,0);
    vkvg_set_line_width(ctx, 1.0);
    vkvg_arc(ctx, 150, 100, 3.5, 0, M_PI*2);
    vkvg_stroke(ctx);
    vkvg_arc(ctx, 200, 200, 10, 0, M_PI*2);
    vkvg_fill(ctx);

    vkvg_set_source_rgb   (ctx, 1,0,0);
    vkvg_scale(ctx,3,3);
    vkvg_arc(ctx, 150, 100, 3.5, 0, M_PI*2);
    vkvg_stroke(ctx);
    vkvg_arc(ctx, 200, 200, 10, 0, M_PI*2);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}

int main() {
    PERFORM_TEST (sizes);
    PERFORM_TEST (scaled_up);
    PERFORM_TEST (test);
    return 0;
}

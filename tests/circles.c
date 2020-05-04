#include "test.h"

void draw_growing_circles (VkvgContext ctx, float y, int count) {
    float x = 2;
    for (int i=1; i<count; i++) {
        x += 0.5*i;
        vkvg_arc(ctx, x + 2, y, 0.5 * i, 0, M_PI*2);
        x += 0.5*i + 5;
    }
}

void scaled_up() {
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgb   (ctx, 1,1,1);
    vkvg_paint(ctx);
    vkvg_set_source_rgb   (ctx, 0,0,0);

    vkvg_scale(ctx,100,100);
    vkvg_arc(ctx, 2, 2, 0.5, 0, M_PI*2);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}
void sizes() {
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgb   (ctx, 1,1,1);
    vkvg_paint(ctx);
    vkvg_set_source_rgb   (ctx, 0,0,0);

    draw_growing_circles (ctx, 100, 40);
    vkvg_fill (ctx);

    draw_growing_circles (ctx, 200, 40);
    vkvg_stroke(ctx);

    vkvg_set_source_rgba  (ctx, 0,0,1,0.4);
    draw_growing_circles (ctx, 300, 40);
    vkvg_fill_preserve (ctx);
    vkvg_set_line_width(ctx,5);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

int main() {
    PERFORM_TEST (sizes);
    PERFORM_TEST (scaled_up);
    return 0;
}

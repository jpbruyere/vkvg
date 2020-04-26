#include "test.h"
static float offset = 0;
void test(){
    offset += 0.1f;
    vkvg_surface_clear(surf);

    VkvgContext ctx = vkvg_create(surf);
    //const float dashes[] = {160.0f, 80};
    float dashes[] = {700.0f, 30};
    //const float dashes[] = {50, 40};
    vkvg_set_line_cap(ctx, VKVG_LINE_CAP_ROUND);
    vkvg_set_dash(ctx, dashes, 2, offset);
    vkvg_set_line_width(ctx, 20);
    vkvg_set_source_rgb(ctx, 0, 0, 1);

    vkvg_move_to (ctx, 150, 50);
    vkvg_rel_line_to (ctx, 500, 0);
    vkvg_rel_line_to (ctx, 0, 200);
    vkvg_rel_line_to (ctx, 200, 0);
    vkvg_rel_line_to (ctx, 0, 500);
    vkvg_rel_line_to (ctx, -700, 0);
    vkvg_close_path(ctx);
    vkvg_stroke (ctx);

    dashes[0] = 0;
    dashes[1] = 30;
    vkvg_set_dash(ctx, dashes, 2, offset);

    vkvg_set_source_rgb(ctx, 0, 1, 0);

    vkvg_move_to (ctx, 200, 100);
    vkvg_rel_line_to (ctx, 400, 0);
    vkvg_rel_line_to (ctx, 0, 200);
    vkvg_rel_line_to (ctx, 200, 0);
    vkvg_rel_line_to (ctx, 0, 400);
    vkvg_rel_line_to (ctx, -600, 0);
    vkvg_close_path(ctx);
    vkvg_stroke (ctx);

    vkvg_destroy(ctx);
}

int main() {

    perform_test (test, 1024, 768);

    return 0;
}

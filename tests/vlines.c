#include "test.h"

void test(){
    vkvg_surface_clear(surf);
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    srand((unsigned) currentTime.tv_usec);
    const float w = 1024.f;
    const float h = 800.f;

    VkvgContext ctx = vkvg_create(surf);
    //vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
    vkvg_set_line_width(ctx,1);
    //vkvg_set_line_join(ctx,VKVG_LINE_JOIN_BEVEL);
    //vkvg_set_source_rgba(ctx,1.0,0.0,0.0,0.1);

    for (int i=0; i<250; i++) {
        randomize_color(ctx);
        float x1 = trunc( ((float)w*rand())/RAND_MAX );
        float y1 = trunc( ((float)h*rand())/RAND_MAX );
        float v = trunc( ((float)500*rand())/RAND_MAX ) + 1;

        vkvg_move_to (ctx, x1, y1);
        vkvg_line_to (ctx, x1, y1 + v);
        vkvg_stroke (ctx);
    }
    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

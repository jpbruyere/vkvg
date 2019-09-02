#include "test.h"

void test(){
    vkvg_surface_clear(surf);
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    srand((unsigned) currentTime.tv_usec);
    const float w = 1024.f;

    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);

    //vkvg_set_line_join(ctx,VKVG_LINE_JOIN_BEVEL);

    for (int i=0; i<iterations; i++) {
        randomize_color(ctx);

        float x = w*rand()/RAND_MAX;
        float y = w*rand()/RAND_MAX;
        float r = 0.1f*w*rand()/RAND_MAX;

        vkvg_arc(ctx, x, y, r, 0, M_PI * 2.f);
        vkvg_fill(ctx);

        if (i%50==0)
            vkvg_flush(ctx);
    }
    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

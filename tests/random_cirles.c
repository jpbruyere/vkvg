#include "test.h"

void test(){
    vkvg_surface_clear(surf);
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    srand((unsigned) currentTime.tv_usec);
    const float w = 800.f;

    VkvgContext ctx = vkvg_create(surf);
    //vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);

    vkvg_set_line_width(ctx, 1.0f);
    //vkvg_set_line_join(ctx,VKVG_LINE_JOIN_BEVEL);

    for (uint i=0; i<test_size; i++) {
        randomize_color(ctx);

        float r = 0.5f*w*rand()/RAND_MAX;
        float x = truncf(0.5f * w*rand()/RAND_MAX + r);
        float y = truncf(0.5f * w*rand()/RAND_MAX + r);

        vkvg_arc(ctx, x, y, r, 0, M_PI * 2.0f);
        //vkvg_fill(ctx);
        vkvg_fill_preserve(ctx);
        randomize_color(ctx);
        vkvg_stroke(ctx);
    }
    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

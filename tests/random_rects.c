#include "test.h"

void test(){
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    srand((unsigned) currentTime.tv_usec);
    const float w = 800.f;

    vkvg_surface_clear(surf);

    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,0,0,0,1);
    vkvg_paint(ctx);

    //vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);
    //vkvg_set_line_width(ctx,10);

    for (uint j=0;j<2;j++) {
        for (uint i=0; i<test_size/2; i++) {
            randomize_color(ctx);

            float x = truncf(0.5f*w*rand()/RAND_MAX);
            float y = truncf(0.5f*w*rand()/RAND_MAX);
            float z = truncf((0.5f*w*rand()/RAND_MAX)+1.f);
            float v = truncf((0.5f*w*rand()/RAND_MAX)+1.f);

            vkvg_rectangle(ctx, x, y, z, v);
            vkvg_fill(ctx);
        }
        vkvg_flush(ctx);
    }
    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 800, 600);

    return 0;
}

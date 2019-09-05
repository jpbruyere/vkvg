#include "test.h"

void test(){
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    srand((unsigned) currentTime.tv_usec);
    const float w = 1024.f;

    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);
    vkvg_set_line_width(ctx, 2.0f);

    vkvg_clear(ctx);
    //vkvg_set_line_join(ctx,VKVG_LINE_JOIN_BEVEL);

    for (uint i=0; i<test_size; i++) {
        randomize_color(ctx);

        float x = truncf(0.8f*w*rand()/RAND_MAX);
        float y = truncf(0.8f*w*rand()/RAND_MAX);
        float z = truncf((0.2f*w*rand()/RAND_MAX)+1.f);
        float v = truncf((0.2f*w*rand()/RAND_MAX)+1.f);

        vkvg_rectangle(ctx, x, y, z, v);
        //vkvg_stroke(ctx);
        vkvg_fill(ctx);

        /*if (i%250==0)
            vkvg_flush(ctx);*/
    }
    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 800, 600);

    return 0;
}

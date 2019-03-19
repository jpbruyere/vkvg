#include "test.h"

void test(){
    vkvg_surface_clear(surf);
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    srand((unsigned) currentTime.tv_usec);
    const float w = 1024.f;

    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_line_width(ctx,1);
    for (int i=0; i<5000; i++) {
        randomize_color(ctx);
        float x = trunc( (0.5*(float)w*rand())/RAND_MAX );
        float y = trunc( (0.5*(float)w*rand())/RAND_MAX );
        float z = trunc( (0.5*(float)w*rand())/RAND_MAX ) + 1;
        float v = trunc( (0.5*(float)w*rand())/RAND_MAX ) + 1;

        vkvg_rectangle(ctx, x+1, y+1, z, v);
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

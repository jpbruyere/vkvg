#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);


    vkvg_set_line_width(ctx, 1);
    vkvg_set_source_rgb   (ctx, 1,1,1);
    vkvg_paint(ctx);
    vkvg_set_source_rgb   (ctx, 0,0,0);

    vkvg_scale(ctx,10.0f,10.0f);
    //vkvg_scale(ctx,2.0f,2.0f);

    float x = 10;
    float r = 1;
    while (x < 600){
        vkvg_arc(ctx, x+0.5f, 10.5f,   r, 0, M_PI*2);
        x += r;
        r ++;
        x += r + 5;
    }

    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

int main() {

    perform_test (test, 1024, 768);

    return 0;
}

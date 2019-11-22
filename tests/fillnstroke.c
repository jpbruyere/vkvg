#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    float x = 50, y = 150, dx = 150, dy = 140;

    vkvg_scale(ctx,2,2);

    vkvg_set_line_width(ctx,40);
    vkvg_set_source_rgba(ctx,0,1,0,1);


    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_ROUND);
    //vkvg_rectangle(ctx,x,y,dx,dy);


    vkvg_move_to(ctx,x,y);
    vkvg_rel_line_to(ctx,50,-30);
    vkvg_rel_line_to(ctx,50,0);
    vkvg_rel_line_to(ctx,50,30);
    vkvg_rel_line_to(ctx,0,60);
    vkvg_rel_line_to(ctx,-50,70);
    vkvg_rel_line_to(ctx,-50,0);
    vkvg_rel_line_to(ctx,-50,-70);
    vkvg_line_to(ctx,x,y);
    //vkvg_close_path(ctx);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgba(ctx,0.5,0,0,0.5);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

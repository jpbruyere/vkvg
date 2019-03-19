#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    float x = 20, y = 20, dx = 30, dy = 60;

    //vkvg_scale(ctx,5,5);
    vkvg_set_line_width(ctx,26);
    vkvg_set_source_rgba(ctx,0,0,0,1);
    vkvg_move_to(ctx,x,y);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_stroke(ctx);
    vkvg_set_line_cap(ctx,VKVG_LINE_CAP_SQUARE);
    vkvg_move_to(ctx,x+dx,y);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_stroke(ctx);
    vkvg_set_line_cap(ctx,VKVG_LINE_CAP_ROUND);
    vkvg_move_to(ctx,x+2*dx,y);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_rel_move_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,dx,dy);
    vkvg_rel_move_to(ctx,dx,-dy/2);
    vkvg_rel_line_to(ctx,dx,0);
    vkvg_rel_move_to(ctx,dx,dy/2);
    vkvg_rel_line_to(ctx,dx,-dy);
    vkvg_rel_move_to(ctx,dx,dy);
    vkvg_rel_line_to(ctx,0,-dy);
    vkvg_rel_move_to(ctx,2*dx,dy);
    vkvg_rel_line_to(ctx,-dx,-dy);
    vkvg_rel_move_to(ctx,3*dx,dy/2);
    vkvg_rel_line_to(ctx,-dx,0);
    //vkvg_rel_line_to(ctx,0,-dy);
    //vkvg_rel_move_to(ctx,dx,dy/2);
    //vkvg_rel_line_to(ctx,dx,0);
    vkvg_stroke(ctx);

    vkvg_set_line_cap(ctx,VKVG_LINE_CAP_BUTT);
    vkvg_set_line_width(ctx,1);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_move_to(ctx,x,y);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_rel_move_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_rel_move_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

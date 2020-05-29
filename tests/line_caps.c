#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,0.9f,0.9f,0.9f,1);
    vkvg_paint(ctx);

    float x = 20, y = 20, dx = 40, dy = 60;

    //vkvg_scale(ctx,5,5);
    vkvg_set_line_width(ctx,30);
    vkvg_set_source_rgba(ctx,0.0,0.0,0,1);
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
    vkvg_rel_move_to(ctx,dx,-dy/2.f);
    vkvg_rel_line_to(ctx,dx,0);
    vkvg_rel_move_to(ctx,dx,dy/2.f);
    vkvg_rel_line_to(ctx,dx,-dy);
    vkvg_rel_move_to(ctx,dx,dy);
    vkvg_rel_line_to(ctx,0,-dy);
    vkvg_rel_move_to(ctx,dx*2.f,dy);
    vkvg_rel_line_to(ctx,-dx,-dy);
    vkvg_rel_move_to(ctx,dx*3.f,dy/2.f);
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

int main() {
    PERFORM_TEST (test);
    return 0;
}

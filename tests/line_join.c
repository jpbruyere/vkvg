#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    float x = 250, y = 150, dx = 150, dy = 140;

    //vkvg_scale(ctx,2,2);

    vkvg_set_line_width(ctx,100);
    vkvg_set_source_rgba(ctx,0,1,0,1);


    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_ROUND);
    //vkvg_rectangle(ctx,x,y,dx,dy);

    vkvg_move_to(ctx,x,y);
    vkvg_rel_line_to(ctx,-50,30);
    vkvg_rel_line_to(ctx,0,60);
    vkvg_rel_line_to(ctx,50,30);
    /*
    vkvg_rel_line_to(ctx,50,-30);
    vkvg_rel_line_to(ctx,50,0);
    vkvg_rel_line_to(ctx,50,30);
    vkvg_rel_line_to(ctx,0,60);
    vkvg_rel_line_to(ctx,-50,70);
    vkvg_rel_line_to(ctx,-50,0);
    vkvg_rel_line_to(ctx,-50,-70);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);

    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_move_to(ctx,x+200,y);
    vkvg_rel_line_to(ctx,50,70);
    vkvg_rel_line_to(ctx,50,0);
    vkvg_rel_line_to(ctx,50,-70);
    vkvg_rel_line_to(ctx,0,-60);
    vkvg_rel_line_to(ctx,-50,-30);
    vkvg_rel_line_to(ctx,-50,0);
    vkvg_rel_line_to(ctx,-50,30);*/
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);

    vkvg_set_source_rgba(ctx,0,0,1,1);
    vkvg_move_to(ctx,x+250,y);
    vkvg_rel_line_to(ctx,50,-30);
    vkvg_rel_line_to(ctx,50,0);
    vkvg_rel_line_to(ctx,50,30);
    vkvg_rel_line_to(ctx,0,60);
    vkvg_rel_line_to(ctx,-50,70);
    vkvg_rel_line_to(ctx,-50,0);
    vkvg_rel_line_to(ctx,-50,-70);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);

//    vkvg_rel_line_to(ctx,dx,-dy);
//    vkvg_rel_line_to(ctx,dx,dy);
//    vkvg_stroke(ctx);
//    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_BEVEL);
//    vkvg_rel_move_to(ctx,-dx*2,abs(dy*1.5));
//    vkvg_rel_line_to(ctx,dx,-dy);
//    vkvg_rel_line_to(ctx,dx,dy);
//    vkvg_stroke(ctx);
//    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_ROUND);
//    vkvg_rel_move_to(ctx,-dx*2,abs(dy*1.5));
//    vkvg_rel_line_to(ctx,dx,-dy);
//    vkvg_rel_line_to(ctx,dx,dy);
//    vkvg_stroke(ctx);
//    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_MITER);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
    PERFORM_TEST (test);
    return 0;
}

#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_line_width(ctx, 1);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_move_to(ctx,200.5,200.5);
    vkvg_line_to(ctx,400.5,200.5);
    vkvg_line_to(ctx,400.5,400.5);
    vkvg_line_to(ctx,200.5,400.5);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);

    vkvg_set_source_rgba(ctx,0,1,0,1);
    vkvg_move_to(ctx,300.5,300.5);
    vkvg_line_to(ctx,500.5,300.5);
    vkvg_line_to(ctx,500.5,500.5);
    vkvg_line_to(ctx,300.5,500.5);
    vkvg_stroke(ctx);

    //vkvg_set_source_rgba(ctx,0,0.2,0.35,1);
    //vkvg_fill(ctx);

    vkvg_set_source_rgba(ctx,0.5,1,0,1);
    vkvg_move_to(ctx,320.5,320.5);
    vkvg_line_to(ctx,520.5,320.5);
    vkvg_line_to(ctx,520.5,520.5);
    vkvg_line_to(ctx,320.5,520.5);
    //vkvg_close_path(ctx);
    vkvg_stroke(ctx);
    vkvg_set_line_width(ctx, 40);
    vkvg_set_source_rgba(ctx,0.5,0.6,1,1.0);
    vkvg_move_to(ctx,700,475);
    vkvg_line_to(ctx,400,475);
    vkvg_stroke(ctx);
    vkvg_set_source_rgba(ctx,0,0.5,0.5,0.5);
    vkvg_move_to(ctx,300,200);
    vkvg_arc(ctx, 200,200,100,0, M_PI);
    vkvg_stroke(ctx);

    vkvg_set_line_width(ctx, 20);
    vkvg_set_source_rgba(ctx,0.1,0.1,0.1,0.5);
    vkvg_move_to(ctx,100,60);
    vkvg_line_to(ctx,400,600);
    vkvg_stroke(ctx);

    vkvg_set_source_rgba(ctx,1,1,1,1);
    vkvg_set_line_width(ctx, 1);
    vkvg_rectangle(ctx,600.5,200.5,100,60);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

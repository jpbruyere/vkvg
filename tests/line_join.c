#include "test.h"

void test() {
    VkvgContext ctx = vkvg_create(surf);

    float x = 250, y = 150;

    // vkvg_scale(ctx,2,2);

    vkvg_set_line_width(ctx, 100);
    vkvg_set_source_rgba(ctx, 0, 1, 0, 1);

    vkvg_set_line_join(ctx, VKVG_LINE_JOIN_ROUND);
    // vkvg_rectangle(ctx,x,y,dx,dy);

    vkvg_move_to(ctx, x, y);
    vkvg_rel_line_to(ctx, -50, 30);
    vkvg_rel_line_to(ctx, 0, 60);
    vkvg_rel_line_to(ctx, 50, 30);
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

    vkvg_set_source_rgba(ctx, 0, 0, 1, 1);
    vkvg_move_to(ctx, x + 250, y);
    vkvg_rel_line_to(ctx, 50, -30);
    vkvg_rel_line_to(ctx, 50, 0);
    vkvg_rel_line_to(ctx, 50, 30);
    vkvg_rel_line_to(ctx, 0, 60);
    vkvg_rel_line_to(ctx, -50, 70);
    vkvg_rel_line_to(ctx, -50, 0);
    vkvg_rel_line_to(ctx, -50, -70);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);

    //    float dx = 150, dy = 140;
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

void test2() {
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
    vkvg_set_line_width(ctx, 30);

    vkvg_set_line_join(ctx, VKVG_LINE_JOIN_ROUND);

    // vkvg_arc (ctx, 200, 500, 100, 0, M_PI);

    vkvg_translate(ctx, -50, -50);

    vkvg_set_source_rgb(ctx, 0.5, 0, 0);

    for (int j = 0; j < 2; j++) {
        int i = 0;
        vkvg_move_to(ctx, 100, 100);
        for (i = 0; i < 5; i++) {
            vkvg_rel_line_to(ctx, 70, 50);
            vkvg_rel_line_to(ctx, -70, 50);
        }
        vkvg_stroke(ctx);

        vkvg_move_to(ctx, 200, 600);
        for (i = 0; i < 5; i++) {
            vkvg_rel_line_to(ctx, 70, -50);
            vkvg_rel_line_to(ctx, -70, -50);
        }
        vkvg_stroke(ctx);

        vkvg_move_to(ctx, 400, 100);
        for (i = 0; i < 5; i++) {
            vkvg_rel_line_to(ctx, -70, 50);
            vkvg_rel_line_to(ctx, 70, 50);
        }
        vkvg_stroke(ctx);

        vkvg_move_to(ctx, 500, 600);
        for (i = 0; i < 5; i++) {
            vkvg_rel_line_to(ctx, -70, -50);
            vkvg_rel_line_to(ctx, 70, -50);
        }
        vkvg_stroke(ctx);
        vkvg_set_line_join(ctx, VKVG_LINE_JOIN_BEVEL);
        vkvg_translate(ctx, 500, 0);
    }

    vkvg_destroy(ctx);
}

void test3() {
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
    vkvg_set_line_width(ctx, 30);

    vkvg_set_line_join(ctx, VKVG_LINE_JOIN_ROUND);

    // vkvg_arc (ctx, 200, 500, 100, 0, M_PI);

    vkvg_translate(ctx, -50, -50);

    vkvg_set_source_rgb(ctx, 0.5, 0, 0);

    for (int j = 0; j < 2; j++) {
        int i = 0;
        vkvg_move_to(ctx, 100, 100);
        for (i = 0; i < 4; i++) {
            vkvg_rel_line_to(ctx, 50, 70);
            vkvg_rel_line_to(ctx, 50, -70);
        }
        vkvg_stroke(ctx);

        vkvg_move_to(ctx, 500, 200);
        for (i = 0; i < 4; i++) {
            vkvg_rel_line_to(ctx, -50, 70);
            vkvg_rel_line_to(ctx, -50, -70);
        }
        vkvg_stroke(ctx);

        vkvg_move_to(ctx, 100, 400);
        for (i = 0; i < 4; i++) {
            vkvg_rel_line_to(ctx, 50, -70);
            vkvg_rel_line_to(ctx, 50, 70);
        }
        vkvg_stroke(ctx);

        vkvg_move_to(ctx, 500, 500);
        for (i = 0; i < 4; i++) {
            vkvg_rel_line_to(ctx, -50, -70);
            vkvg_rel_line_to(ctx, -50, 70);
        }
        vkvg_stroke(ctx);
        vkvg_set_line_join(ctx, VKVG_LINE_JOIN_BEVEL);
        vkvg_translate(ctx, 450, 0);
    }

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
    no_test_size = true;
    PERFORM_TEST(test, argc, argv);
    PERFORM_TEST(test2, argc, argv);
    PERFORM_TEST(test3, argc, argv);
    return 0;
}

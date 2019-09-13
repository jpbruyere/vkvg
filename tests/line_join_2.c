#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    //vkvg_set_fill_rule(ctx, VKVG_FILL_RULE);
    vkvg_set_line_width(ctx, 30);

    //vkvg_set_line_join(ctx, VKVG_LINE_JOIN_ROUND);

    //vkvg_arc (ctx, 200, 500, 100, 0, M_PI);

    vkvg_translate(ctx,-50,-50);

    vkvg_set_source_rgba   (ctx, 0.5,0,0,1);

    for (int j=0;j<2;j++) {
         int i=0;
         vkvg_move_to(ctx,100,100);
         for (i=0;i<5;i++) {
             vkvg_rel_line_to(ctx,70,50);
             vkvg_rel_line_to(ctx,-70,50);
         }
         vkvg_stroke(ctx);

         vkvg_move_to(ctx,200,600);
         for (i=0;i<5;i++) {
             vkvg_rel_line_to(ctx,70,-50);
             vkvg_rel_line_to(ctx,-70,-50);
         }
         vkvg_stroke(ctx);

         vkvg_move_to(ctx,400,100);
         for (i=0;i<5;i++) {
             vkvg_rel_line_to(ctx,-70,50);
             vkvg_rel_line_to(ctx,70,50);
         }
         vkvg_stroke(ctx);

         vkvg_move_to(ctx,500,600);
         for (i=0;i<5;i++) {
             vkvg_rel_line_to(ctx,-70,-50);
             vkvg_rel_line_to(ctx,70,-50);
         }
         vkvg_stroke(ctx);
         //vkvg_set_line_join(ctx, VKVG_LINE_JOIN_BEVEL);
         vkvg_translate(ctx,500,0);
    }

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

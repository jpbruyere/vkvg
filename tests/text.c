#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgba(ctx,0.9,0.9,0.9,1);
    vkvg_paint(ctx);

    int size = 19;
    int penY = 50;
    int penX = 10;

    /*vkvg_rectangle(ctx,30,0,100,400);
    vkvg_clip(ctx);*/

    //vkvg_select_font_face(ctx, "/usr/local/share/fonts/DroidSansMono.ttf");
    //vkvg_select_font_face(ctx, "/usr/share/fonts/truetype/unifont/unifont.ttf");

    vkvg_set_font_size(ctx,12);
    vkvg_select_font_face(ctx, "droid");
    vkvg_font_extents_t fe;
    vkvg_font_extents (ctx,&fe);
    vkvg_move_to(ctx, penX,penY);
    vkvg_set_source_rgba(ctx,0.1,0.1,0.1,1);
    vkvg_text_extents_t te;
    vkvg_text_extents(ctx,"abcdefghijk",&te);
    vkvg_show_text (ctx,"abcdefghijk");
    penX+= te.x_advance;
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"*abcdefghijk2");
    penY+=2*size;

    vkvg_select_font_face(ctx, "times");
    vkvg_set_source_rgba(ctx,0.1,0.1,0.2,1);
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"abcdefghijklmnopqrstuvwxyz");
    penY+=size;

    vkvg_select_font_face(ctx, "droid");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"lmnopqrstuvwxyz123456789");
    penY+=size;

    vkvg_select_font_face(ctx, "times:bold");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"abcdefghijklmnopqrstuvwxyz");
    penY+=size;

    vkvg_select_font_face(ctx, "droid");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    penY+=size;

    vkvg_select_font_face(ctx, "arial:italic");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"abcdefghijklmnopqrstuvwxyz");
    penY+=size;

    vkvg_select_font_face(ctx, "arial");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    penY+=size;
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"this is a test");
    penY+=size;
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"this is another test to see if label is working");
    penY+=size;

    vkvg_select_font_face(ctx, "mono");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    penY+=size;

    vkvg_move_to(ctx, 80,400);
    vkvg_show_text (ctx,"Ленивый рыжий кот");

    /*vkvg_move_to(ctx, 150,250);
    vkvg_show_text (ctx,"test string é€");
    vkvg_move_to(ctx, 150,300);
    vkvg_show_text (ctx,"كسول الزنجبيل القط");
    vkvg_move_to(ctx, 150,350);
    vkvg_show_text (ctx,"懶惰的姜貓");*/

    //vkvg_show_text (ctx,"ABCDABCD");
    //vkvg_show_text (ctx,"j");

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

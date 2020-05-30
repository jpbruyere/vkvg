#include "test.h"

static const char* txt = "The quick brown fox jumps over the lazy dog";

void print(VkvgContext ctx, float penY, uint32_t size) {
	vkvg_set_font_size(ctx,size);
	vkvg_move_to(ctx, 10,penY);
	vkvg_show_text (ctx,txt);
}

void test2() {
	VkvgContext ctx = vkvg_create(surf);

	vkvg_color_t bg = {0.0,0.0,0.0,1};
	vkvg_color_t fg = {1.0f,1.0f,1.0f,1};

	vkvg_set_source_rgba(ctx,bg.r,bg.g,bg.b,bg.a);
	vkvg_paint(ctx);
	vkvg_set_source_rgba(ctx,fg.r,fg.g,fg.b,fg.a);
	//vkvg_select_font_face(ctx, "droid");
	vkvg_select_font_face(ctx, "times");


	float penY = 10.f;

	for (uint32_t size=4;size<39;size++) {
		print(ctx,(float)penY,size);
		penY+=size;
	}

	vkvg_destroy(ctx);
}
void test1() {
	VkvgContext ctx = vkvg_create(surf);

	vkvg_color_t fg = {0.0,0.0,0.0,1};
	vkvg_color_t bg = {0.9f,0.9f,0.9f,1};

	vkvg_set_source_rgba(ctx,bg.r,bg.g,bg.b,bg.a);
	vkvg_paint(ctx);
	vkvg_set_source_rgba(ctx,fg.r,fg.g,fg.b,fg.a);

	uint32_t size = 8;
	float penY = 100.f;

	vkvg_set_font_size(ctx,size);

	vkvg_select_font_face(ctx, "mono");
	vkvg_move_to(ctx, 100.f,penY);
	vkvg_show_text (ctx,txt);

	penY += 1.2f * size;

	vkvg_select_font_face(ctx, "times");
	vkvg_move_to(ctx, 100.f, penY);
	vkvg_show_text (ctx,txt);

	penY += 1.2f * size;

	vkvg_select_font_face(ctx, "arial:italic");
	vkvg_move_to(ctx, 100.f, penY);
	vkvg_show_text (ctx,txt);

	vkvg_destroy(ctx);
}
void test(){
	VkvgContext ctx = vkvg_create(surf);

	//vkvg_color_t fg = {0.2,0.2,0.2,1};
	vkvg_color_t fg = {0.0,0.0,0.0,1};
	vkvg_color_t bg = {1.0f,1.0f,1.0f,1};
	vkvg_set_source_rgba(ctx,bg.r,bg.g,bg.b,bg.a);
	vkvg_paint(ctx);

	float size = 19;
	float penY = 50;
	float penX = 10;

	/*vkvg_rectangle(ctx,30,0,100,400);
	vkvg_clip(ctx);*/

	//vkvg_select_font_face(ctx, "/usr/local/share/fonts/DroidSansMono.ttf");
	//vkvg_select_font_face(ctx, "/usr/share/fonts/truetype/unifont/unifont.ttf");

	vkvg_set_font_size(ctx,12);
	vkvg_select_font_face(ctx, "droid");
	vkvg_font_extents_t fe;
	vkvg_font_extents (ctx,&fe);
	vkvg_move_to(ctx, penX,penY);
	vkvg_set_source_rgba(ctx,fg.r,fg.g,fg.b,fg.a);
	vkvg_text_extents_t te;
	vkvg_text_extents(ctx,"abcdefghijk",&te);
	vkvg_show_text (ctx,"abcdefghijk");
	penX+= te.x_advance;
	vkvg_move_to(ctx, penX,penY);
	vkvg_show_text (ctx,"*abcdefghijk2");
	penY+=2.f*size;

	vkvg_select_font_face(ctx, "times");
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

	PERFORM_TEST (test, argc, argv);
	PERFORM_TEST (test1, argc, argv);
	PERFORM_TEST (test2, argc, argv);

	return 0;
}

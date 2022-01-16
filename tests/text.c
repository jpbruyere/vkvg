#include "test.h"

static const char* txt = "The quick brown fox jumps over the lazy dog";

void print(VkvgContext ctx, float penY, uint32_t size) {
	vkvg_set_font_size(ctx,size);
	vkvg_move_to(ctx, 10, penY);
	vkvg_show_text (ctx,txt);
}
void print_boxed(VkvgContext ctx, const char* text, float penX, float penY, uint32_t size) {
	vkvg_set_font_size(ctx,size);
	vkvg_text_extents_t te = {0};
	vkvg_text_extents(ctx,text,&te);
	vkvg_font_extents_t fe = {0};
	vkvg_font_extents(ctx, &fe);

	vkvg_move_to(ctx, penX, penY);
	vkvg_rectangle(ctx, penX, penY -fe.ascent, te.width, fe.height);
	vkvg_set_source_rgb(ctx,0.2f,0.2f,0.7f);
	vkvg_fill(ctx);

	vkvg_move_to(ctx, penX, penY);
	vkvg_set_source_rgb(ctx,1,1,1);
	vkvg_show_text (ctx,text);
}
void print_unboxed(VkvgContext ctx, const char* text, float penX, float penY, uint32_t size) {
	vkvg_set_font_size(ctx,size);
	vkvg_move_to(ctx, penX, penY);
	vkvg_set_source_rgb(ctx,1,1,1);
	vkvg_show_text (ctx,text);
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

	for (uint32_t size = 4; size < 39; size++) {
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

	vkvg_set_font_size (ctx, 12);
	vkvg_select_font_face (ctx, "droid");
	vkvg_font_extents_t fe;
	vkvg_font_extents (ctx, &fe);
	vkvg_move_to (ctx, penX,penY);
	vkvg_set_source_rgba (ctx, fg.r, fg.g, fg.b, fg.a);
	vkvg_text_extents_t te;
	vkvg_text_extents (ctx, "abcdefghijk", &te);
	vkvg_show_text (ctx, "abcdefghijk");
	penX += te.x_advance;
	vkvg_move_to(ctx, penX,penY);
	vkvg_show_text (ctx,"*abcdefghijk2");
	penY += 2.f*size;

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

	vkvg_move_to(ctx, 150,250);
	vkvg_show_text (ctx,"test string é€");
	vkvg_move_to(ctx, 150,300);
	vkvg_show_text (ctx,"كسول الزنجبيل القط");
	vkvg_move_to(ctx, 150,350);
	vkvg_show_text (ctx,"懶惰的姜貓");

	//vkvg_show_text (ctx,"ABCDABCD");
	//vkvg_show_text (ctx,"j");

	vkvg_destroy(ctx);
}
void single_font_and_size () {
	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);
	for (uint32_t i=0; i<test_size; i++) {
		randomize_color(ctx);
		float x = rndf() * test_width;
		float y = rndf() * test_height;
		vkvg_select_font_face(ctx,"mono");
		vkvg_set_font_size(ctx, 20);
		vkvg_move_to(ctx,x,y);
		vkvg_show_text(ctx,"This is a test string!");
	}
	vkvg_destroy(ctx);
}
void simple_text () {
	VkvgContext ctx = vkvg_create(surf);

	vkvg_set_source_rgb		(ctx, 0, 0, 0);
	vkvg_paint				(ctx);
	vkvg_set_source_rgb		(ctx, 1, 1, 1);

	vkvg_load_font_from_path (ctx, "data/DancingScript-Regular.ttf", "dancing");
	print_boxed				(ctx, "abcdefghijklmnopqrstuvwxyz", 20,60,20);
	print_boxed				(ctx, "ABC", 20,160,60);
	vkvg_select_font_face	(ctx, "mono");
	print_boxed				(ctx, "This is a test string!", 20, 250, 20);
	print_boxed				(ctx, "ANOTHER ONE TO CHECK..", 20, 350, 20);

	vkvg_destroy			(ctx);

}
void font_file_path () {
	VkvgContext ctx = vkvg_create(surf);

	vkvg_set_source_rgb		(ctx, 0, 0, 0);
	vkvg_paint				(ctx);
	vkvg_set_source_rgb		(ctx, 1, 1, 1);
	vkvg_load_font_from_path (ctx, "data/DancingScript-Regular.ttf", "droid");
	print_boxed				(ctx, "This is a test string!", 50,20,12);
	print_boxed				(ctx, "This is a test string!", 50,50,20);
	print_boxed				(ctx, "ANOTHER ONE TO CHECK..", 50,80,20);
	print_boxed				(ctx, "this is another string to check if ligature are well set", 10,120,20);
	vkvg_destroy			(ctx);
}

void random_size () {
	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);
	vkvg_select_font_face(ctx,"mono");
	for (uint32_t i=0; i<test_size; i++) {
		randomize_color(ctx);
		float x = rndf() * test_width;
		float y = rndf() * test_height;
		uint32_t c = (uint32_t)(rndf() * 120)+1;

		vkvg_set_font_size(ctx, c);
		vkvg_move_to(ctx,x,y);
		vkvg_show_text(ctx,"This is a test string!");
	}
	vkvg_destroy(ctx);
}
const char* const fonts[] =
	{ "mono", "droid", "times", "arial", "times:bold"};

void random_font_and_size () {
	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);

	for (uint32_t i=0; i<test_size; i++) {
		randomize_color(ctx);
		float x = rndf() * test_width;
		float y = rndf() * test_height;
		uint32_t c = (uint32_t)(rndf() * 80)+1;
		uint32_t f = (uint32_t)(rndf() * 4);

		vkvg_set_font_size(ctx, c);
		vkvg_select_font_face(ctx, fonts[f]);
		vkvg_move_to(ctx,x,y);
		vkvg_show_text(ctx,"This is a test string!");
	}
	vkvg_destroy(ctx);
}
void proto_sinaitic () {
	VkvgContext ctx = vkvg_create(surf);

	vkvg_set_source_rgb		(ctx, 0, 0, 0);
	vkvg_paint				(ctx);
	vkvg_set_source_rgb		(ctx, 1, 1, 1);

	vkvg_load_font_from_path (ctx, "data/Proto-Sinaitic15.ttf", "sinaitic");
	print_boxed				(ctx, "hwhy", 100, 150, 60);

	vkvg_destroy			(ctx);
}
int main(int argc, char *argv[]) {
	no_test_size = true;
	//vkvg_log_level = VKVG_LOG_INFO;
	PERFORM_TEST (simple_text, argc, argv);
	PERFORM_TEST (font_file_path, argc, argv);
	PERFORM_TEST (single_font_and_size, argc, argv);
	PERFORM_TEST (random_size, argc, argv);
	PERFORM_TEST (random_font_and_size, argc, argv);
	PERFORM_TEST (test, argc, argv);
	PERFORM_TEST (test1, argc, argv);
	PERFORM_TEST (test2, argc, argv);
	PERFORM_TEST (proto_sinaitic, argc, argv);

	return 0;
}

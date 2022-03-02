#include "test.h"

const float lineWidth = 20.f;
void arc_line (VkvgContext ctx){
	vkvg_arc_negative (ctx,80,80,70, M_PIF*1.5f, M_PIF);
	vkvg_line_to (ctx, 70, 70);

	vkvg_stroke(ctx);
}
void line_arc (VkvgContext ctx){
	vkvg_move_to(ctx, 70, 70);
	vkvg_line_to (ctx, 10, 80);
	vkvg_arc (ctx,80,80,70, M_PIF, M_PIF*1.5f);
	vkvg_stroke(ctx);
}
void small_long (VkvgContext ctx){
	vkvg_move_to(ctx, 10, 40);
	vkvg_line_to (ctx, 10, 50);
	vkvg_line_to (ctx, 70, 45);
	vkvg_stroke(ctx);
}
void tests() {
	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);
	/*vkvg_set_line_width(ctx, 1);
	vkvg_set_source_rgba(ctx,0.5,0.5,0.5,1);
	for (int x=100; x<test_width ; x+=100) {
		vkvg_move_to(ctx, x, 0);
		vkvg_line_to(ctx, x, test_height);
	}
	for (int y=100; y<test_height; y+=100) {
		vkvg_move_to(ctx, 0, y);
		vkvg_line_to(ctx, test_width, y);
	}

	vkvg_stroke(ctx);*/

	vkvg_set_line_width(ctx, lineWidth);
	vkvg_set_source_rgba(ctx,1,0,0,1);

	arc_line (ctx);
	vkvg_translate(ctx, 100, 0);
	line_arc (ctx);
	vkvg_translate(ctx, 100, 0);
	small_long(ctx);

	vkvg_destroy(ctx);
}


int main(int argc, char *argv[]) {
	no_test_size = true;
	PERFORM_TEST (tests, argc, argv);
	return 0;
}

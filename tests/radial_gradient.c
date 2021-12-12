#include "test.h"

void test(){
	VkvgContext ctx = vkvg_create(surf);

	VkvgPattern radial = vkvg_pattern_create_radial (200,200,50,50,55,200);
	vkvg_pattern_add_color_stop(radial, 0.0,1,0,0,1);
	vkvg_pattern_add_color_stop(radial, 0.5,0,1,0,1);
	vkvg_pattern_add_color_stop(radial, 1.0,0,0,1,1);
	vkvg_set_source(ctx, radial);
	/*vkvg_rectangle(ctx,10,10,400,400);
	vkvg_fill(ctx);*/
	vkvg_paint(ctx);
	vkvg_pattern_destroy(radial);

	/*vkvg_set_source_rgba(ctx,0.7f,0.7f,0.7f,1);
	vkvg_paint(ctx);

	vkvg_set_source_rgba(ctx,0,1,0,1);
	vkvg_set_line_width(ctx,10);

	vkvg_move_to(ctx,100,100);
	vkvg_line_to(ctx,100,200);
	vkvg_line_to(ctx,100,100);
	vkvg_stroke(ctx);*/

	vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
	no_test_size = true;
	PERFORM_TEST (test, argc, argv);

	return 0;
}

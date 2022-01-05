#include "test.h"

void test(){
	VkvgContext ctx = vkvg_create(surf);

	vkvg_move_to (ctx, 100, 100);
	vkvg_rel_line_to (ctx, 50, -80);
	vkvg_rel_line_to (ctx, 50, 80);
	vkvg_close_path (ctx);

	vkvg_move_to (ctx, 300, 100);
	vkvg_rel_line_to (ctx, 50, -80);
	vkvg_rel_line_to (ctx, 50, 80);
	vkvg_close_path (ctx);

	vkvg_set_line_width (ctx, 10.0);
	vkvg_set_source_rgb (ctx, 0, 0, 1);
	vkvg_fill_preserve (ctx);
	//vkvg_fill(ctx);
	vkvg_set_source_rgb (ctx, 1, 0, 0);
	vkvg_stroke (ctx);

	vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
	no_test_size = true;
	PERFORM_TEST (test, argc, argv);
	return 0;
}

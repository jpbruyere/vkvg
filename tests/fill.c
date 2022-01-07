#include "test.h"

void test(){
	VkvgContext ctx = _initCtx(surf);

	vkvg_set_source_rgba(ctx,0.1f,0.9f,0.1f,1.0f);
	vkvg_move_to(ctx,100,100);
	vkvg_rel_line_to(ctx,50,200);
	vkvg_rel_line_to(ctx,150,-100);
	vkvg_rel_line_to(ctx,100,200);
	vkvg_rel_line_to(ctx,-100,100);
	vkvg_rel_line_to(ctx,-10,-100);
	vkvg_rel_line_to(ctx,-190,-50);
	vkvg_close_path(ctx);

	vkvg_fill(ctx);

	vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
	no_test_size = true;
	PERFORM_TEST (test, argc, argv);
	return 0;
}

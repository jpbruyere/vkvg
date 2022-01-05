#include "test.h"

void test(){
	VkvgContext ctx = vkvg_create(surf);
	vkvg_save(ctx);
	vkvg_set_line_width(ctx,30);
	vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);

	vkvg_set_source_rgba(ctx,0.1f,0.9f,0.1f,1.0f);
	vkvg_move_to(ctx,100,100);
	vkvg_rel_line_to(ctx,200,0);
	vkvg_rel_line_to(ctx,0,150);
	vkvg_rel_line_to(ctx,-200,0);
	vkvg_close_path(ctx);

	vkvg_move_to(ctx,150,150);
	vkvg_rel_line_to(ctx,0,50);
	vkvg_rel_line_to(ctx,100,0);
	vkvg_rel_line_to(ctx,0,-50);
	vkvg_close_path(ctx);

	vkvg_fill(ctx);

	vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
	//vkvg_log_level = VKVG_LOG_FULL;
	no_test_size = true;
	PERFORM_TEST (test, argc, argv);
	return 0;
}

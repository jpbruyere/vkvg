#include "test.h"

void test(){
	VkvgContext ctx = vkvg_create(surf);

	vkvg_set_line_width(ctx, 1);
	vkvg_set_source_rgba(ctx,1,0,0,1);
	vkvg_move_to(ctx,200.5f,200.5f);
	vkvg_line_to(ctx,400.5f,200.5f);
	vkvg_line_to(ctx,400.5f,400.5f);
	vkvg_line_to(ctx,200.5f,400.5f);
	vkvg_close_path(ctx);
	vkvg_stroke(ctx);

	vkvg_set_source_rgba(ctx,0,1,0,1);
	vkvg_move_to(ctx,300.5f,300.5f);
	vkvg_line_to(ctx,500.5f,300.5f);
	vkvg_line_to(ctx,500.5f,500.5f);
	vkvg_line_to(ctx,300.5f,500.5f);
	vkvg_stroke(ctx);

	//vkvg_set_source_rgba(ctx,0,0.2,0.35,1);
	//vkvg_fill(ctx);

	vkvg_set_source_rgba(ctx,0.5f,1,0,1);
	vkvg_move_to(ctx,320.5f,320.5f);
	vkvg_line_to(ctx,520.5f,320.5f);
	vkvg_line_to(ctx,520.5f,520.5f);
	vkvg_line_to(ctx,320.5f,520.5f);
	//vkvg_close_path(ctx);
	vkvg_stroke(ctx);
	vkvg_set_line_width(ctx, 40);
	vkvg_set_source_rgba(ctx,0.5f,0.6f,1,1.0f);
	vkvg_move_to(ctx,700,475);
	vkvg_line_to(ctx,400,475);
	vkvg_stroke(ctx);
	vkvg_set_source_rgba(ctx,0,0.5f,0.5f,0.5f);
	vkvg_move_to(ctx,300,200);
	vkvg_arc(ctx, 200,200,100,0, M_PIF);
	vkvg_stroke(ctx);

	vkvg_set_line_width(ctx, 20);
	vkvg_set_source_rgba(ctx,0.1f,0.1f,0.1f,0.5f);
	vkvg_move_to(ctx,100,60);
	vkvg_line_to(ctx,400,600);
	vkvg_stroke(ctx);

	vkvg_set_source_rgba(ctx,1,1,1,1);
	vkvg_set_line_width(ctx, 1);
	vkvg_rectangle(ctx,600.5f,200.5f,100,60);
	vkvg_stroke(ctx);

	vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
	no_test_size = true;
	PERFORM_TEST (test, argc, argv);
	return 0;
}

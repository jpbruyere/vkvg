#include "test.h"

void compositing(){
	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);

	vkvg_set_source_rgba(ctx, 1,0,0,0.5f);
	vkvg_rectangle(ctx,100,100,200,200);
	vkvg_fill(ctx);

	vkvg_set_source_rgba(ctx, 0,0,1,0.5f);
	vkvg_rectangle(ctx,200,200,200,200);
	vkvg_fill(ctx);

	vkvg_destroy(ctx);
}
void opacity(){
	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);

	vkvg_set_source_rgba(ctx, 1,0,0,1.0f);
	vkvg_rectangle(ctx,100,100,200,200);
	vkvg_fill(ctx);

	vkvg_set_opacity(ctx,0.5f);

	vkvg_set_source_rgba(ctx, 0,0,1,1.0f);
	vkvg_rectangle(ctx,200,200,200,200);
	vkvg_fill(ctx);

	vkvg_destroy(ctx);
}


int main(int argc, char *argv[]) {
	no_test_size = true;
	PERFORM_TEST (compositing, argc, argv);
	PERFORM_TEST (opacity, argc, argv);
	return 0;
}

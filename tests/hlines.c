#include "test.h"

void test(){
	vkvg_surface_clear(surf);
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);

	srand((unsigned) currentTime.tv_usec);
	const float w = 1024.f;
	const float h = 800.f;

	VkvgContext ctx = vkvg_create(surf);
	//vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
	vkvg_set_line_width(ctx,1);
	//vkvg_set_line_join(ctx,VKVG_LINE_JOIN_BEVEL);

	for (uint32_t i=0; i<test_size; i++) {
		randomize_color(ctx);
		float x1 = w*rand()/RAND_MAX;
		float y1 = h*rand()/RAND_MAX;
		float v = 500.f*rand()/RAND_MAX;

		vkvg_move_to (ctx, x1, y1);
		vkvg_line_to (ctx, x1 + v, y1);
		vkvg_stroke (ctx);
	}
	vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    PERFORM_TEST (test);

	return 0;
}

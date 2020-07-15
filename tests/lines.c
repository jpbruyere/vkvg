#include "test.h"

static float line_width = 1.f;
static vkvg_line_cap_t line_cap = VKVG_LINE_CAP_BUTT;

void horizontal() {
	float w = (float)test_width;
	float h = (float)test_height;

	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);
	vkvg_set_line_width (ctx,line_width);
	vkvg_set_line_cap(ctx, line_cap);

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
void vertical() {
	float w = (float)test_width;
	float h = (float)test_height;

	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);
	vkvg_set_line_width (ctx,line_width);

	for (uint32_t i=0; i<test_size; i++) {
		randomize_color(ctx);
		float x1 = w*rand()/RAND_MAX;
		float y1 = h*rand()/RAND_MAX;
		float v = 500.f*rand()/RAND_MAX;

		vkvg_move_to (ctx, x1, y1);
		vkvg_line_to (ctx, x1, y1 + v);
		vkvg_stroke (ctx);
	}
	vkvg_destroy(ctx);
}
void horzAndVert(){
	float w = (float)test_width;
	float h = (float)test_height;

	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);
	vkvg_set_line_width (ctx,line_width);

	for (uint32_t i=0; i<test_size; i++) {
		randomize_color(ctx);

		float x1 = w*rand()/RAND_MAX;
		float y1 = h*rand()/RAND_MAX;
		float x2 = (w*rand()/RAND_MAX) + 1;
		float y2 = (h*rand()/RAND_MAX) + 1;

		vkvg_move_to (ctx, x1, y1);
		vkvg_line_to (ctx, x2, y2);
		vkvg_stroke (ctx);
	}
	vkvg_destroy(ctx);
}
void multilines(){
	float w = (float)test_width;
	float h = (float)test_height;

	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);
	vkvg_set_line_width (ctx,line_width);

	randomize_color(ctx);

	for (uint32_t i=0; i<test_size; i++) {

		float x1 = w*rand()/RAND_MAX;
		float y1 = h*rand()/RAND_MAX;
		float x2 = (w*rand()/RAND_MAX) + 1;
		float y2 = (h*rand()/RAND_MAX) + 1;

		vkvg_move_to (ctx, x1, y1);
		vkvg_line_to (ctx, x2, y2);
	}
	vkvg_stroke (ctx);
	vkvg_destroy(ctx);
}


int main(int argc, char *argv[]) {
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);

	srand((unsigned) currentTime.tv_usec);

	PERFORM_TEST(horizontal, argc, argv);
	PERFORM_TEST(vertical, argc, argv);
	PERFORM_TEST(horzAndVert, argc, argv);
	PERFORM_TEST(multilines, argc, argv);
	return 0;
}

#include "test.h"

void draw_growing_circles (VkvgContext ctx, float y, int count) {
	float x = 2;
	for (int i=1; i<count; i++) {
		x += 0.5f*i;
		//vkvg_set_source_rgb   (ctx, 1,0,1);
		vkvg_arc(ctx, x + 2, y, 0.5f * i, 0, M_PIF*1.5f);
		//vkvg_set_source_rgb   (ctx, 0,1,1);
		//vkvg_arc_negative(ctx, x + 2, y, 0.5 * i, M_PI/2,0);
		x += 0.5f*i + 5;
	}
}

void scaled_up() {
	VkvgContext ctx = vkvg_create(surf);

	vkvg_set_source_rgb   (ctx, 1,1,1);
	vkvg_paint(ctx);
	vkvg_set_source_rgb   (ctx, 0,0,0);

	vkvg_scale(ctx,10,10);
	vkvg_arc(ctx, 20, 20, 2.0f, 0, M_PIF/2.f);
	vkvg_stroke(ctx);

	vkvg_destroy(ctx);
}
void sizes() {
	VkvgContext ctx = vkvg_create(surf);

	vkvg_set_source_rgb   (ctx, 1,1,1);
	vkvg_paint(ctx);
	vkvg_set_source_rgb   (ctx, 0,0,0);

	draw_growing_circles (ctx, 100, 40);
	vkvg_stroke(ctx);

	vkvg_destroy(ctx);
}
void test(){
	VkvgContext ctx = vkvg_create(surf);

	vkvg_set_source_rgb   (ctx, 1,1,1);
	vkvg_paint(ctx);
	vkvg_set_source_rgb   (ctx, 0,0,0);

	vkvg_set_source_rgb   (ctx, 1,0,1);
	vkvg_set_line_width(ctx, 5.0);
	vkvg_arc(ctx, 100, 100, 20, 0, M_PIF/2);
	vkvg_stroke(ctx);

	vkvg_set_source_rgb   (ctx, 0,1,1);
	vkvg_arc_negative(ctx, 100, 100, 20, 0, M_PIF/2);
	vkvg_stroke(ctx);

	vkvg_set_source_rgb   (ctx, 1,0,1);
	vkvg_arc(ctx, 100, 200, 20, M_PIF/2, 0);
	vkvg_stroke(ctx);

	vkvg_set_source_rgb   (ctx, 0,1,1);
	vkvg_arc_negative(ctx, 100, 200, 20, M_PIF/2, 0);
	vkvg_stroke(ctx);

	vkvg_set_source_rgb   (ctx, 0,0,1);
	vkvg_set_line_width(ctx, 10.0);
	vkvg_arc(ctx, 350, 100, 40, 0, M_PIF*2);
	vkvg_stroke(ctx);

	vkvg_set_source_rgb   (ctx, 0,1,0);
	vkvg_set_line_width(ctx, 1.0);
	vkvg_arc(ctx, 150, 100, 3.5, 0, M_PIF*2);
	vkvg_stroke(ctx);
	vkvg_arc(ctx, 200, 200, 10, 0, M_PIF*2);
	vkvg_fill(ctx);

	vkvg_set_source_rgb   (ctx, 1,0,0);
	vkvg_scale(ctx,3,3);
	vkvg_arc(ctx, 150, 100, 3.5, 0, M_PIF*2);
	vkvg_stroke(ctx);
	vkvg_arc(ctx, 200, 200, 10, 0, M_PIF*2);
	vkvg_fill(ctx);

	vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
	no_test_size = true;
	PERFORM_TEST (sizes, argc, argv);
	PERFORM_TEST (scaled_up, argc, argv);
	PERFORM_TEST (test, argc, argv);	
	return 0;
}

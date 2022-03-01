#include "test.h"

void drawRandomRect (VkvgContext ctx, float s) {
	float w = (float)test_width;
	float h = (float)test_height;
	randomize_color(ctx);

	float x = truncf(w*rndf());
	float y = truncf(h*rndf());

	vkvg_rectangle(ctx, x, y, s, s);
}
void fixedSizeRects(){
	VkvgContext ctx = _initCtx ();
	for (uint32_t i=0; i<test_size; i++) {
		drawRandomRect(ctx, 50.0f);
		vkvg_fill (ctx);
	}
	vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
    PERFORM_TEST (fixedSizeRects, argc, argv);
	return 0;
}

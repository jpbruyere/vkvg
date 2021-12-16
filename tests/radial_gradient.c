#include "test.h"

void test(){
	VkvgContext ctx = vkvg_create(surf);

	VkvgPattern radial = vkvg_pattern_create_radial (250.0f,250.0f,0,300,300,260.0f);
	vkvg_pattern_add_color_stop(radial, 0.0,1,0,0,1);
	vkvg_pattern_add_color_stop(radial, 0.5,0,1,0,1);
	vkvg_pattern_add_color_stop(radial, 1.0,0,0,0,1);
	vkvg_set_source(ctx, radial);
	vkvg_paint(ctx);
	vkvg_pattern_destroy(radial);

	vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
	no_test_size = true;
	PERFORM_TEST (test, argc, argv);

	return 0;
}

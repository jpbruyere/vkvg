#include "test.h"

void test(){
	dash_offset += 0.1f;

	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);
	//const float dashes[] = {160.0f, 80};
	float dashes[] = {700.0f, 30};
	//const float dashes[] = {50, 40};
	vkvg_set_line_cap(ctx, VKVG_LINE_CAP_ROUND);
	vkvg_set_dash(ctx, dashes, 2, dash_offset);
	vkvg_set_line_width(ctx, 20);
	vkvg_set_source_rgb(ctx, 0, 0, 1);

	vkvg_move_to (ctx, 150, 50);
	vkvg_rel_line_to (ctx, 500, 0);
	vkvg_rel_line_to (ctx, 0, 200);
	vkvg_rel_line_to (ctx, 200, 0);
	vkvg_rel_line_to (ctx, 0, 500);
	vkvg_rel_line_to (ctx, -700, 0);
	vkvg_close_path(ctx);
	vkvg_stroke (ctx);

	dashes[0] = 0;
	dashes[1] = 30;
	vkvg_set_dash(ctx, dashes, 2, dash_offset);

	vkvg_set_source_rgb(ctx, 0, 1, 0);

	vkvg_move_to (ctx, 200, 100);
	vkvg_rel_line_to (ctx, 400, 0);
	vkvg_rel_line_to (ctx, 0, 200);
	vkvg_rel_line_to (ctx, 200, 0);
	vkvg_rel_line_to (ctx, 0, 400);
	vkvg_rel_line_to (ctx, -600, 0);
	vkvg_close_path(ctx);
	vkvg_stroke (ctx);

	vkvg_destroy(ctx);
}


void _long_curve () {
	float w = (float)test_width;
	float h = (float)test_height;

	VkvgContext ctx = _initCtx();

	randomize_color(ctx);
	float x1 = w*rand()/RAND_MAX;
	float y1 = h*rand()/RAND_MAX;
	vkvg_move_to (ctx, x1, y1);

	for (uint32_t i=0; i<test_size; i++)
		draw_random_curve(ctx);

	vkvg_stroke (ctx);
	vkvg_destroy(ctx);

}
void _long_path() {
    float w = (float)test_width-10;
    float h = (float)test_height-10;

    VkvgContext ctx = _initCtx();

    randomize_color(ctx);
    float x1 = w*rndf();
    float y1 = h*rndf();
    vkvg_move_to (ctx, x1, y1);
    for (uint32_t i=0; i<test_size; i++) {
        x1 = w*rndf();
        y1 = h*rndf();
        vkvg_line_to (ctx, x1, y1);
    }
    vkvg_stroke (ctx);
    vkvg_destroy(ctx);
}

void path () {
	_long_path ();
}
void curve () {
	_long_curve ();
}


int main(int argc, char *argv[]) {	
	//PERFORM_TEST(test, argc, argv);
	//vkvg_log_level = VKVG_LOG_ERR|VKVG_LOG_DEBUG|VKVG_LOG_INFO|VKVG_LOG_INFO_PATH|VKVG_LOG_DBG_ARRAYS|VKVG_LOG_FULL;
	dashes_count = 2;
	dashes[0] = 2;
	dashes[1] = 3;
	line_width = 2;
	line_cap = VKVG_LINE_CAP_ROUND;
	PERFORM_TEST(path, argc, argv);
	//PERFORM_TEST(curve, argc, argv);
	return 0;
}

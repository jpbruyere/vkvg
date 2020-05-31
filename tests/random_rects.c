#include "test.h"

/*void drawRandomRect (VkvgContext ctx) {
	float w = (float)test_width;
	float h = (float)test_height;
	randomize_color(ctx);

	float z = truncf((0.5f*w*rand()/RAND_MAX)+1.f);
	float v = truncf((0.5f*w*rand()/RAND_MAX)+1.f);
	float x = truncf((w-z)*rand()/RAND_MAX);
	float y = truncf((h-v)*rand()/RAND_MAX);

	vkvg_rectangle(ctx, x, y, z, v);
}*/

void _shape_fill(shape_t shape){
	vkvg_surface_clear(surf);
	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
	for (uint32_t i=0; i<test_size; i++) {
		draw_random_shape(ctx, shape);
		vkvg_fill(ctx);
	}
	vkvg_destroy(ctx);
}
void _shape_stroke(shape_t shape){
	vkvg_surface_clear (surf);
	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_line_width (ctx, 5.f);
	vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
	for (uint32_t i=0; i<test_size; i++) {
		draw_random_shape(ctx, shape);
		vkvg_stroke (ctx);
	}
	vkvg_destroy(ctx);
}
void _shape_fill_stroke(shape_t shape){
	vkvg_surface_clear(surf);
	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_line_width (ctx, 10.f);
	vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
	for (uint32_t i=0; i<test_size; i++) {
		draw_random_shape(ctx, shape);
		vkvg_fill_preserve(ctx);
		vkvg_stroke(ctx);
	}
	vkvg_destroy(ctx);
}

void rectangles_fill () {
	_shape_fill(SHAPE_RECTANGLE);
}
void rectangles_stroke () {
	_shape_stroke(SHAPE_RECTANGLE);
}
void rectangles_fill_stroke () {
	_shape_fill_stroke(SHAPE_RECTANGLE);
}
void rounded_rects_fill () {
	_shape_fill(SHAPE_ROUNDED_RECTANGLE);
}
void rounded_rects_stroke () {
	_shape_stroke(SHAPE_ROUNDED_RECTANGLE);
}
void rounded_rects_fill_stroke () {
	_shape_fill_stroke(SHAPE_ROUNDED_RECTANGLE);
}
void circles_fill () {
	_shape_fill(SHAPE_CIRCLE);
}
void circles_stroke () {
	_shape_stroke(SHAPE_CIRCLE);
}
void circles_fill_stroke () {
	_shape_fill_stroke(SHAPE_CIRCLE);
}
void stars_fill () {
	_shape_fill(SHAPE_STAR);
}
void stars_stroke () {
	_shape_stroke(SHAPE_STAR);
}
void stars_fill_stroke () {
	_shape_fill_stroke(SHAPE_STAR);
}
void random_fill () {
	_shape_fill(SHAPE_RANDOM);
}
void random_stroke () {
	_shape_stroke(SHAPE_RANDOM);
}
void random_fill_stroke () {
	_shape_fill_stroke(SHAPE_RANDOM);
}

int main(int argc, char *argv[]) {

	/*PERFORM_TEST (rectangles_fill, argc, argv);
	PERFORM_TEST (rectangles_stroke, argc, argv);
	PERFORM_TEST (rectangles_fill_stroke, argc, argv);

	PERFORM_TEST (rounded_rects_fill, argc, argv);
	PERFORM_TEST (rounded_rects_stroke, argc, argv);
*/	PERFORM_TEST (rounded_rects_fill_stroke, argc, argv);/*

	PERFORM_TEST (circles_fill, argc, argv);
	PERFORM_TEST (circles_stroke, argc, argv);
	PERFORM_TEST (circles_fill_stroke, argc, argv);*/

	/*PERFORM_TEST (stars_fill, argc, argv);
	PERFORM_TEST (stars_stroke, argc, argv);
	PERFORM_TEST (stars_fill_stroke, argc, argv);

	PERFORM_TEST (random_fill, argc, argv);
	PERFORM_TEST (random_stroke, argc, argv);
	PERFORM_TEST (random_fill_stroke, argc, argv);*/

	return 0;
}

#include "test.h"

VkvgPattern create_grad (VkvgContext ctx, float x) {
	VkvgPattern pat = vkvg_pattern_create_linear(x,0,300,0);
	vkvg_pattern_add_color_stop(pat, 0, 1, 0, 0, 1);
	vkvg_pattern_add_color_stop(pat, 0.5f, 0, 1, 0, 1);
	vkvg_pattern_add_color_stop(pat, 1, 0, 0, 1, 1);
	return pat;
}

void paint(){
	VkvgContext ctx = _initCtx (surf);
	//vkvg_translate(ctx,100,100);
	VkvgPattern pat = create_grad(ctx,0);
	//vkvg_pattern_set_extend(pat,VKVG_EXTEND_NONE);
	vkvg_rectangle(ctx, 0,0,400,460);
	vkvg_set_source (ctx, pat);
	vkvg_pattern_destroy (pat);
	vkvg_fill(ctx);

	float x = 100;
	pat = create_grad(ctx,x);
	//vkvg_pattern_set_extend(pat,VKVG_EXTEND_NONE);
	vkvg_rectangle(ctx, x,200,50,50);
	vkvg_set_source (ctx, pat);
	vkvg_pattern_destroy (pat);
	vkvg_fill(ctx);

	x+=100;

	pat = vkvg_pattern_create_linear(10,0,300,0);
	vkvg_pattern_add_color_stop(pat, 0, 0, 0, 1, 0);
	vkvg_pattern_add_color_stop(pat, 1, 1, 0, 0, 1);
	//vkvg_pattern_set_extend(pat,VKVG_EXTEND_NONE);
	vkvg_rectangle(ctx, 10,10,300,50);
	vkvg_set_source (ctx, pat);
	vkvg_pattern_destroy (pat);
	vkvg_fill(ctx);

	x+=200;

	pat = create_grad(ctx,x);
	//vkvg_pattern_set_extend(pat,VKVG_EXTEND_NONE);
	vkvg_rectangle(ctx, x,200,50,50);
	vkvg_set_source (ctx, pat);
	vkvg_pattern_destroy (pat);
	vkvg_fill(ctx);

	x+=100;

	pat = create_grad(ctx,x);
	//vkvg_pattern_set_extend(pat,VKVG_EXTEND_NONE);
	vkvg_rectangle(ctx, x,200,50,50);
	vkvg_set_source (ctx, pat);
	vkvg_pattern_destroy (pat);
	vkvg_fill(ctx);

	/*vkvg_set_source_rgb(ctx, 0,1,0);
	vkvg_rectangle(ctx, 100,100,200,160);
	vkvg_fill_preserve(ctx);
	vkvg_set_source_rgb(ctx, 0,0,0);
	vkvg_set_line_width(ctx,1.0f);
	vkvg_stroke(ctx);*/


	vkvg_destroy(ctx);
}
void paint_repeat(){
	VkvgContext ctx = _initCtx (surf);
	VkvgPattern pat = create_grad(ctx,0);
	vkvg_pattern_set_extend(pat,VKVG_EXTEND_REPEAT);
	vkvg_set_source (ctx, pat);
	vkvg_pattern_destroy (pat);
	vkvg_paint(ctx);

	vkvg_destroy(ctx);
}

void test(){
	VkvgContext ctx = _initCtx (surf);
	VkvgPattern pat = create_grad(ctx,0);
	vkvg_set_source (ctx, pat);
	vkvg_rectangle(ctx,100,100,200,200);
	vkvg_set_line_width(ctx, 20);
	//vkvg_fill (ctx);
	//vkvg_paint(ctx);
	vkvg_stroke (ctx);
	vkvg_pattern_destroy (pat);

	vkvg_destroy(ctx);
}
void test2(){
	VkvgContext ctx = _initCtx (surf);

	vkvg_set_source_rgb(ctx,1,0,0);
	vkvg_paint(ctx);

	VkvgPattern pat = vkvg_pattern_create_linear(100,0,300,0);
	vkvg_set_line_width(ctx, 20);
	vkvg_pattern_add_color_stop(pat, 0, 1, 1, 1, 1);
	vkvg_pattern_add_color_stop(pat, 1, 1, 1, 0, 0);
	vkvg_set_source (ctx, pat);
	vkvg_rectangle(ctx,100,100,200,200);
	vkvg_fill (ctx);
	//vkvg_stroke (ctx);
	vkvg_pattern_destroy (pat);

	vkvg_destroy(ctx);
}

void gradient_transform() {
	VkvgContext ctx = _initCtx (surf);

	//vkvg_translate(ctx,-100,-100);

	vkvg_translate(ctx, 200, 100);
	vkvg_rotate(ctx, 0.5f);

	//vkvg_scale(ctx,2,2);
	VkvgPattern pat = vkvg_pattern_create_linear(0, 0, 400, 0);
	vkvg_pattern_set_extend(pat, VKVG_EXTEND_NONE);
	vkvg_set_line_width(ctx, 20);
	vkvg_pattern_add_color_stop(pat, 0, 1, 0, 0, 1);
	vkvg_pattern_add_color_stop(pat, 0.5f, 0, 1, 0, 1);
	vkvg_pattern_add_color_stop(pat, 1, 0, 0, 1, 1);
	vkvg_set_source(ctx, pat);
	vkvg_rectangle(ctx, 0, 0, 400, 200);
	//vkvg_fill (ctx);
	vkvg_stroke(ctx);
	//vkvg_paint(ctx);
	vkvg_pattern_destroy(pat);

	vkvg_destroy(ctx);
}
void gradient_alpha() {
	VkvgContext ctx = _initCtx (surf);
	vkvg_set_source_rgb(ctx,0,1,0);
	vkvg_paint (ctx);

	VkvgPattern pat = vkvg_pattern_create_linear(10,0,300,0);
	vkvg_pattern_add_color_stop(pat, 1, 1, 0, 0, 0.5);
	vkvg_pattern_add_color_stop(pat, 1, 1, 0, 0, 0.5);

	vkvg_set_source (ctx, pat);
	vkvg_pattern_destroy (pat);
	vkvg_rectangle (ctx, 10, 10, 300, 200);
	vkvg_fill (ctx);

	vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
	no_test_size = true;
	PERFORM_TEST(gradient_alpha, argc, argv);
	PERFORM_TEST(paint, argc, argv);
	PERFORM_TEST(paint_repeat, argc, argv);
	PERFORM_TEST(gradient_transform, argc, argv);
	return 0;
}

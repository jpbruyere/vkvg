#include "test.h"

void paint(){
	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_source_rgba(ctx,1,0,0,1);
	vkvg_paint(ctx);
	vkvg_destroy(ctx);
}
void paint_with_rotation(){
	VkvgContext ctx = vkvg_create(surf);
	vkvg_rotate(ctx, 45);
	vkvg_set_source_rgba(ctx,1,0,0,1);
	vkvg_paint(ctx);
	vkvg_destroy(ctx);
}
void paint_with_scale(){
	VkvgContext ctx = vkvg_create(surf);
	vkvg_scale (ctx, 0.2f,0.2f);
	vkvg_set_source_rgba(ctx,1,0,0,1);
	vkvg_paint(ctx);
	vkvg_destroy(ctx);
}
void paint_rect(){
	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_source_rgba(ctx,1,0,0,1);
	vkvg_rectangle(ctx,100,100,300,200);
	vkvg_paint(ctx);
	vkvg_destroy(ctx);
}
//TODO:test failed: full screen paint instead of rotated rect
void paint_rect_with_rotation(){
	VkvgContext ctx = vkvg_create(surf);
	vkvg_rotate(ctx, 45);
	vkvg_set_source_rgba(ctx,1,0,0,1);
	vkvg_rectangle(ctx,100,100,300,200);
	vkvg_paint(ctx);
	vkvg_destroy(ctx);
}
void paint_rect_with_scale(){
	VkvgContext ctx = vkvg_create(surf);
	vkvg_scale (ctx, 0.2f,0.2f);
	vkvg_set_source_rgba(ctx,1,0,0,1);
	vkvg_rectangle(ctx,100,100,300,200);
	vkvg_paint(ctx);
	vkvg_destroy(ctx);
}
int main(int argc, char *argv[]) {
	PERFORM_TEST (paint, argc, argv);
	PERFORM_TEST (paint_with_rotation, argc, argv);
	PERFORM_TEST (paint_with_scale, argc, argv);
	PERFORM_TEST (paint_rect, argc, argv);
	PERFORM_TEST (paint_rect_with_rotation, argc, argv);
	PERFORM_TEST (paint_rect_with_scale, argc, argv);
	return 0;
}

#include "test.h"

vkvg_fill_rule_t fillrule = VKVG_FILL_RULE_NON_ZERO;
float lineWidth = 20.f;

VkvgSurface createSurf (uint32_t width, uint32_t height) {
	VkvgSurface s = vkvg_surface_create(device, width, height);
	VkvgContext ctx = vkvg_create(s);
	vkvg_set_fill_rule(ctx,fillrule);
	vkvg_set_line_width(ctx,lineWidth);
	float hlw = lineWidth/2.f;
	vkvg_rectangle(ctx,hlw,hlw,(float)width-hlw,(float)height-hlw);
	vkvg_set_source_rgba(ctx,0,1,0,0.5);
	vkvg_fill_preserve(ctx);
	vkvg_set_source_rgba(ctx,0,0,1,0.5);
	vkvg_stroke(ctx);
	vkvg_destroy(ctx);
	return s;
}

void paint(){
	VkvgSurface src = createSurf(256,256);
	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_fill_rule(ctx,fillrule);
	vkvg_set_source_surface(ctx, src, 0, 0);
	vkvg_paint(ctx);
	vkvg_destroy(ctx);
	vkvg_surface_destroy(src);
}
void paint_with_offset(){
	VkvgSurface src = createSurf(256,256);
	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_fill_rule(ctx,fillrule);
	vkvg_set_source_surface(ctx, src, 100, 100);
	vkvg_paint(ctx);
	vkvg_destroy(ctx);
	vkvg_surface_destroy(src);
}
void paint_multiple(){
	VkvgSurface src = createSurf(256,256);
	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_fill_rule(ctx,fillrule);
	for(int i=0;i<10;i++) {
		vkvg_set_source_surface(ctx, src, i * 20, i * 20);
		vkvg_paint(ctx);
	}
	vkvg_destroy(ctx);
	vkvg_surface_destroy(src);
}
void paint_with_rotation(){
	VkvgSurface src = createSurf(256,256);
	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_fill_rule(ctx,fillrule);
	vkvg_set_source_surface(ctx, src, 0, 0);
	vkvg_rotate(ctx, 45);
	vkvg_paint(ctx);
	vkvg_destroy(ctx);
	vkvg_surface_destroy(src);
}
void paint_with_scale(){
	VkvgSurface src = createSurf(256,256);
	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_fill_rule(ctx,fillrule);
	vkvg_set_source_surface(ctx, src, 0, 0);
	vkvg_scale (ctx, 0.2f,0.2f);
	vkvg_paint(ctx);
	vkvg_destroy(ctx);
	vkvg_surface_destroy(src);
}
void paint_rect(){
	VkvgSurface src = createSurf(256,256);
	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_fill_rule(ctx,fillrule);
	vkvg_set_source_surface(ctx, src, 0, 0);
	vkvg_rectangle(ctx,100,100,300,200);
	vkvg_paint(ctx);
	vkvg_destroy(ctx);
	vkvg_surface_destroy(src);
}
//TODO:test failed: full screen paint instead of rotated rect
void paint_rect_with_rotation(){
	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_fill_rule(ctx,fillrule);
	vkvg_rotate(ctx, 45);
	vkvg_set_source_rgba(ctx,1,0,0,1);
	vkvg_rectangle(ctx,100,100,300,200);
	vkvg_paint(ctx);
	vkvg_destroy(ctx);
}
void paint_rect_with_scale(){
	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_fill_rule(ctx,fillrule);
	vkvg_scale (ctx, 0.2f,0.2f);
	vkvg_set_source_rgba(ctx,1,0,0,1);
	vkvg_rectangle(ctx,100,100,300,200);
	vkvg_paint(ctx);
	vkvg_destroy(ctx);
}
int main(int argc, char *argv[]) {
	PERFORM_TEST (paint, argc, argv);
	PERFORM_TEST (paint_with_offset, argc, argv);
	PERFORM_TEST (paint_multiple, argc, argv);
	PERFORM_TEST (paint_with_rotation, argc, argv);
	PERFORM_TEST (paint_with_scale, argc, argv);
	PERFORM_TEST (paint_rect, argc, argv);
	/*PERFORM_TEST (paint_rect_with_rotation, argc, argv);
	PERFORM_TEST (paint_rect_with_scale, argc, argv);*/
	return 0;
}

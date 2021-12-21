/*
 * Copyright (c) 2018 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "test.h"

void cairo_test_fill_rule (VkvgContext cr){
	vkvg_set_line_width (cr, 6);

	//vkvg_scale(cr,3,3);
	vkvg_set_source_rgba(cr,1,0,0,1);
	vkvg_move_to(cr,50,150);
	vkvg_rel_line_to(cr,50,70);
	vkvg_rel_line_to(cr,50,0);
	vkvg_rel_line_to(cr,50,-70);
	vkvg_rel_line_to(cr,0,-60);
	vkvg_rel_line_to(cr,-50,-30);
	vkvg_rel_line_to(cr,-50,0);
	vkvg_rel_line_to(cr,-50,30);
	vkvg_close_path(cr);


//    vkvg_set_line_join(cr,VKVG_LINE_JOIN_ROUND);
	vkvg_set_source_rgb (cr, 0, 0.7f, 0);
	vkvg_rectangle (cr, 12, 12, 232, 70);
	//vkvg_stroke (cr);
//    vkvg_new_sub_path (cr);
	vkvg_arc (cr, 64, 64, 40, 0, M_PIF*2.f);
	//vkvg_close_path(cr);

	vkvg_new_sub_path (cr);
	vkvg_arc_negative (cr, 192, 64, 40, M_PIF*2.f, 0);
	//vkvg_close_path(cr);

	//vkvg_rectangle (cr, 30, 30, 20, 200);
	//vkvg_rectangle (cr, 130, 30, 20, 200);
	//vkvg_set_fill_rule (cr, vkvg_FILL_RULE_EVEN_ODD);

	vkvg_fill_preserve(cr);

	vkvg_set_source_rgb (cr, 0, 0, 0);
	vkvg_stroke (cr);
}
void cairo_test_text (VkvgContext cr) {
	vkvg_text_extents_t extents;
	vkvg_font_extents_t ft;

	//vkvg_set_fill_rule(cr, VKVG_FILL_RULE_NON_ZERO);
	const char *utf8 = "vkvg|Ãp";
	float x,y;

	//vkvg_select_font_face (cr, "times");
	vkvg_select_font_face (cr, "linux biolinum keyboard");
	vkvg_set_font_size (cr, 50);
	vkvg_font_extents(cr, &ft);
	vkvg_text_extents (cr, utf8, &extents);
	vkvg_set_source_rgb(cr,0,0,0);

	x=25.0f;
	y=150.0f;

	vkvg_move_to (cr, x,y);
	vkvg_show_text (cr, utf8);

	/* draw helping lines */
	vkvg_set_source_rgba (cr, 0, 0.2f, 0.2f, 0.6f);
	vkvg_set_line_width (cr, 1.0f);
	vkvg_new_path(cr);
	vkvg_arc (cr, x, y, 10.0f, 0, 2.f*M_PIF);
	vkvg_fill (cr);
	vkvg_move_to (cr, x,y);
	//vkvg_rel_line_to (cr, 0, -30);
	vkvg_rel_line_to (cr, 0, -ft.ascent);
	vkvg_rel_line_to (cr, extents.width, 0);
	vkvg_rel_line_to (cr, extents.x_bearing, -extents.y_bearing);

	vkvg_stroke (cr);

	vkvg_move_to (cr, x,y);
	vkvg_rel_line_to (cr, extents.width, 0);
	vkvg_set_source_rgba (cr, 0.0, 0.0, 0.9f, 0.6f);

	vkvg_stroke (cr);

	vkvg_move_to (cr, x,y);
	vkvg_rel_line_to (cr, 0, ft.descent);
	vkvg_rel_line_to (cr, extents.width, 0);
	vkvg_set_source_rgba (cr, 0.9f, 0.0, 0.0, 0.6f);

	vkvg_stroke (cr);

	vkvg_move_to (cr, x-10,y-ft.ascent);
	vkvg_rel_line_to (cr, 0, ft.height);
	vkvg_set_source_rgba (cr, 0.0, 0.1f, 0.0, 0.6f);

	vkvg_stroke (cr);
}
void cairo_test_clip (VkvgContext cr){
	vkvg_arc (cr, 128.0f, 128.0f, 76.8f, 0, 2.f * M_PIF);
	vkvg_clip (cr);
	//vkvg_new_path (cr);  /* current path is not
	//                         consumed by vkvg_clip() */
	vkvg_set_source_rgba(cr, 0, 0, 0, 1);
	vkvg_rectangle (cr, 0, 0, 256, 256);
	vkvg_fill (cr);
	vkvg_set_source_rgba (cr, 0, 1, 0, 1);
	vkvg_move_to (cr, -100, -100);
	vkvg_line_to (cr, 256, 256);
	vkvg_move_to (cr, 356, -100);
	vkvg_line_to (cr, 0, 256);
	vkvg_set_line_width (cr, 10.0f);
	vkvg_stroke (cr);
}
void cairo_test_curves (VkvgContext cr){
	float x=25.6f,  y=128.0f;
	float x1=102.4f, y1=230.4f,
		   x2=153.6f, y2=25.6f,
		   x3=230.4f, y3=128.0f;

	vkvg_set_source_rgb (cr, 0, 0, 0);
	vkvg_move_to (cr, x, y);
	vkvg_curve_to (cr, x1, y1, x2, y2, x3, y3);

	vkvg_set_line_width (cr, 10.0f);
	vkvg_stroke (cr);

	vkvg_set_source_rgba (cr, 1, 0.2f, 0.2f, 0.6f);
	vkvg_set_line_width (cr, 6.0f);
	vkvg_move_to (cr,x,y);   vkvg_line_to (cr,x1,y1);
	vkvg_move_to (cr,x2,y2); vkvg_line_to (cr,x3,y3);
	vkvg_stroke (cr);
}
void cairo_test_rounded_rect (VkvgContext cr) {
	/* a custom shape that could be wrapped in a function */
	float x0      = 25.6f,   /* parameters like vkvg_rectangle */
		   y0      = 25.6f,
		   rect_width  = 204.8f,
		   rect_height = 204.8f,
		   radius = 102.4f;   /* and an approximate curvature radius */

	float x1,y1;

	x1=x0+rect_width;
	y1=y0+rect_height;
	if (!rect_width || !rect_height)
		return;
	if (rect_width/2<radius) {
		if (rect_height/2<radius) {
			vkvg_move_to  (cr, x0, (y0 + y1)/2);
			vkvg_curve_to (cr, x0 ,y0, x0, y0, (x0 + x1)/2, y0);
			vkvg_curve_to (cr, x1, y0, x1, y0, x1, (y0 + y1)/2);
			vkvg_curve_to (cr, x1, y1, x1, y1, (x1 + x0)/2, y1);
			vkvg_curve_to (cr, x0, y1, x0, y1, x0, (y0 + y1)/2);
		} else {
			vkvg_move_to  (cr, x0, y0 + radius);
			vkvg_curve_to (cr, x0 ,y0, x0, y0, (x0 + x1)/2, y0);
			vkvg_curve_to (cr, x1, y0, x1, y0, x1, y0 + radius);
			vkvg_line_to (cr, x1 , y1 - radius);
			vkvg_curve_to (cr, x1, y1, x1, y1, (x1 + x0)/2, y1);
			vkvg_curve_to (cr, x0, y1, x0, y1, x0, y1- radius);
		}
	} else {
		if (rect_height/2<radius) {
			vkvg_move_to  (cr, x0, (y0 + y1)/2);
			vkvg_curve_to (cr, x0 , y0, x0 , y0, x0 + radius, y0);
			vkvg_line_to (cr, x1 - radius, y0);
			vkvg_curve_to (cr, x1, y0, x1, y0, x1, (y0 + y1)/2);
			vkvg_curve_to (cr, x1, y1, x1, y1, x1 - radius, y1);
			vkvg_line_to (cr, x0 + radius, y1);
			vkvg_curve_to (cr, x0, y1, x0, y1, x0, (y0 + y1)/2);
		} else {
			vkvg_move_to  (cr, x0, y0 + radius);
			vkvg_curve_to (cr, x0 , y0, x0 , y0, x0 + radius, y0);
			vkvg_line_to (cr, x1 - radius, y0);
			vkvg_curve_to (cr, x1, y0, x1, y0, x1, y0 + radius);
			vkvg_line_to (cr, x1 , y1 - radius);
			vkvg_curve_to (cr, x1, y1, x1, y1, x1 - radius, y1);
			vkvg_line_to (cr, x0 + radius, y1);
			vkvg_curve_to (cr, x0, y1, x0, y1, x0, y1- radius);
		}
	}
	vkvg_close_path (cr);

	vkvg_set_source_rgb (cr, 0.5f, 0.5f, 1);
	vkvg_fill_preserve (cr);
	vkvg_set_source_rgba (cr, 0.5f, 0, 0, 0.5f);
	vkvg_set_line_width (cr, 10.0f);
	vkvg_stroke (cr);
}
void cairo_test_fill_and_stroke2 (VkvgContext cr){
	vkvg_move_to (cr, 128.0f, 25.6f);
	vkvg_line_to (cr, 230.4f, 230.4f);
	vkvg_rel_line_to (cr, -102.4f, 0.0);
	vkvg_curve_to (cr, 51.2f, 230.4f, 51.2f, 128.0f, 128.0f, 128.0f);
	vkvg_close_path (cr);

	vkvg_move_to (cr, 64.0f, 25.6f);
	vkvg_rel_line_to (cr, 51.2f, 51.2f);
	vkvg_rel_line_to (cr, -51.2f, 51.2f);
	vkvg_rel_line_to (cr, -51.2f, -51.2f);
	vkvg_close_path (cr);

	/*vkvg_translate(cr,100,100);
	vkvg_move_to (cr, 100, 100);
	vkvg_line_to(cr,300,300);
	vkvg_line_to(cr,100,300);*/


	vkvg_set_line_width (cr, 10.0f);
	vkvg_set_source_rgb (cr, 0, 0, 1);
	vkvg_fill_preserve (cr);
	vkvg_set_source_rgb (cr, 0, 0, 0);
	vkvg_stroke (cr);
}
void cairo_print_arc_neg (VkvgContext cr){
	float xc = 128.0f;
	float yc = 128.0f;
	float radius = 100.0f;
	float angle1 = 45.0f  * (M_PIF/180.0f);  /* angles are specified */
	float angle2 = 180.0f * (M_PIF/180.0f);  /* in radians           */

	vkvg_set_source_rgba(cr, 0, 0, 0, 1);
	vkvg_set_line_width (cr, 10.0f);
	vkvg_arc_negative (cr, xc, yc, radius, angle1, angle2);
	vkvg_stroke (cr);

	/* draw helping lines */
	vkvg_set_source_rgba (cr, 1, 0.2f, 0.2f, 0.6f);
	vkvg_set_line_width (cr, 6.0f);

	vkvg_arc (cr, xc, yc, 10.0f, 0, 2.f*M_PIF);
	vkvg_fill (cr);

	vkvg_arc (cr, xc, yc, radius, angle1, angle1);
	vkvg_line_to (cr, xc, yc);
	vkvg_arc (cr, xc, yc, radius, angle2, angle2);
	//vkvg_line_to (cr, xc, yc);
	vkvg_stroke (cr);

}
void cairo_test_line_caps (VkvgContext cr) {
	vkvg_set_source_rgb (cr, 0, 0, 0);
	vkvg_set_line_width (cr, 30.0f);
	vkvg_set_line_cap  (cr, VKVG_LINE_CAP_BUTT); /* default */
	vkvg_move_to (cr, 64.0f, 50.0f); vkvg_line_to (cr, 64.0f, 200.0f);
	vkvg_stroke (cr);
	vkvg_set_line_cap  (cr, VKVG_LINE_CAP_ROUND);
	vkvg_move_to (cr, 128.0f, 50.0f); vkvg_line_to (cr, 128.0f, 200.0f);
	vkvg_stroke (cr);
	vkvg_set_line_cap  (cr, VKVG_LINE_CAP_SQUARE);
	vkvg_move_to (cr, 192.0f, 50.0f); vkvg_line_to (cr, 192.0f, 200.0f);
	vkvg_stroke (cr);

	/* draw helping lines */
	vkvg_set_source_rgb (cr, 1, 0.2f, 0.2f);
	vkvg_set_line_width (cr, 2.56f);
	vkvg_move_to (cr, 64.0f, 50.0f); vkvg_line_to (cr, 64.0f, 200.0f);
	vkvg_move_to (cr, 128.0f, 50.0f);  vkvg_line_to (cr, 128.0f, 200.0f);
	vkvg_move_to (cr, 192.0f, 50.0f); vkvg_line_to (cr, 192.0f, 200.0f);
	vkvg_stroke (cr);
}
void cairo_test_line_joins (VkvgContext cr) {
	vkvg_set_source_rgb (cr, 0, 0, 0);
	vkvg_set_line_width (cr, 40.96f);
	vkvg_move_to (cr, 76.8f, 84.48f);
	vkvg_rel_line_to (cr, 51.2f, -51.2f);
	vkvg_rel_line_to (cr, 51.2f, 51.2f);
	vkvg_set_line_join (cr, VKVG_LINE_JOIN_MITER); /* default */
	vkvg_stroke (cr);

	vkvg_move_to (cr, 76.8f, 161.28f);
	vkvg_rel_line_to (cr, 51.2f, -51.2f);
	vkvg_rel_line_to (cr, 51.2f, 51.2f);
	vkvg_set_line_join (cr, VKVG_LINE_JOIN_BEVEL);
	vkvg_stroke (cr);

	vkvg_move_to (cr, 76.8f, 238.08f);
	vkvg_rel_line_to (cr, 51.2f, -51.2f);
	vkvg_rel_line_to (cr, 51.2f, 51.2f);
	vkvg_set_line_join (cr, VKVG_LINE_JOIN_ROUND);
	vkvg_stroke (cr);

		/* draw helping lines */
	vkvg_set_source_rgb (cr, 1, 0.2f, 0.2f);
	vkvg_set_line_width (cr, 2.56f);
	vkvg_set_line_join (cr, VKVG_LINE_JOIN_MITER);
	vkvg_move_to (cr, 76.8f, 84.48f);
	vkvg_rel_line_to (cr, 51.2f, -51.2f);
	vkvg_rel_line_to (cr, 51.2f, 51.2f);
	vkvg_move_to (cr, 76.8f, 161.28f);
	vkvg_rel_line_to (cr, 51.2f, -51.2f);
	vkvg_rel_line_to (cr, 51.2f, 51.2f);
	vkvg_move_to (cr, 76.8f, 238.08f);
	vkvg_rel_line_to (cr, 51.2f, -51.2f);
	vkvg_rel_line_to (cr, 51.2f, 51.2f);

	vkvg_stroke (cr);

}
void cairo_print_arc (VkvgContext cr) {
	float xc = 128.0f;
	float yc = 128.0f;
	float radius = 100.0f;
	float angle1 = 45.0f  * (M_PIF/180.0f);  /* angles are specified */
	float angle2 = 180.0f * (M_PIF/180.0f);  /* in radians           */

	vkvg_set_source_rgba(cr, 0, 0, 0, 1);
	vkvg_set_line_width (cr, 10.0f);
	vkvg_arc (cr, xc, yc, radius, angle1, angle2);
	vkvg_stroke (cr);

	/* draw helping lines */
	vkvg_set_source_rgba(cr, 1, 0.2f, 0.2f, 0.6f);
	vkvg_set_line_width (cr, 6.0f);

	vkvg_arc (cr, xc, yc, 10.0f, 0, 2.f*M_PIF);
	vkvg_fill (cr);

	vkvg_arc (cr, xc, yc, radius, angle1, angle1);
	vkvg_line_to (cr, xc, yc);
	vkvg_arc (cr, xc, yc, radius, angle2, angle2);
	vkvg_stroke (cr);
}
static float rotation = 0.f;

void cairo_tests () {
	rotation+=0.002f;

	vkvg_matrix_t mat;
	vkvg_matrix_init_translate (&mat, 512,400);
	vkvg_matrix_rotate(&mat,rotation);
	vkvg_matrix_scale(&mat,zoom,zoom);
	vkvg_matrix_translate(&mat,-512.f + panX,-400.f +panY);


	VkvgContext ctx = vkvg_create(surf);
	vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
	vkvg_set_source_rgba(ctx,1.0f,1.0f,1.0f,1);
	vkvg_paint(ctx);

	//vkvg_set_matrix(ctx,&mat);

	cairo_print_arc(ctx);

	vkvg_translate(ctx,200,0);
	cairo_test_fill_rule(ctx);

	vkvg_translate(ctx,250,0);
	cairo_test_rounded_rect(ctx);

	vkvg_translate(ctx,-450,250);
	cairo_test_fill_and_stroke2(ctx);

	vkvg_translate(ctx,250,0);
	cairo_print_arc_neg(ctx);

	vkvg_translate(ctx,250,0);
	cairo_test_text(ctx);

	vkvg_translate(ctx,-500,250);
	cairo_test_curves(ctx);

	vkvg_translate(ctx,250,0);
	cairo_test_line_joins(ctx);

	vkvg_translate(ctx,250,0);
	cairo_test_line_caps(ctx);

	vkvg_destroy(ctx);
}


int main(int argc, char *argv[]) {
	no_test_size = true;
	PERFORM_TEST (cairo_tests, argc, argv);
	return 0;
}

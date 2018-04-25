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

#include "vkengine.h"
#include "vkvg.h"

VkvgDevice device;
VkvgSurface surf = NULL;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;
    switch (key) {
    case GLFW_KEY_ESCAPE :
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    }
}
static void char_callback (GLFWwindow* window, uint32_t c){}
static void mouse_move_callback(GLFWwindow* window, double x, double y){}
static void mouse_button_callback(GLFWwindow* window, int but, int state, int modif){}

void vkvg_test_gradient (VkvgContext ctx) {
    VkvgPattern pat = vkvg_pattern_create_linear(100,0,300,0);
    vkvg_set_line_width(ctx, 20);
    vkvg_patter_add_color_stop(pat, 0, 1, 0, 0, 1);
    vkvg_patter_add_color_stop(pat, 0.5, 0, 1, 0, 1);
    vkvg_patter_add_color_stop(pat, 1, 0, 0, 1, 1);
    vkvg_set_source (ctx, pat);
    vkvg_rectangle(ctx,100,100,200,200);
    vkvg_fill (ctx);
    //vkvg_stroke (ctx);
    vkvg_pattern_destroy (pat);
}

void vkvg_test_clip(VkvgContext ctx){
    vkvg_move_to(ctx,10,10);
    vkvg_line_to(ctx,400,150);
    vkvg_line_to(ctx,900,10);
    vkvg_line_to(ctx,700,450);
    vkvg_line_to(ctx,900,750);
    vkvg_line_to(ctx,500,650);
    vkvg_line_to(ctx,100,800);
    vkvg_line_to(ctx,150,400);
    vkvg_clip(ctx);
}
void vkvg_test_fill(VkvgContext ctx){
    vkvg_set_source_rgba(ctx,0.1,0.9,0.1,1.0);
    vkvg_move_to(ctx,10,10);
    vkvg_rel_line_to(ctx,100,100);
    vkvg_rel_line_to(ctx,100,-100);
    vkvg_rel_line_to(ctx,100,400);
//    vkvg_line_to(ctx,400,350);
//    vkvg_line_to(ctx,900,150);
//    vkvg_line_to(ctx,700,450);
//    vkvg_line_to(ctx,900,750);
//    vkvg_line_to(ctx,500,650);
//    vkvg_line_to(ctx,100,800);
//    vkvg_line_to(ctx,150,400);
    vkvg_close_path(ctx);
    vkvg_fill(ctx);
}
void vkvg_test_fill_and_stroke (VkvgContext ctx){
    vkvg_move_to (ctx, 100, 100);
    vkvg_rel_line_to (ctx, 50, -80);
    vkvg_rel_line_to (ctx, 50, 80);
    //vkvg_close_path (ctx);

    vkvg_move_to (ctx, 300, 100);
    vkvg_rel_line_to (ctx, 50, -80);
    vkvg_rel_line_to (ctx, 50, 80);
    vkvg_close_path (ctx);

    vkvg_set_line_width (ctx, 10.0);
    vkvg_set_source_rgb (ctx, 0, 0, 1);
    vkvg_fill_preserve (ctx);
    //vkvg_fill(ctx);
    vkvg_set_source_rgb (ctx, 0, 0, 0);
    vkvg_stroke (ctx);
}
void vkvg_test_curves2 (VkvgContext ctx) {
    vkvg_set_source_rgba   (ctx, 0.5,0.0,1.0,0.5);
    vkvg_set_line_width(ctx, 10);


    vkvg_move_to    (ctx, 100, 400);
    vkvg_curve_to   (ctx, 100, 100, 600,700,600,400);

    vkvg_move_to    (ctx, 100, 100);
    vkvg_curve_to   (ctx, 1000, 100, 100, 800, 1000, 800);
    vkvg_move_to    (ctx, 100, 150);
    vkvg_curve_to   (ctx, 1000, 500, 700, 500, 700, 100);

    vkvg_stroke     (ctx);
}
void vkvg_test_curves (VkvgContext ctx){

    vkvg_set_line_width(ctx, 10);
    vkvg_set_source_rgb   (ctx, 0,0,0);

    vkvg_arc(ctx, 150, 100, 10, 0, M_PI*2);
    vkvg_fill(ctx);
    vkvg_arc(ctx, 200, 200, 10, 0, M_PI*2);
    vkvg_fill(ctx);

    vkvg_set_source_rgba   (ctx, 0.5,0.0,1.0,0.5);
    vkvg_move_to(ctx,100,100);
    vkvg_line_to(ctx,200,100);
    vkvg_curve_to(ctx,250,100,300,150,300,200);
    vkvg_line_to(ctx,300,300);
    vkvg_curve_to(ctx,300,350,250,400,200,400);
    vkvg_line_to(ctx,100,400);
    vkvg_curve_to(ctx,50,400,10,350,10,300);
    vkvg_line_to(ctx,10,200);
    vkvg_curve_to(ctx,10,150,50,100,100,100);
    vkvg_close_path(ctx);
    //vkvg_curve_to(ctx, 150,100,200,150,200,200);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgba   (ctx, 1,1,1.0,0.5);
    vkvg_stroke(ctx);
}
void vkvg_test_stroke(VkvgContext ctx){
    vkvg_set_line_width(ctx, 2);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_move_to(ctx,200.5,200.5);
    vkvg_line_to(ctx,400.5,200.5);
    vkvg_line_to(ctx,400.5,400.5);
    vkvg_line_to(ctx,200.5,400.5);
    vkvg_close_path(ctx);
    vkvg_save (ctx);
    vkvg_stroke_preserve(ctx);
    vkvg_set_source_rgba(ctx,0,0.2,0.35,1);
    vkvg_fill(ctx);
    vkvg_set_source_rgba(ctx,0.5,1,0,1);
    vkvg_move_to(ctx,300.5,300.5);
    vkvg_line_to(ctx,500.5,300.5);
    vkvg_line_to(ctx,500.5,500.5);
    vkvg_line_to(ctx,300.5,500.5);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);
    vkvg_set_line_width(ctx, 40);
    vkvg_restore(ctx);
    vkvg_set_source_rgba(ctx,0.5,0.6,1,1.0);
    vkvg_move_to(ctx,700,475);
    vkvg_line_to(ctx,400,475);
    vkvg_stroke(ctx);
    vkvg_set_source_rgba(ctx,0,0.5,0.5,0.5);
    vkvg_move_to(ctx,300,200);
    vkvg_arc(ctx, 200,200,100,0, M_PI);
    vkvg_stroke(ctx);

    vkvg_set_line_width(ctx, 20);
    vkvg_set_source_rgba(ctx,0.1,0.1,0.1,0.5);
    vkvg_move_to(ctx,100,60);
    vkvg_line_to(ctx,400,600);
    vkvg_stroke(ctx);
}
void test_text (VkvgContext ctx) {
    int size = 19;
    int penY = 50;
    int penX = 10;

    /*vkvg_rectangle(ctx,30,0,100,400);
    vkvg_clip(ctx);*/

    //vkvg_select_font_face(ctx, "/usr/local/share/fonts/DroidSansMono.ttf");
    //vkvg_select_font_face(ctx, "/usr/share/fonts/truetype/unifont/unifont.ttf");

    vkvg_set_font_size(ctx,size-10);
    vkvg_select_font_face(ctx, "droid");
    vkvg_move_to(ctx, penX,penY);
    vkvg_set_source_rgba(ctx,0.7,0.7,0.7,1);
    vkvg_show_text (ctx,"abcdefghijk");
    penY+=size;


    vkvg_select_font_face(ctx, "times");
    vkvg_set_source_rgba(ctx,0.9,0.7,0.7,1);
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"abcdefghijklmnopqrstuvwxyz");
    penY+=size;



    vkvg_select_font_face(ctx, "droid");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"lmnopqrstuvwxyz123456789");
    penY+=size;



    vkvg_select_font_face(ctx, "times:bold");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"abcdefghijklmnopqrstuvwxyz");
    penY+=size;


    vkvg_select_font_face(ctx, "droid");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    penY+=size;



    vkvg_select_font_face(ctx, "arial:italic");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"abcdefghijklmnopqrstuvwxyz");
    penY+=size;


    vkvg_select_font_face(ctx, "arial");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    penY+=size;
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"this is a test");
    penY+=size;
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"this is another test to see if label is working");
    penY+=size;

    vkvg_select_font_face(ctx, "mono");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    penY+=size;

    vkvg_move_to(ctx, 80,400);
    vkvg_show_text (ctx,"Ленивый рыжий кот");


    /*vkvg_move_to(ctx, 150,250);
    vkvg_show_text (ctx,"test string é€");
    vkvg_move_to(ctx, 150,300);
    vkvg_show_text (ctx,"كسول الزنجبيل القط");
    vkvg_move_to(ctx, 150,350);
    vkvg_show_text (ctx,"懶惰的姜貓");*/

    //vkvg_show_text (ctx,"ABCDABCD");
    //vkvg_show_text (ctx,"j");
}
void vkvg_test_stroke2(VkvgContext ctx){
    vkvg_set_line_width(ctx,20);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_move_to(ctx,200,200);
    vkvg_line_to(ctx,400,200);
    vkvg_line_to(ctx,400,400);
    vkvg_line_to(ctx,200,400);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);
    vkvg_set_source_rgba(ctx,0.5,1,0,1);
    vkvg_move_to(ctx,300,300);
    vkvg_line_to(ctx,500,300);
    vkvg_line_to(ctx,500,500);
    vkvg_line_to(ctx,300,500);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);
    vkvg_set_line_width(ctx,10);
    vkvg_set_source_rgba(ctx,0.5,0.6,1,1);
    vkvg_move_to(ctx,700,475);
    vkvg_line_to(ctx,400,475);
    vkvg_stroke(ctx);
    vkvg_set_source_rgba(ctx,1,0,1,1);
    vkvg_move_to(ctx,700,500);

    vkvg_arc(ctx, 600,500,100,M_PI, 2.0*M_PI);
    vkvg_stroke(ctx);


    vkvg_set_line_width(ctx,20);
    vkvg_set_source_rgba(ctx,1,1,0,1);
    vkvg_move_to(ctx,100,50);
    vkvg_line_to(ctx,400,50);
    vkvg_stroke(ctx);
}
void vkvg_test_fill2(VkvgContext ctx){
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_move_to(ctx,200,200);
    vkvg_line_to(ctx,250,150);
    vkvg_line_to(ctx,200,100);
    vkvg_line_to(ctx,300,150);
    vkvg_line_to(ctx,700,100);
    vkvg_line_to(ctx,400,200);
    vkvg_line_to(ctx,400,400);
    vkvg_line_to(ctx,200,400);
    vkvg_line_to(ctx,300,300);
    vkvg_close_path(ctx);
    vkvg_fill(ctx);
}
void test_img_surface (VkvgContext ctx) {
    VkvgSurface imgSurf;// = vkvg_surface_create_from_image(device, "/mnt/data/images/blason.png");
    //VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "/mnt/data/images/2000px-Tux.svg.png");
    //VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "/mnt/data/images/path2674.png");
    //VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "/mnt/data/images/horse-black-head-shape-of-a-chess-piece_318-52446.jpg");
    /*vkvg_set_source_surface(ctx, imgSurf, 200, 200);
    vkvg_paint(ctx);
    vkvg_set_source_surface(ctx, imgSurf, 400, 400);
    vkvg_paint(ctx);
    vkvg_flush(ctx);
    vkvg_surface_destroy(imgSurf);*/

    imgSurf = vkvg_surface_create_from_image(device, "/mnt/data/images/miroir.jpg");
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);
    vkvg_flush(ctx);
    vkvg_set_source_rgba(ctx,0,0,0,1);
    vkvg_surface_destroy(imgSurf);
}
void test_line_caps (VkvgContext ctx) {

    float x = 20, y = 20, dx = 30, dy = 60;

    //vkvg_scale(ctx,5,5);
    vkvg_set_line_width(ctx,26);
    vkvg_set_source_rgba(ctx,0,0,0,1);
    vkvg_move_to(ctx,x,y);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_stroke(ctx);
    vkvg_set_line_cap(ctx,VKVG_LINE_CAP_SQUARE);
    vkvg_rel_move_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_stroke(ctx);
    vkvg_set_line_cap(ctx,VKVG_LINE_CAP_ROUND);
    vkvg_rel_move_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_rel_move_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,dx,dy);
    vkvg_rel_move_to(ctx,dx,-dy/2);
    vkvg_rel_line_to(ctx,dx,0);
    vkvg_rel_move_to(ctx,dx,dy/2);
    vkvg_rel_line_to(ctx,dx,-dy);
    vkvg_rel_move_to(ctx,dx,dy);
    vkvg_rel_line_to(ctx,0,-dy);
    vkvg_rel_move_to(ctx,2*dx,dy);
    vkvg_rel_line_to(ctx,-dx,-dy);
    vkvg_rel_move_to(ctx,3*dx,dy/2);
    vkvg_rel_line_to(ctx,-dx,0);
    //vkvg_rel_line_to(ctx,0,-dy);
    //vkvg_rel_move_to(ctx,dx,dy/2);
    //vkvg_rel_line_to(ctx,dx,0);
    vkvg_stroke(ctx);

    vkvg_set_line_cap(ctx,VKVG_LINE_CAP_BUTT);
    vkvg_set_line_width(ctx,1);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_move_to(ctx,x,y);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_rel_move_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_rel_move_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_stroke(ctx);
}
void test_line_join (VkvgContext ctx){
    float x = 50, y = 150, dx = 150, dy = 140;

    //vkvg_scale(ctx,2,2);

    vkvg_set_line_width(ctx,10);
    vkvg_set_source_rgba(ctx,0,0,0,1);


    //vkvg_set_line_join(ctx,VKVG_LINE_JOIN_ROUND);
    //vkvg_rectangle(ctx,x,y,dx,dy);

    vkvg_move_to(ctx,x,y);
    vkvg_rel_line_to(ctx,50,-30);
    vkvg_rel_line_to(ctx,50,0);
    vkvg_rel_line_to(ctx,50,30);
    vkvg_rel_line_to(ctx,0,60);
    vkvg_rel_line_to(ctx,-50,70);
    vkvg_rel_line_to(ctx,-50,0);
    vkvg_rel_line_to(ctx,-50,-70);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);

    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_move_to(ctx,x+200,y);
    vkvg_rel_line_to(ctx,50,70);
    vkvg_rel_line_to(ctx,50,0);
    vkvg_rel_line_to(ctx,50,-70);
    vkvg_rel_line_to(ctx,0,-60);
    vkvg_rel_line_to(ctx,-50,-30);
    vkvg_rel_line_to(ctx,-50,0);
    vkvg_rel_line_to(ctx,-50,30);
    vkvg_close_path(ctx);
    vkvg_fill(ctx);

    vkvg_move_to(ctx,x,y);
    vkvg_rel_line_to(ctx,50,-30);
    vkvg_rel_line_to(ctx,50,0);
    vkvg_rel_line_to(ctx,50,30);
    vkvg_rel_line_to(ctx,0,60);
    vkvg_rel_line_to(ctx,-50,70);
    vkvg_rel_line_to(ctx,-50,0);
    vkvg_rel_line_to(ctx,-50,-70);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);

//    vkvg_rel_line_to(ctx,dx,-dy);
//    vkvg_rel_line_to(ctx,dx,dy);
//    vkvg_stroke(ctx);
//    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_BEVEL);
//    vkvg_rel_move_to(ctx,-dx*2,abs(dy*1.5));
//    vkvg_rel_line_to(ctx,dx,-dy);
//    vkvg_rel_line_to(ctx,dx,dy);
//    vkvg_stroke(ctx);
//    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_ROUND);
//    vkvg_rel_move_to(ctx,-dx*2,abs(dy*1.5));
//    vkvg_rel_line_to(ctx,dx,-dy);
//    vkvg_rel_line_to(ctx,dx,dy);
//    vkvg_stroke(ctx);
//    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_MITER);
}
void test_colinear () {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,0.7,0.7,0.7,1);
    vkvg_paint(ctx);

    vkvg_set_source_rgba(ctx,0,0,0,1);
    vkvg_set_line_width(ctx,10);

    vkvg_move_to(ctx,100,100);
    vkvg_line_to(ctx,100,200);
    vkvg_line_to(ctx,100,100);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}

void multi_test1 () {
    //VkvgSurface surf2 = vkvg_surface_create (device,1024,800);;
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_source_rgba(ctx,0.1,0.1,0.3,1.0);
    vkvg_paint(ctx);

//    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_ROUND);


    //test_line_join(ctx);
//    vkvg_set_source_rgba(ctx,0.0,1.0,0.0,1.0);

//    vkvg_move_to(ctx,50,50);
//    vkvg_rel_line_to(ctx,400,-10);
//    vkvg_rel_line_to(ctx,-200,300);
//    vkvg_close_path(ctx);
//    vkvg_fill(ctx);

//    vkvg_set_source_rgba(ctx,1.0,1.0,0.0,1.0);
//    vkvg_move_to(ctx,100,100);
//    vkvg_line_to(ctx,100,200);
//    vkvg_stroke(ctx);

    //vkvg_test_clip(ctx);

//    vkvg_set_source_rgba (ctx,0.02,0.8,0.3,1.0);
//    vkvg_rectangle (ctx,200,200,300,300);
//    vkvg_fill (ctx);

    //test_line_caps(ctx);


//    test_text(ctx);


//    vkvg_test_fill(ctx);
//    vkvg_test_fill2(ctx);

//    vkvg_test_stroke(ctx);

//    vkvg_translate(ctx, 10,10);
//    vkvg_rotate(ctx, 0.2);
    //vkvg_scale(ctx, 2,2);


//    vkvg_test_gradient (ctx);
//    vkvg_test_curves(ctx);
//    vkvg_test_curves2(ctx);

    test_img_surface(ctx);

    vkvg_destroy(ctx);
//    ctx = vkvg_create(surf);

//    vkvg_set_source_rgba(ctx,0.0,0.0,0.0,1);
//    vkvg_paint(ctx);

//    vkvg_set_source_surface(ctx, surf2, 0, 0);
//    vkvg_paint(ctx);

//    vkvg_destroy(ctx);
//    vkvg_surface_destroy(surf2);
}

void cairo_test_fill_rule (VkvgContext cr){
    vkvg_set_line_width (cr, 6);

    //vkvg_scale(cr,3,3);
//    vkvg_set_source_rgba(cr,1,0,0,1);
//    vkvg_move_to(cr,50,150);
//    vkvg_rel_line_to(cr,50,70);
//    vkvg_rel_line_to(cr,50,0);
//    vkvg_rel_line_to(cr,50,-70);
//    vkvg_rel_line_to(cr,0,-60);
//    vkvg_rel_line_to(cr,-50,-30);
//    vkvg_rel_line_to(cr,-50,0);
//    vkvg_rel_line_to(cr,-50,30);
//    vkvg_close_path(cr);


//    vkvg_set_line_join(cr,VKVG_LINE_JOIN_ROUND);
    vkvg_set_source_rgb (cr, 0, 0.7, 0);
    vkvg_rectangle (cr, 12, 12, 232, 70);
    //vkvg_stroke (cr);
//    vkvg_new_sub_path (cr);
    vkvg_arc (cr, 64, 64, 40, 0, M_PI*2);
    //vkvg_close_path(cr);

    vkvg_new_sub_path (cr);
    vkvg_arc_negative (cr, 192, 64, 40, 2*M_PI, 0);
    //vkvg_close_path(cr);

    //vkvg_rectangle (cr, 30, 30, 20, 200);
    //vkvg_rectangle (cr, 130, 30, 20, 200);
    //vkvg_set_fill_rule (cr, vkvg_FILL_RULE_EVEN_ODD);

    vkvg_fill_preserve(cr);

    vkvg_set_source_rgb (cr, 0, 0, 0);
    vkvg_stroke (cr);
}
void cairo_test_clip (VkvgContext cr){
    vkvg_arc (cr, 128.0, 128.0, 76.8, 0, 2 * M_PI);
    vkvg_clip (cr);

    //vkvg_new_path (cr);  /* current path is not
    //                         consumed by vkvg_clip() */
    vkvg_set_source_rgba(cr, 0, 0, 0, 1);
    vkvg_rectangle (cr, 0, 0, 256, 256);
    vkvg_fill (cr);
    vkvg_set_source_rgba (cr, 0, 1, 0, 1);
    vkvg_move_to (cr, 0, 0);
    vkvg_line_to (cr, 256, 256);
    vkvg_move_to (cr, 256, 0);
    vkvg_line_to (cr, 0, 256);
    vkvg_set_line_width (cr, 10.0);
    vkvg_stroke (cr);
}
void cairo_test_curves (VkvgContext cr){
    float x=25.6,  y=128.0;
    float x1=102.4, y1=230.4,
           x2=153.6, y2=25.6,
           x3=230.4, y3=128.0;

    vkvg_set_source_rgb (cr, 0, 0, 0);
    vkvg_move_to (cr, x, y);
    vkvg_curve_to (cr, x1, y1, x2, y2, x3, y3);

    vkvg_set_line_width (cr, 10.0);
    vkvg_stroke (cr);

    vkvg_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
    vkvg_set_line_width (cr, 6.0);
    vkvg_move_to (cr,x,y);   vkvg_line_to (cr,x1,y1);
    vkvg_move_to (cr,x2,y2); vkvg_line_to (cr,x3,y3);
    vkvg_stroke (cr);
}
void cairo_test_rounded_rect (VkvgContext cr) {
    /* a custom shape that could be wrapped in a function */
    float x0      = 25.6,   /* parameters like vkvg_rectangle */
           y0      = 25.6,
           rect_width  = 204.8,
           rect_height = 204.8,
           radius = 102.4;   /* and an approximate curvature radius */

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

    vkvg_set_source_rgb (cr, 0.5, 0.5, 1);
    vkvg_fill_preserve (cr);
    vkvg_set_source_rgba (cr, 0.5, 0, 0, 0.5);
    vkvg_set_line_width (cr, 10.0);
    vkvg_stroke (cr);
}
void cairo_test_fill_and_stroke2 (VkvgContext cr){
    vkvg_move_to (cr, 128.0, 25.6);
    vkvg_line_to (cr, 230.4, 230.4);
    vkvg_rel_line_to (cr, -102.4, 0.0);
    vkvg_curve_to (cr, 51.2, 230.4, 51.2, 128.0, 128.0, 128.0);
    vkvg_close_path (cr);

    vkvg_move_to (cr, 64.0, 25.6);
    vkvg_rel_line_to (cr, 51.2, 51.2);
    vkvg_rel_line_to (cr, -51.2, 51.2);
    vkvg_rel_line_to (cr, -51.2, -51.2);
    vkvg_close_path (cr);

    /*vkvg_translate(cr,100,100);
    vkvg_move_to (cr, 100, 100);
    vkvg_line_to(cr,300,300);
    vkvg_line_to(cr,100,300);*/


    vkvg_set_line_width (cr, 10.0);
    vkvg_set_source_rgb (cr, 0, 0, 1);
    vkvg_fill_preserve (cr);
    vkvg_set_source_rgb (cr, 0, 0, 0);
    vkvg_stroke (cr);
}
void cairo_print_arc_neg (VkvgContext cr){
    float xc = 128.0;
    float yc = 128.0;
    float radius = 100.0;
    float angle1 = 45.0  * (M_PI/180.0);  /* angles are specified */
    float angle2 = 180.0 * (M_PI/180.0);  /* in radians           */

    vkvg_set_source_rgba(cr, 0, 0, 0, 1);
    vkvg_set_line_width (cr, 10.0);
    vkvg_arc_negative (cr, xc, yc, radius, angle1, angle2);
    vkvg_stroke (cr);

    /* draw helping lines */
    vkvg_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
    vkvg_set_line_width (cr, 6.0);

    vkvg_arc (cr, xc, yc, 10.0, 0, 2*M_PI);
    vkvg_fill (cr);

    vkvg_arc (cr, xc, yc, radius, angle1, angle1);
    vkvg_line_to (cr, xc, yc);
    vkvg_arc (cr, xc, yc, radius, angle2, angle2);
    //vkvg_line_to (cr, xc, yc);
    vkvg_stroke (cr);

}
void cairo_print_arc (VkvgContext cr) {
    float xc = 128.0;
    float yc = 128.0;
    float radius = 100.0;
    float angle1 = 45.0  * (M_PI/180.0);  /* angles are specified */
    float angle2 = 180.0 * (M_PI/180.0);  /* in radians           */

    vkvg_set_source_rgba(cr, 0, 0, 0, 1);
    vkvg_set_line_width (cr, 10.0);
    vkvg_arc (cr, xc, yc, radius, angle1, angle2);
    vkvg_stroke (cr);

    /* draw helping lines */
    vkvg_set_source_rgba(cr, 1, 0.2, 0.2, 0.6);
    vkvg_set_line_width (cr, 6.0);

    vkvg_arc (cr, xc, yc, 10.0, 0, 2*M_PI);
    vkvg_fill (cr);

    vkvg_arc (cr, xc, yc, radius, angle1, angle1);
    vkvg_line_to (cr, xc, yc);
    vkvg_arc (cr, xc, yc, radius, angle2, angle2);
    //vkvg_line_to (cr, xc, yc);
    vkvg_stroke (cr);
}
void cairo_tests () {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,0.7,0.7,0.7,1);
    vkvg_paint(ctx);
    //cairo_print_arc(ctx);
    //cairo_test_clip(ctx);
    //vkvg_translate(ctx,200,0);
    cairo_test_fill_rule(ctx);

    //cairo_test_rounded_rect(ctx);
    //vkvg_translate(ctx,250,0);
    //cairo_test_curves(ctx);
//    vkvg_translate(ctx,200,0);
//    cairo_test_fill_and_stroke2(ctx);
//    vkvg_translate(ctx,-650,300);
    //vkvg_translate(ctx,250,0);
    //cairo_print_arc_neg(ctx);

    //vkvg_test_fill_and_stroke(ctx);
    vkvg_destroy(ctx);
}

void test_grad_transforms () {
    VkvgContext ctx = vkvg_create(surf);

    vkvg_translate(ctx,-100,-100);
    vkvg_rotate(ctx,1.5);
    //vkvg_translate(ctx,100,100);

    //vkvg_scale(ctx,0.2,0.2);
    VkvgPattern pat = vkvg_pattern_create_linear(0,0,200,0);
    vkvg_set_line_width(ctx, 20);
    vkvg_patter_add_color_stop(pat, 0, 1, 0, 0, 1);
    vkvg_patter_add_color_stop(pat, 0.5, 0, 1, 0, 1);
    vkvg_patter_add_color_stop(pat, 1, 0, 0, 1, 1);
    vkvg_set_source (ctx, pat);
    vkvg_rectangle(ctx,0,0,200,200);
    vkvg_fill (ctx);
    //vkvg_stroke (ctx);
    vkvg_pattern_destroy (pat);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    VkEngine* e = vke_create();
    vke_set_key_callback (e, key_callback);

    device = vkvg_device_create(e->phy, e->dev, e->renderer.queue, e->renderer.qFam);
    surf = vkvg_surface_create (device,1024,800);


    //multi_test1();

    //test_grad_transforms();

    cairo_tests();

    //test_colinear();

    vke_init_blit_renderer(e, vkvg_surface_get_vk_image(surf));

    while (!glfwWindowShouldClose(e->renderer.window)) {
        glfwPollEvents();
        draw(e, vkvg_surface_get_vk_image(surf));
    }

    vkDeviceWaitIdle(e->dev);

    vkvg_surface_destroy(surf);
    vkvg_device_destroy(device);

    vke_destroy (e);

    return 0;
}

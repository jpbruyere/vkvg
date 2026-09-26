#include "drawTestBase.h"
#define _USE_MATH_DEFINES
#include <math.h>

class PathDrawTest : public DrawTestBase {

  protected:
    void SetUp() override { surf = vkvg_surface_create(dev, 256, 256); }
};

TEST_F(PathDrawTest, CircleFill) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.8,0.8,0.8);
    vkvg_paint(ctx);

    vkvg_set_source_rgb(ctx, 0.1, 0.2, 1);
    vkvg_arc(ctx, 128, 128, 100, 0, M_PI * 2);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, CircleFillStroke) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.8,0.8,0.8);
    vkvg_paint(ctx);

    vkvg_set_source_rgb(ctx, 0.5, 0.5, 1);
    vkvg_arc(ctx, 128, 128, 100, 0, M_PI * 2);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb(ctx, 0, 0, 0);
    vkvg_set_line_width(ctx, 10);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, CircleFillStrokeLarge) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.8,0.8,0.8);
    vkvg_paint(ctx);

    vkvg_set_source_rgb(ctx, 0.5, 0.5, 1);
    vkvg_arc(ctx, 256, 256, 220, 0, M_PI * 2);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb(ctx, 0, 0, 0);
    vkvg_set_line_width(ctx, 20);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, CircleFillStrokeVeryLarge) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.8,0.8,0.8);
    vkvg_paint(ctx);

    vkvg_set_source_rgb(ctx, 0.5, 0.5, 1);
    vkvg_arc(ctx, 512, 512, 500, 0, M_PI * 2);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb(ctx, 0, 0, 0);
    vkvg_set_line_width(ctx, 20);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, CircleFillStrokeScaleUp10) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.8,0.8,0.8);
    vkvg_paint(ctx);
    vkvg_scale(ctx,10,10);
    vkvg_set_source_rgb(ctx, 0.5, 0.5, 1);
    vkvg_arc(ctx, 12.8, 12.8, 10, 0, M_PI * 2);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb(ctx, 0, 0, 0);
    vkvg_set_line_width(ctx, 1);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, CircleFillStrokeScaleUp100) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.8,0.8,0.8);
    vkvg_paint(ctx);
    vkvg_scale(ctx,100,100);
    vkvg_set_source_rgb(ctx, 0.5, 0.5, 1);
    vkvg_arc(ctx, 1.28f, 1.28f, 1, 0, M_PI * 2);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb(ctx, 0, 0, 0);
    vkvg_set_line_width(ctx, 0.1f);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, CircleFillStrokeScaleDown10) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.8,0.8,0.8);
    vkvg_paint(ctx);
    vkvg_scale(ctx,0.1f,0.1f);
    vkvg_set_source_rgb(ctx, 0.5, 0.5, 1);
    vkvg_arc(ctx, 1280, 1280, 1000, 0, M_PI * 2);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb(ctx, 0, 0, 0);
    vkvg_set_line_width(ctx, 100);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, curved_rect) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.8,0.8,0.8);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
    compareWithRefImage();
}
TEST_F(PathDrawTest, CircleCurve) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.8,0.8,0.8);
    vkvg_paint(ctx);
    vkvg_scale(ctx,0.1,0.1);
    vkvg_set_source_rgb(ctx, 0.5, 0.5, 1);
    vkvg_arc(ctx, 1280, 1280, 1000, 0, M_PI * 2);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgb(ctx, 0, 0, 0);
    vkvg_set_line_width(ctx, 100);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);

    compareWithRefImage();
}
void emitCubic(VkvgContext ctx, const float x1, const float y1, const float cpx1, const float cpy1, const float cpx2, const float cpy2, const float x2, const float y2) {
    vkvg_set_source_rgb(ctx, 0.8,0.8,0.8);
    vkvg_paint(ctx);

    vkvg_move_to(ctx,x1,y1);
    vkvg_curve_to(ctx, cpx1, cpy1, cpx2 ,cpy2, x2 ,y2);
    vkvg_set_line_width(ctx,10);
    vkvg_set_source_rgb(ctx, 0.5, 0.5, 1);
    vkvg_stroke(ctx);
    vkvg_set_line_width(ctx,1);
    vkvg_set_source_rgb(ctx, 0, 0, 0);
    vkvg_move_to(ctx,x1,y1);
    vkvg_line_to(ctx,cpx1,cpy1);
    vkvg_move_to(ctx,x2,y2);
    vkvg_line_to(ctx,cpx2,cpy2);
    vkvg_stroke(ctx);
    vkvg_arc(ctx, x1, y1, 5, 0, M_PI * 2);
    vkvg_arc(ctx, x2, y2, 5, 0, M_PI * 2);
    vkvg_fill(ctx);
    vkvg_set_source_rgb(ctx, 1, 0.2, 0.2);
    vkvg_arc(ctx, cpx1, cpy1, 5, 0, M_PI * 2);
    vkvg_arc(ctx, cpx2, cpy2, 5, 0, M_PI * 2);
    vkvg_fill(ctx);
}
void emitCubicScaled(VkvgContext ctx, const float x1, const float y1, const float cpx1, const float cpy1, const float cpx2, const float cpy2, const float x2, const float y2, const float scale) {
    vkvg_set_source_rgb(ctx, 0.8,0.8,0.8);
    vkvg_paint(ctx);

    vkvg_scale(ctx,scale,scale);

    vkvg_move_to(ctx,x1,y1);
    vkvg_curve_to(ctx, cpx1, cpy1, cpx2 ,cpy2, x2 ,y2);
    vkvg_set_line_width(ctx,10.f/scale);
    vkvg_set_source_rgb(ctx, 0.5, 0.5, 1);
    vkvg_stroke(ctx);
    vkvg_set_line_width(ctx,1.f/scale);
    vkvg_set_source_rgb(ctx, 0, 0, 0);
    vkvg_move_to(ctx,x1,y1);
    vkvg_line_to(ctx,cpx1,cpy1);
    vkvg_move_to(ctx,x2,y2);
    vkvg_line_to(ctx,cpx2,cpy2);
    vkvg_stroke(ctx);
    vkvg_arc(ctx, x1, y1, 5.f/scale, 0, M_PI * 2);
    vkvg_arc(ctx, x2, y2, 5.f/scale, 0, M_PI * 2);
    vkvg_fill(ctx);
    vkvg_set_source_rgb(ctx, 1, 0.2, 0.2);
    vkvg_arc(ctx, cpx1, cpy1, 5.f/scale, 0, M_PI * 2);
    vkvg_arc(ctx, cpx2, cpy2, 5.f/scale, 0, M_PI * 2);
    vkvg_fill(ctx);
}

TEST_F(PathDrawTest, cubic1) {
    VkvgContext ctx = vkvg_create(surf);
    emitCubic(ctx,20,236,20,30,30,20,236,20);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, cubic2) {
    VkvgContext ctx = vkvg_create(surf);
    emitCubic(ctx,128,220, 30, 128, 128 ,20, 236 ,128);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, cubic3) {
    VkvgContext ctx = vkvg_create(surf);
    emitCubic(ctx,128,236, 236 ,30, 30, 236, 236 ,128);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, cubic4) {
    VkvgContext ctx = vkvg_create(surf);
    emitCubic(ctx,140, 200,20, 240, 240,20, 200 ,140);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, cubic1ScaleUp10) {
    VkvgContext ctx = vkvg_create(surf);
    emitCubicScaled(ctx,2,23.6,2,3,3,2,23.6,2, 10);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, cubic2ScaleUp10) {
    VkvgContext ctx = vkvg_create(surf);
    emitCubicScaled(ctx,12.8,22, 3, 12.8, 12.8 ,2, 23.6 ,12.8, 10);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, cubic3ScaleUp10) {
    VkvgContext ctx = vkvg_create(surf);
    emitCubicScaled(ctx,12.8,23.6, 23.6 ,3, 3, 23.6, 23.6 ,12.8, 10);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, cubic4ScaleUp10) {
    VkvgContext ctx = vkvg_create(surf);
    emitCubicScaled(ctx,14, 20,2, 24, 24,2, 20 ,14, 10);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, cubic1ScaleUp100) {
    VkvgContext ctx = vkvg_create(surf);
    emitCubicScaled(ctx,0.2f,2.36f,0.2f,0.3f,0.3f,0.2f,2.36f, 0.2f, 100);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, cubic2ScaleUp100) {
    VkvgContext ctx = vkvg_create(surf);
    emitCubicScaled(ctx,1.28f,2.2f, 0.3f, 1.28f, 1.28f ,0.2f, 2.36f ,1.28f, 100);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, cubic3ScaleUp100) {
    VkvgContext ctx = vkvg_create(surf);
    emitCubicScaled(ctx,1.28f,2.36f, 2.36f ,0.3f, 0.3f, 2.36f, 2.36f ,1.28f, 100);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, cubic4ScaleUp100) {
    VkvgContext ctx = vkvg_create(surf);
    emitCubicScaled(ctx,1.4f, 2,0.2f, 2.4f, 2.4f,0.2f, 2 ,1.4f, 100);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, curve2) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.8,0.8,0.8);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PathDrawTest, curve3) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.8,0.8,0.8);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);

    compareWithRefImage();
}

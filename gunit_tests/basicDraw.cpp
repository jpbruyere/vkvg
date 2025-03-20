#include "drawTestBase.h"

class BasicDrawTest : public DrawTestBase {

  protected:

    void SetUp() override {
        surf = vkvg_surface_create(dev, 256, 256);
    }
};

TEST_F(BasicDrawTest, CtxSolidPaintRGB) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx,1,0,0);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(BasicDrawTest, CtxSolidPaintRGBA) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(BasicDrawTest, CtxSolidPaintWithActiveTransform) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_translate(ctx,100,100);
    vkvg_rotate(ctx, 2.f);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(BasicDrawTest, CtxSolidPaintOver) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,1,0,0,0.5);
    vkvg_paint(ctx);
    vkvg_set_source_rgba(ctx,0,0,1,0.5);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(BasicDrawTest, CtxSolidPaintRectangle) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx,1,0,0);
    vkvg_rectangle(ctx,50,50,200,150);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(BasicDrawTest, CtxSolidPaintRectanglesOver) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,1,0,0,0.5f);
    vkvg_rectangle(ctx,50,50,100,100);
    vkvg_paint(ctx);
    vkvg_set_source_rgba(ctx,0,0,1,0.5f);
    vkvg_rectangle(ctx,100,100,100,100);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(BasicDrawTest, CtxSolidFillOver) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,1,0,0,0.5);
    vkvg_rectangle(ctx,50,50,100,100);
    vkvg_fill(ctx);
    vkvg_set_source_rgba(ctx,0,0,1,0.5);
    vkvg_rectangle(ctx,100,100,100,100);
    vkvg_fill(ctx);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(BasicDrawTest, CtxSolidPaintRectangleOver) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,1,0,0,0.5f);
    vkvg_paint(ctx);
    vkvg_set_source_rgba(ctx,0,0,1,0.5f);
    vkvg_rectangle(ctx,100,100,100,100);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(BasicDrawTest, CtxSolidPaintRectangleOverClipped) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_rectangle(ctx,90,90,70,70);
    vkvg_clip(ctx);
    vkvg_set_source_rgba(ctx,1,0,0,0.5);
    vkvg_rectangle(ctx,50,50,100,100);
    vkvg_fill(ctx);
    vkvg_set_source_rgba(ctx,0,0,1,0.5);
    vkvg_rectangle(ctx,100,100,100,100);
    vkvg_fill(ctx);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(BasicDrawTest, CtxSolidPaintClipped) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_rectangle(ctx,50,50,200,150);
    vkvg_clip(ctx);

    vkvg_set_source_rgba(ctx,1,0,0,0.5f);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(BasicDrawTest, CtxSolidPaintClippedOver) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,1,0,0,0.5f);
    vkvg_paint(ctx);
    vkvg_rectangle(ctx,50,50,200,150);
    vkvg_clip(ctx);
    vkvg_set_source_rgba(ctx,0,0,1,0.5f);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);

    compareWithRefImage();
}

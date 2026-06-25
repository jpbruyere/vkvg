#include "drawTestBase.h"
#include <math.h>

class SaveRestoreTest : public DrawTestBase {

  protected:
    fs::path    imgPath = fs::path(GTEST_DATA_ROOT) / "img512.png";
    VkvgSurface imgSurf;

    void SetUp() override {
        surf    = vkvg_surface_create(dev, 512, 512);
        imgSurf = vkvg_surface_create_from_image(dev, (char*)imgPath.c_str());
    }
    void TearDown() override {
        vkvg_surface_destroy(imgSurf);
        DrawTestBase::TearDown();
    }
    void recurse_draw(VkvgContext ctx, int depth, int maxDepth) {
        static float gap = 10.f;
        depth++;
        float depthRatio = (float)depth / maxDepth;

        vkvg_rectangle(ctx, gap * depth, gap * depth, 512.f - gap * 2.f * depth, 512.f - gap * 2.f * depth);
        vkvg_clip(ctx);
        vkvg_set_source_rgb(ctx, depthRatio, depthRatio, 1.f);
        vkvg_paint(ctx);
        vkvg_arc(ctx, 256, 256, 256.f - gap * depth, 0, 2.f*M_PI);
        vkvg_clip(ctx);
        vkvg_set_source_surface(ctx, imgSurf, 0, 0);
        vkvg_paint(ctx);

        vkvg_save(ctx);

        if (depth < maxDepth)
            recurse_draw(ctx, depth, maxDepth);
        float a = M_PI * 0.5f * depthRatio;
        float x = cos(a) * 256;
        float y = sin(a) * 256;

        vkvg_set_line_width(ctx, 2.f);
        vkvg_move_to(ctx, 256.f - x, 256.f - y);
        vkvg_line_to(ctx, 256.f + x, 256.f + y);
        vkvg_set_source_rgba(ctx, 0.f, 1.f, 0.f, 0.6f);
        vkvg_stroke(ctx);

        vkvg_restore(ctx);
        EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_status(ctx));

        depth--;
    }
};

TEST_F(SaveRestoreTest, RestoreClip1) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_arc(ctx, 140, 140, 100, 0, 2.f*M_PI);
    vkvg_clip(ctx);
    vkvg_save(ctx);
    vkvg_reset_clip(ctx);
    vkvg_restore(ctx);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);

    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_status(ctx));
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(SaveRestoreTest, RecursePaint3) {
    VkvgContext ctx = vkvg_create(surf);

    EXPECT_NO_FATAL_FAILURE(recurse_draw(ctx, 0, 3));

    vkvg_destroy(ctx);

    compareWithRefImage();
}

TEST_F(SaveRestoreTest, RecursePaint6) {
    VkvgContext ctx = vkvg_create(surf);

    EXPECT_NO_FATAL_FAILURE(recurse_draw(ctx, 0, 6));

    vkvg_destroy(ctx);

    compareWithRefImage();
}

TEST_F(SaveRestoreTest, RecursePaint9) {
    VkvgContext ctx = vkvg_create(surf);

    EXPECT_NO_FATAL_FAILURE(recurse_draw(ctx, 0, 9));

    vkvg_destroy(ctx);

    compareWithRefImage();
}

TEST_F(SaveRestoreTest, RecursePaint10) {
    VkvgContext ctx = vkvg_create(surf);

    EXPECT_NO_FATAL_FAILURE(recurse_draw(ctx, 0, 10));

    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(SaveRestoreTest, RecursePaint11) {
    VkvgContext ctx = vkvg_create(surf);

    EXPECT_NO_FATAL_FAILURE(recurse_draw(ctx, 0, 11));

    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(SaveRestoreTest, RecursePaint20) {
    VkvgContext ctx = vkvg_create(surf);

    EXPECT_NO_FATAL_FAILURE(recurse_draw(ctx, 0, 20));

    vkvg_destroy(ctx);

    compareWithRefImage();
}

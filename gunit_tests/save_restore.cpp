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
        static float gap = 15.f;
        depth++;
        vkvg_save(ctx);

        vkvg_rectangle(ctx, gap * depth, gap * depth, 512.f - gap * 2.f * depth, 512.f - gap * 2.f * depth);
        vkvg_clip(ctx);
        vkvg_set_source_rgb(ctx, 0.3f/depth, 1.f / depth, 1.f);
        vkvg_paint(ctx);
        vkvg_arc(ctx, 256, 256, 256.f - gap * depth, 0, 2.f*M_PI);
        vkvg_clip(ctx);
        vkvg_set_source_surface(ctx, imgSurf, 0, 0);
        vkvg_paint(ctx);

        if (depth < maxDepth)
            recurse_draw(ctx, depth, maxDepth);

        vkvg_restore(ctx);
        EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_status(ctx));
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

TEST_F(SaveRestoreTest, RecursePaint5) {
    VkvgContext ctx = vkvg_create(surf);

    EXPECT_NO_FATAL_FAILURE(recurse_draw(ctx, 0, 5));

    vkvg_destroy(ctx);

    compareWithRefImage();
}


TEST_F(SaveRestoreTest, RecursePaint10) {
    VkvgContext ctx = vkvg_create(surf);

    EXPECT_NO_FATAL_FAILURE(recurse_draw(ctx, 0, 10));

    vkvg_destroy(ctx);

    compareWithRefImage();
}

TEST_F(SaveRestoreTest, RecursePaint16) {
    VkvgContext ctx = vkvg_create(surf);

    EXPECT_NO_THROW(recurse_draw(ctx, 0, 16));

    vkvg_destroy(ctx);

    compareWithRefImage();
}

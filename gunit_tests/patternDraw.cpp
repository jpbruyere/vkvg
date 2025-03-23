#include "drawTestBase.h"

class PatternDrawTest : public DrawTestBase {

  protected:
    fs::path imgPath = fs::path(GTEST_DATA_ROOT) / "miroir2.png";
    VkvgSurface imgSurf;

    void SetUp() override {
        surf = vkvg_surface_create(dev, 512, 512);
        imgSurf = vkvg_surface_create_from_image(dev, (char*)imgPath.c_str());
    }
    void TearDown() override {
        vkvg_surface_destroy(imgSurf);
        DrawTestBase::TearDown();
    }
};
TEST_F(PatternDrawTest, References) {
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_pattern_status(NULL));
    EXPECT_EQ(0, vkvg_pattern_get_reference_count(NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_pattern_reference(NULL));

    VkvgPattern pat = vkvg_pattern_create_for_surface(NULL);
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_pattern_status(pat));

    pat = vkvg_pattern_create_linear(0,0,0,0);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_status(pat));
    EXPECT_EQ(1, vkvg_pattern_get_reference_count(pat));
    vkvg_pattern_reference(pat);
    EXPECT_EQ(2, vkvg_pattern_get_reference_count(pat));

    EXPECT_NO_FATAL_FAILURE(vkvg_pattern_destroy(NULL));
    vkvg_pattern_destroy(pat);
    EXPECT_EQ(1, vkvg_pattern_get_reference_count(pat));
    vkvg_pattern_destroy(pat);
    //EXPECT_EQ(NULL, pat);
}

TEST_F(PatternDrawTest, PaintPattern) {

    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = vkvg_pattern_create_for_surface(imgSurf);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_status(pat));
    EXPECT_EQ(1, vkvg_pattern_get_reference_count(pat));
    EXPECT_EQ(2, vkvg_surface_get_reference_count(imgSurf));
    vkvg_set_source(ctx, pat);
    EXPECT_EQ(2, vkvg_pattern_get_reference_count(pat));
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    EXPECT_EQ(1, vkvg_pattern_get_reference_count(pat));
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintPatternRepeat) {
    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_REPEAT);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintPatternRepeatScaled) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_scale(ctx, 0.2f, 0.2f);
    VkvgPattern pat = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_REPEAT);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintPatternPad) {
    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_PAD);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}

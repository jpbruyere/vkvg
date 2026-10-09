#include "drawTestBase.h"
#include "unitTest.h"


class TextDrawTest : public DrawTestBase {

  protected:
    void SetUp() override {
        surf    = vkvg_surface_create(dev, 512, 512);

    }
    void TearDown() override {
        DrawTestBase::TearDown();
    }
};
TEST_F(TextDrawTest, References) {
    /*EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_font_status(NULL));
    EXPECT_EQ(0, vkvg_pattern_get_reference_count(NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_pattern_reference(NULL));

    VkvgPattern pat = vkvg_pattern_create_for_surface(NULL);
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_pattern_status(pat));

    pat = vkvg_pattern_create_linear(0, 0, 0, 0);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_status(pat));
    EXPECT_EQ(1, vkvg_pattern_get_reference_count(pat));
    vkvg_pattern_reference(pat);
    EXPECT_EQ(2, vkvg_pattern_get_reference_count(pat));

    EXPECT_NO_FATAL_FAILURE(vkvg_pattern_destroy(NULL));
    vkvg_pattern_destroy(pat);
    EXPECT_EQ(1, vkvg_pattern_get_reference_count(pat));
    vkvg_pattern_destroy(pat);

    pat = vkvg_pattern_create_radial(0, 0, 0, 0, 0, 0);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_status(pat));
    EXPECT_EQ(1, vkvg_pattern_get_reference_count(pat));
    vkvg_pattern_destroy(pat);*/
}

TEST_F(TextDrawTest, PaintImgPattern) {

    VkvgContext ctx = vkvg_create(surf);
    vkvg_destroy(ctx);

    compareWithRefImage();
}

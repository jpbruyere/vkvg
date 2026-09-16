#include "drawTestBase.h"
#include "unitTest.h"


class PatternDrawTest : public DrawTestBase {

  protected:
    fs::path    imgPath = fs::path(GTEST_DATA_ROOT) / "mirror2.png";
    VkvgSurface imgSurf;

    void SetUp() override {
        surf    = vkvg_surface_create(dev, 512, 512);
        imgSurf = vkvg_surface_create_from_image(dev, (char*)imgPath.c_str());
    }
    void TearDown() override {
        vkvg_surface_destroy(imgSurf);
        DrawTestBase::TearDown();
    }
    VkvgPattern CreateLinearPattern () {
        VkvgPattern pat = vkvg_pattern_create_linear(0, 0, 200, 0);
        vkvg_pattern_add_color_stop(pat, 0.0, 1, 0, 0, 1);
        vkvg_pattern_add_color_stop(pat, 0.5, 0, 1, 0, 1);
        vkvg_pattern_add_color_stop(pat, 1.0, 0, 0, 1, 1);
        return pat;
    }
    VkvgPattern CreateRadialPattern () {
        VkvgPattern pat = vkvg_pattern_create_radial(150.0f, 150.0f, 10.f, 170, 170, 130.0f);
        vkvg_pattern_add_color_stop(pat, 0.0, 1, 0, 0, 1);
        vkvg_pattern_add_color_stop(pat, 0.5, 0, 1, 0, 1);
        vkvg_pattern_add_color_stop(pat, 1.0, 0, 0, 1, 1);
        return pat;
    }
};
TEST_F(PatternDrawTest, References) {
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_pattern_status(NULL));
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
    vkvg_pattern_destroy(pat);
}

TEST_F(PatternDrawTest, CreateSetGet) {
    VkvgContext ctx = vkvg_create(surf);

    EXPECT_NO_FATAL_FAILURE(vkvg_set_source(ctx, NULL));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_status(ctx));

    VkvgPattern pat = vkvg_pattern_create_linear(0, 0, 200, 0);

    EXPECT_EQ(VKVG_PATTERN_TYPE_LINEAR, vkvg_pattern_get_type(pat));
    EXPECT_EQ(VKVG_EXTEND_NONE, vkvg_pattern_get_extend(pat));
    EXPECT_EQ(VKVG_FILTER_FAST, vkvg_pattern_get_filter(pat));

    vkvg_pattern_add_color_stop(pat, 0, 1, 0, 0, 1);
    vkvg_pattern_add_color_stop(pat, 0.5, 0, 1, 0, 1);
    vkvg_pattern_add_color_stop(pat, 1.0, 0, 0, 1, 1);

    uint32_t ccpt = 0;
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_pattern_get_color_stop_count(NULL, &ccpt));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_get_color_stop_count(pat, &ccpt));
    EXPECT_EQ(3, ccpt);

    float r,g,b,a,offset;
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_get_color_stop_rgba(pat, 0, &offset, &r, &g, &b, &a));
    EXPECT_EQ(0, offset);
    EXPECT_EQ(1, r); EXPECT_EQ(0, g); EXPECT_EQ(0, b); EXPECT_EQ(1, a);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_get_color_stop_rgba(pat, 1, &offset, &r, &g, &b, &a));
    EXPECT_EQ(0.5, offset);
    EXPECT_EQ(0, r); EXPECT_EQ(1, g); EXPECT_EQ(0, b); EXPECT_EQ(1, a);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_get_color_stop_rgba(pat, 2, &offset, &r, &g, &b, &a));
    EXPECT_EQ(0, r); EXPECT_EQ(0, g); EXPECT_EQ(1, b); EXPECT_EQ(1, a);
    EXPECT_EQ(1, offset);
    EXPECT_EQ(VKVG_STATUS_INVALID_INDEX, vkvg_pattern_get_color_stop_rgba(pat, 3, &offset, &r, &g, &b, &a));

    vkvg_matrix_t mat = {0};
    vkvg_pattern_get_matrix(pat, &mat);
    CHECK_MAT(1, 0, 0, 1, 0, 0);
    vkvg_matrix_translate(&mat, 100, 50);
    vkvg_pattern_set_matrix(pat, &mat);
    vkvg_pattern_get_matrix(pat, &mat);
    CHECK_MAT(1, 0, 0, 1, 100, 50);

    vkvg_set_source(ctx, pat);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_status(ctx));
    EXPECT_EQ(2, vkvg_pattern_get_reference_count(pat));
    vkvg_save(ctx);
    EXPECT_EQ(3, vkvg_pattern_get_reference_count(pat));
    vkvg_restore(ctx);
    EXPECT_EQ(2, vkvg_pattern_get_reference_count(pat));

    pat = vkvg_get_source(ctx);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_status(pat));
    EXPECT_EQ(VKVG_PATTERN_TYPE_LINEAR, vkvg_pattern_get_type(pat));
    EXPECT_EQ(3, vkvg_pattern_get_reference_count(pat));

    vkvg_set_source_rgb(ctx, 1, 0, 0);
    EXPECT_EQ(2, vkvg_pattern_get_reference_count(pat));

    vkvg_pattern_destroy(pat);
    vkvg_pattern_destroy(pat);

    pat = vkvg_pattern_create_radial(250.0f, 250.0f, 0, 300, 300, 260.0f);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_status(pat));
    EXPECT_EQ(VKVG_PATTERN_TYPE_RADIAL, vkvg_pattern_get_type(pat));
    EXPECT_EQ(VKVG_EXTEND_NONE, vkvg_pattern_get_extend(pat));
    EXPECT_EQ(VKVG_FILTER_FAST, vkvg_pattern_get_filter(pat));

    vkvg_pattern_add_color_stop(pat, 0.0, 1, 0, 0, 1);
    vkvg_pattern_add_color_stop(pat, 0.5, 0, 1, 0, 1);
    vkvg_pattern_add_color_stop(pat, 1.0, 0, 0, 0, 1);

    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_get_color_stop_count(pat, &ccpt));
    EXPECT_EQ(3, ccpt);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_get_color_stop_rgba(pat, 0, &offset, &r, &g, &b, &a));
    EXPECT_EQ(0, offset);
    EXPECT_EQ(1, r); EXPECT_EQ(0, g); EXPECT_EQ(0, b); EXPECT_EQ(1, a);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_get_color_stop_rgba(pat, 1, &offset, &r, &g, &b, &a));
    EXPECT_EQ(0.5, offset);
    EXPECT_EQ(0, r); EXPECT_EQ(1, g); EXPECT_EQ(0, b); EXPECT_EQ(1, a);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_get_color_stop_rgba(pat, 2, &offset, &r, &g, &b, &a));
    EXPECT_EQ(0, r); EXPECT_EQ(0, g); EXPECT_EQ(0, b); EXPECT_EQ(1, a);
    EXPECT_EQ(1, offset);
    EXPECT_EQ(VKVG_STATUS_INVALID_INDEX, vkvg_pattern_get_color_stop_rgba(pat, 3, &offset, &r, &g, &b, &a));

    vkvg_set_source(ctx, pat);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_status(ctx));
    EXPECT_EQ(2, vkvg_pattern_get_reference_count(pat));
    vkvg_save(ctx);
    EXPECT_EQ(3, vkvg_pattern_get_reference_count(pat));
    vkvg_restore(ctx);
    EXPECT_EQ(2, vkvg_pattern_get_reference_count(pat));

    pat = vkvg_get_source(ctx);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_status(pat));
    EXPECT_EQ(VKVG_PATTERN_TYPE_RADIAL, vkvg_pattern_get_type(pat));
    EXPECT_EQ(3, vkvg_pattern_get_reference_count(pat));

    vkvg_set_source_rgb(ctx, 1, 0, 0);
    EXPECT_EQ(2, vkvg_pattern_get_reference_count(pat));

    vkvg_pattern_destroy(pat);
    vkvg_pattern_destroy(pat);

    vkvg_destroy(ctx);
}

TEST_F(PatternDrawTest, PaintImgPattern) {

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
TEST_F(PatternDrawTest, FillImgPattern) {

    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_rectangle(ctx, 70, 70, 200, 160);
    vkvg_set_source(ctx, pat);
    vkvg_fill(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, FillImgPatternTransformed) {

    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_rotate(ctx, 0.3f);
    vkvg_rectangle(ctx, 50, 50, 200, 160);
    vkvg_set_source(ctx, pat);
    vkvg_fill(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, FillImgTransformedPattern) {

    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_matrix_t mat = {0};
    vkvg_matrix_init_rotate(&mat, 0.3f);
    vkvg_pattern_set_matrix(pat, &mat);
    vkvg_rectangle(ctx, 50, 50, 200, 160);
    vkvg_set_source(ctx, pat);
    vkvg_fill(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, StrokeImgPattern) {

    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_set_line_width(ctx, 20);
    vkvg_rectangle(ctx, 50, 50, 200, 160);
    vkvg_set_source(ctx, pat);
    vkvg_stroke(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintImgPatternRepeat) {
    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_REPEAT);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintImgPatternRepeatScaled) {
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
TEST_F(PatternDrawTest, PaintImgPatternPad) {
    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_PAD);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintLinearPattern) {

    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = CreateLinearPattern();
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, FillLinearPatternTransformed) {

    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = CreateLinearPattern();
    vkvg_rotate(ctx, 0.3f);
    vkvg_rectangle(ctx, 50, 50, 200, 160);
    vkvg_set_source(ctx, pat);
    vkvg_fill(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, FillLinearTransformedPattern) {

    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = CreateLinearPattern();
    vkvg_matrix_t mat = {0};
    vkvg_matrix_init_rotate(&mat, 0.3f);
    vkvg_pattern_set_matrix(pat, &mat);
    vkvg_rectangle(ctx, 50, 50, 200, 160);
    vkvg_set_source(ctx, pat);
    vkvg_fill(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintLinearPatternRepeat) {
    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = CreateLinearPattern();
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_REPEAT);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintLinearPatternRepeatScaled) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_scale(ctx, 0.2f, 0.2f);
    VkvgPattern pat = CreateLinearPattern();
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_REPEAT);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintLinearPatternPad) {
    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = CreateLinearPattern();
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_PAD);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintRadialPattern) {

    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = CreateRadialPattern();
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, FillRadialPatternTransformed) {

    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = CreateRadialPattern();
    vkvg_rotate(ctx, 0.3f);
    vkvg_rectangle(ctx, 50, 50, 200, 160);
    vkvg_set_source(ctx, pat);
    vkvg_fill(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, FillRadialTransformedPattern) {

    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = CreateRadialPattern();
    vkvg_matrix_t mat = {0};
    vkvg_matrix_init_rotate(&mat, 0.3f);
    vkvg_pattern_set_matrix(pat, &mat);
    vkvg_rectangle(ctx, 50, 50, 200, 160);
    vkvg_set_source(ctx, pat);
    vkvg_fill(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintRadialPatternRepeat) {
    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = CreateRadialPattern();
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_REPEAT);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintRadialPatternRepeatScaled) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_scale(ctx, 0.2f, 0.2f);
    VkvgPattern pat = CreateRadialPattern();
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_REPEAT);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintRadialPatternPad) {
    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = CreateRadialPattern();
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_PAD);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintLinearPatternTransformed) {

    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = CreateLinearPattern();
    vkvg_matrix_t mat = {0};
    vkvg_matrix_init_rotate(&mat, 0.3f);
    vkvg_matrix_translate(&mat, -20, -20);
    vkvg_matrix_scale(&mat, 0.5f, 0.5f);
    vkvg_pattern_set_matrix(pat, &mat);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(PatternDrawTest, PaintRadialPatternTransformed) {

    VkvgContext ctx = vkvg_create(surf);
    VkvgPattern pat = CreateRadialPattern();
    vkvg_matrix_t mat = {0};
    vkvg_matrix_init_rotate(&mat, 0.3f);
    vkvg_matrix_translate(&mat, -20, -20);
    vkvg_matrix_scale(&mat, 0.5f, 0.5f);
    vkvg_pattern_set_matrix(pat, &mat);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
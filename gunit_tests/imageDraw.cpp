#include "drawTestBase.h"

class ImageDrawTest : public DrawTestBase {

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
TEST_F(ImageDrawTest, References) {
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_surface_status(NULL));
    EXPECT_EQ(0, vkvg_surface_get_reference_count(NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_surface_reference(NULL));

    VkvgSurface surf2 = vkvg_surface_create_from_image(NULL, NULL);
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_surface_status(surf2));
    surf2 = vkvg_surface_create_from_image(dev, NULL);
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_surface_status(surf2));

    uint32_t devRefCntInit = vkvg_device_get_reference_count(dev);

    surf2 = vkvg_surface_create_from_image(dev, (char*)imgPath.c_str());
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf2));
    EXPECT_EQ(1, vkvg_surface_get_reference_count(surf2));
    EXPECT_EQ(devRefCntInit + 1, vkvg_device_get_reference_count(dev));
    EXPECT_EQ(285, vkvg_surface_get_width(surf2));
    EXPECT_EQ(285, vkvg_surface_get_height(surf2));

    vkvg_surface_reference(surf2);
    EXPECT_EQ(2, vkvg_surface_get_reference_count(surf2));

    vkvg_surface_destroy(surf2);

    EXPECT_EQ(1, vkvg_surface_get_reference_count(surf2));
    vkvg_surface_destroy(surf2);
    //EXPECT_EQ(NULL, surf2);
    EXPECT_EQ(devRefCntInit, vkvg_device_get_reference_count(dev));
}

TEST_F(ImageDrawTest, PaintImage) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    EXPECT_EQ(2, vkvg_surface_get_reference_count(imgSurf));
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
    EXPECT_EQ(1, vkvg_surface_get_reference_count(imgSurf));

    compareWithRefImage();
}
TEST_F(ImageDrawTest, PaintImageOffset) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.5f, 0.5f, 0.5f);
    vkvg_paint(ctx);

    vkvg_set_source_surface(ctx, imgSurf, 50, 50);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(ImageDrawTest, PaintImageRotateOffset) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.5f, 0.5f, 0.5f);
    vkvg_paint(ctx);

    vkvg_rotate(ctx, 1.2f);
    vkvg_set_source_surface(ctx, imgSurf, 100, 100);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(ImageDrawTest, PaintImageRotateInPlace) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.5f, 0.5f, 0.5f);
    vkvg_paint(ctx);
    vkvg_matrix_t mat;
    vkvg_matrix_init_translate(&mat, 142,142);
    vkvg_matrix_rotate(&mat,0.1f);
    vkvg_matrix_init_translate(&mat, -142,-142);

    vkvg_set_source_surface(ctx, imgSurf, 50, 50);
    vkvg_set_matrix(ctx, &mat);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);

    compareWithRefImage();
}

TEST_F(ImageDrawTest, PaintImageTransform) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0.5f, 0.5f, 0.5f);
    vkvg_paint(ctx);

    fs::path imgPath = fs::path(GTEST_DATA_ROOT) / "miroir2-64.png";
    VkvgSurface imgSurf = vkvg_surface_create_from_image(dev, (char*)imgPath.c_str());

    vkvg_translate(ctx, 10, 10);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);

    vkvg_identity_matrix(ctx);
    vkvg_translate(ctx, 80, 0);
    vkvg_scale(ctx, 2.f, 1.f);
    vkvg_paint(ctx);

    vkvg_identity_matrix(ctx);
    vkvg_translate(ctx, 240, 0);
    vkvg_scale(ctx, 2.f, 2.f);
    vkvg_paint(ctx);

    vkvg_identity_matrix(ctx);
    vkvg_translate(ctx, 0, 80);
    vkvg_translate(ctx, -32, -32);
    vkvg_rotate(ctx, 1.2f);
    vkvg_translate(ctx, 32, 32);

    vkvg_paint(ctx);

    vkvg_destroy(ctx);

    compareWithRefImage();
}

TEST_F(ImageDrawTest, PaintImageOnImage) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);

    fs::path imgPath2 = fs::path(GTEST_DATA_ROOT) / "filled.png";
    VkvgSurface imgSurf2 = vkvg_surface_create_from_image(dev, (char*)imgPath2.c_str());

    vkvg_set_source_surface(ctx, imgSurf2, 50, 50);
    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf2);
    vkvg_destroy(ctx);

    compareWithRefImage();
}


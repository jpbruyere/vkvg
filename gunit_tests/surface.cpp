#include "vkvg.h"
#include <gtest/gtest.h>

// The fixture for testing class Foo.
class SurfaceTest : public testing::Test {
  public:
    VkvgDevice dev;

  protected:
    // You can remove any or all of the following functions if their bodies would
    // be empty.

    SurfaceTest() {
        vkvg_device_create_info_t info{};
        dev = vkvg_device_create(&info);
    }

    ~SurfaceTest() override { vkvg_device_destroy(dev); }

    void SetUp() override {}

    void TearDown() override {}

    void checkPixels(VkvgSurface surf, uint32_t expectedPixelValue) {
        uint32_t w = vkvg_surface_get_width(surf);
        uint32_t h = vkvg_surface_get_height(surf);
        uint32_t* buff = (uint32_t*)malloc(w * h * sizeof(uint32_t));
        vkvg_surface_write_to_memory(surf, (unsigned char*)buff);

        for (int i = 0; i < w * h; ++i) {
            EXPECT_EQ(expectedPixelValue, buff[i]);
        }
        free(buff);
    }
};

TEST_F(SurfaceTest, SurfCreate) {
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_surface_status(NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_surface_destroy(NULL));

    VkvgSurface surf = vkvg_surface_create(NULL, 0, 0);
    EXPECT_EQ(VKVG_STATUS_DEVICE_ERROR, vkvg_surface_status(surf));
    EXPECT_NO_FATAL_FAILURE(vkvg_surface_reference(surf));

    surf = vkvg_surface_create(dev, 512, 512);

    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));

    vkvg_surface_destroy(surf);
}

TEST_F(SurfaceTest, SurfReference) {
    EXPECT_EQ(0, vkvg_surface_get_reference_count(NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_surface_reference(NULL));

    VkvgSurface surf = vkvg_surface_create(NULL, 0, 0);
    EXPECT_EQ(0, vkvg_surface_get_reference_count(surf));
    EXPECT_EQ(VKVG_STATUS_DEVICE_ERROR, vkvg_surface_status(surf));
    EXPECT_NO_FATAL_FAILURE(vkvg_surface_reference(surf));

    uint32_t devRefCntInit = vkvg_device_get_reference_count(dev);

    surf = vkvg_surface_create(dev, 512, 512);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));
    EXPECT_EQ(devRefCntInit + 1, vkvg_device_get_reference_count(dev));
    EXPECT_EQ(1, vkvg_surface_get_reference_count(surf));

    vkvg_surface_reference(surf);
    EXPECT_EQ(2, vkvg_surface_get_reference_count(surf));

    vkvg_surface_destroy(surf);
    EXPECT_EQ(1, vkvg_surface_get_reference_count(surf));

    vkvg_surface_destroy(surf);
    EXPECT_EQ(devRefCntInit, vkvg_device_get_reference_count(dev));
    //EXPECT_EQ(0, vkvg_surface_get_reference_count(surf));
}
TEST_F(SurfaceTest, SurfProperties) {
    EXPECT_EQ(0, vkvg_surface_get_width(NULL));
    EXPECT_EQ(0, vkvg_surface_get_height(NULL));
    EXPECT_EQ(NULL, vkvg_surface_get_vk_image(NULL));
    EXPECT_EQ(VK_FORMAT_UNDEFINED, vkvg_surface_get_vk_format(NULL));

    VkvgSurface surf = vkvg_surface_create(dev, 512, 256);
    EXPECT_EQ(512, vkvg_surface_get_width(surf));
    EXPECT_EQ(256, vkvg_surface_get_height(surf));
    EXPECT_EQ(VK_FORMAT_B8G8R8A8_UNORM, vkvg_surface_get_vk_format(surf));
    EXPECT_NE(nullptr, vkvg_surface_get_vk_image(surf));

    vkvg_surface_destroy(surf);
}
TEST_F(SurfaceTest, SurfWrite) {
    EXPECT_EQ(VKVG_STATUS_INVALID_STATUS, vkvg_surface_write_to_memory(NULL, NULL));
    EXPECT_EQ(VKVG_STATUS_INVALID_STATUS, vkvg_surface_write_to_png(NULL, NULL));
    const uint32_t imgSize = 8;
    VkvgSurface surf = vkvg_surface_create(dev, imgSize, imgSize);
    EXPECT_EQ(VKVG_STATUS_WRITE_ERROR, vkvg_surface_write_to_memory(surf, NULL));
    EXPECT_EQ(VKVG_STATUS_WRITE_ERROR, vkvg_surface_write_to_png(surf, NULL));
    uint32_t buff[imgSize*imgSize];
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_write_to_memory(surf, (unsigned char*)buff));

    vkvg_surface_destroy(surf);
}

TEST_F(SurfaceTest, SurfClear) {
    const uint32_t imgSize = 8;
    VkvgSurface surf = vkvg_surface_create(dev, imgSize, imgSize);
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);

    vkvg_surface_clear(surf);

    checkPixels(surf, 0x00000000);
    vkvg_surface_destroy(surf);
}

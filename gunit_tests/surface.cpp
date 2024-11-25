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
};

TEST_F(SurfaceTest, SurfCreate) {
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_surface_status(NULL));

    VkvgSurface surf = vkvg_surface_create(NULL, 0, 0);

    EXPECT_EQ(VKVG_STATUS_DEVICE_ERROR, vkvg_surface_status(surf));

    surf = vkvg_surface_create(dev, 512, 512);

    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));

    vkvg_surface_destroy(surf);
}

TEST_F(SurfaceTest, SurfReference) {
    VkvgSurface surf = vkvg_surface_create(NULL, 0, 0);
    EXPECT_EQ(0, vkvg_surface_get_reference_count(surf));
    vkvg_surface_reference(surf);

    EXPECT_EQ(VKVG_STATUS_DEVICE_ERROR, vkvg_surface_status(surf));

    surf = vkvg_surface_create(dev, 512, 512);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));
    EXPECT_EQ(2, vkvg_device_get_reference_count(dev));
    EXPECT_EQ(1, vkvg_surface_get_reference_count(surf));

    vkvg_surface_reference(surf);

    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));
    EXPECT_EQ(2, vkvg_surface_get_reference_count(surf));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));

    vkvg_surface_destroy(surf);

    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));
    EXPECT_EQ(1, vkvg_surface_get_reference_count(surf));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));

    vkvg_surface_destroy(surf);
    EXPECT_EQ(0, vkvg_surface_get_reference_count(surf));
}

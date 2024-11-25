#include "vkvg.h"
#include <gtest/gtest.h>

// The fixture for testing class Foo.
class ContextTest : public testing::Test {
  public:
    VkvgDevice  dev;
    VkvgSurface surf;

  protected:
    // You can remove any or all of the following functions if their bodies would
    // be empty.

    ContextTest() {
        vkvg_device_create_info_t info{};
        dev  = vkvg_device_create(&info);
        surf = vkvg_surface_create(dev, 512, 512);
    }

    ~ContextTest() override {
        vkvg_surface_destroy(surf);
        vkvg_device_destroy(dev);
    }

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(ContextTest, CtxCreate) {
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_status(NULL));

    VkvgContext ctx = vkvg_create(NULL);
    EXPECT_EQ(VKVG_STATUS_INVALID_SURFACE, vkvg_status(ctx));
    vkvg_destroy(ctx);

    ctx = vkvg_create(surf);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_status(ctx));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));

    vkvg_destroy(ctx);
}

TEST_F(ContextTest, CtxReference) {
    EXPECT_EQ(0, vkvg_get_reference_count(NULL));

    VkvgContext ctx = vkvg_create(NULL);
    EXPECT_EQ(0, vkvg_get_reference_count(ctx));

    ctx = vkvg_create(surf);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_status(ctx));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));

    EXPECT_EQ(1, vkvg_get_reference_count(ctx));
    EXPECT_EQ(2, vkvg_surface_get_reference_count(surf));
    EXPECT_EQ(2, vkvg_device_get_reference_count(dev));

    vkvg_reference(ctx);
    EXPECT_EQ(2, vkvg_get_reference_count(ctx));

    vkvg_destroy(ctx);
    EXPECT_EQ(1, vkvg_get_reference_count(ctx));

    vkvg_destroy(ctx);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));
    EXPECT_EQ(0, vkvg_get_reference_count(ctx));
    EXPECT_EQ(1, vkvg_surface_get_reference_count(surf));
    EXPECT_EQ(2, vkvg_device_get_reference_count(dev));
}

#include "vkvg.h"
#include <gtest/gtest.h>

TEST(Device, CreateNullInfo) {
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_device_status(NULL));

    VkvgDevice dev = vkvg_device_create(NULL);
    EXPECT_EQ(VKVG_STATUS_INVALID_DEVICE_CREATE_INFO, vkvg_device_status(dev));
    vkvg_device_destroy(dev);
}
TEST(Device, CreateEmpty) {
    vkvg_device_create_info_t info{};
    VkvgDevice                dev = vkvg_device_create(&info);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    vkvg_device_destroy(dev);
}

TEST(Device, Reference) {
    VkvgDevice dev = vkvg_device_create(NULL);
    EXPECT_EQ(0, vkvg_device_get_reference_count(dev));
    vkvg_device_reference(dev);

    vkvg_device_create_info_t info{};
    dev = vkvg_device_create(&info);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    EXPECT_EQ(1, vkvg_device_get_reference_count(dev));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));

    vkvg_device_reference(dev);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    EXPECT_EQ(2, vkvg_device_get_reference_count(dev));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));

    vkvg_device_destroy(dev);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    EXPECT_EQ(1, vkvg_device_get_reference_count(dev));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));

    vkvg_device_destroy(dev);
    //EXPECT_EQ(0, vkvg_device_get_reference_count(dev));
}

/*TEST(Device, GetVulkanRequirements)
{

    vkvg_public void vkvg_get_required_instance_extensions(const char **pExtensions, uint32_t *pExtCount);
    vkvg_public vkvg_status_t vkvg_get_required_device_extensions(VkPhysicalDevice phy, const char **pExtensions,
    vkvg_public const void *vkvg_get_device_requirements(VkPhysicalDeviceFeatures *pEnabledFeatures);
}*/

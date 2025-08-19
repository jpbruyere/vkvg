#include "VkvgTest.hpp"
TEST(surface_create_destroy_multi_512) {
    VkvgSurface* surfs = (VkvgSurface*)malloc(sizeof(VkvgSurface) * test->app->testSize);
    for (uint32_t i = 0; i < test->app->testSize; i++)
        surfs[i] = vkvg_surface_create(test->device, 512, 512);
    for (uint32_t i = 0; i < test->app->testSize; i++)
        vkvg_surface_destroy(surfs[i]);
    free(surfs);
}

TEST(surface_create_destroy_single_512) {
    VkvgSurface s = vkvg_surface_create(test->device, 512, 512);
    vkvg_surface_destroy(s);
}


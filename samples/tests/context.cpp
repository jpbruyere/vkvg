#include "VkvgTest.hpp"

TEST(context_create_destroy_multi) {
    VkvgContext* ctxs = (VkvgContext*)malloc(sizeof(VkvgContext) * test->app->testSize);
    for (uint32_t i = 0; i < test->app->testSize; i++)
        ctxs[i] = vkvg_create(test->surf);
    for (uint32_t i = 0; i < test->app->testSize; i++)
        vkvg_destroy(ctxs[i]);
    free(ctxs);
}

TEST(context_create_destroy_single) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_destroy(ctx);
}

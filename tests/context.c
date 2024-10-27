#include "test.h"

void create_destroy_multi() {
    VkvgContext *ctxs = (VkvgContext *)malloc(sizeof(VkvgContext) * test_size);
    for (uint32_t i = 0; i < test_size; i++)
        ctxs[i] = vkvg_create(surf);
    for (uint32_t i = 0; i < test_size; i++)
        vkvg_destroy(ctxs[i]);
    free(ctxs);
}

void create_destroy_single() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
    PERFORM_TEST(create_destroy_multi, argc, argv);
    no_test_size = true;
    PERFORM_TEST(create_destroy_single, argc, argv);
    return 0;
}

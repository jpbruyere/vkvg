#include "VkvgTest.hpp"
TEST(compositing) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    vkvg_set_source_rgba(ctx, 1, 0, 0, 0.5f);
    vkvg_rectangle(ctx, 100, 100, 200, 200);
    vkvg_fill(ctx);

    vkvg_set_source_rgba(ctx, 0, 0, 1, 0.5f);
    vkvg_rectangle(ctx, 200, 200, 200, 200);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}
TEST(opacity) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_clear(ctx);

    vkvg_set_source_rgba(ctx, 1, 0, 0, 1.0f);
    vkvg_rectangle(ctx, 100, 100, 200, 200);
    vkvg_fill(ctx);

    vkvg_set_opacity(ctx, 0.5f);

    vkvg_set_source_rgba(ctx, 0, 0, 1, 1.0f);
    vkvg_rectangle(ctx, 200, 200, 200, 200);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}

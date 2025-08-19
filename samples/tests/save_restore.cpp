#include "VkvgTest.hpp"

void recurse_draw(VkvgContext ctx, int depth) {
    depth++;
    vkvg_save(ctx);

    vkvg_translate(ctx, 5, 5);
    vkvg_rectangle(ctx, (float)depth, (float)depth, 200, 200);
    /*vkvg_clip_preserve(ctx);
    vkvg_set_source_rgb(ctx, 1.f/depth, 1.f / depth, 1.f / depth);
    vkvg_fill_preserve(ctx);*/
    vkvg_set_source_rgb(ctx, 0, 1, 0);
    vkvg_stroke(ctx);

    if (depth < 20)
        recurse_draw(ctx, depth);

    vkvg_restore(ctx);
}

TEST(save_restrore) {
    VkvgContext ctx = vkvg_create(test->surf);
    recurse_draw(ctx, 0);
    vkvg_destroy(ctx);
}






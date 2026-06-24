#include "VkvgTest.hpp"

VkvgSurface imgSurf;

void recurse_draw(VkvgContext ctx, int depth) {
    depth++;
    vkvg_save(ctx);

    vkvg_translate(ctx, 5, 5);
    vkvg_rectangle(ctx, (float)depth, (float)depth, 200, 200);
    vkvg_set_source_rgb(ctx, 0, 1, 0);
    vkvg_stroke(ctx);

    if (depth < 20)
        recurse_draw(ctx, depth);

    vkvg_restore(ctx);
}
void recurse_draw_clipped(VkvgContext ctx, int depth, int maxDepth) {
    static float gap = 15.f;
    depth++;
    vkvg_save(ctx);

    vkvg_rectangle(ctx, gap * depth, gap * depth, 512.f - gap * 2.f * depth, 512.f - gap * 2.f * depth);
    vkvg_clip(ctx);
    vkvg_set_source_rgb(ctx, 0.3f/depth, 1.f / depth, 1.f);
    vkvg_paint(ctx);
    vkvg_arc(ctx, 256, 256, 256.f - gap * depth, 0, 2.f*M_PI);
    vkvg_clip(ctx);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);

    if (depth < maxDepth)
        recurse_draw_clipped(ctx, depth, maxDepth);

    vkvg_restore(ctx);
    depth--;
}

TEST(save_restrore) {
    VkvgContext ctx = vkvg_create(test->surf);
    recurse_draw(ctx, 0);
    vkvg_destroy(ctx);
}
TEST(save_restrore_clipped) {
    VkvgContext ctx = vkvg_create(test->surf);
    imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("img512.png"));
    recurse_draw_clipped(ctx, 0, 14);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}






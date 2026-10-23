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
    static float gap = 10.f;
    depth++;
    float depthRatio = (float)depth / maxDepth;

    vkvg_rectangle(ctx, gap * depth, gap * depth, 512.f - gap * 2.f * depth, 512.f - gap * 2.f * depth);
    vkvg_clip(ctx);
    vkvg_set_source_rgb(ctx, depthRatio, depthRatio, 1.f);
    vkvg_paint(ctx);
    vkvg_arc(ctx, 256, 256, 256.f - gap * depth, 0, 2.f*M_PI);
    vkvg_clip(ctx);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);

    vkvg_save(ctx);

    if (depth < maxDepth)
        recurse_draw_clipped(ctx, depth, maxDepth);
    float a = M_PIF * 0.5f * depthRatio;
    float x = cos(a) * 256;
    float y = sin(a) * 256;

    vkvg_set_line_width(ctx, 2.f);
    vkvg_move_to(ctx, 256.f - x, 256.f - y);
    vkvg_line_to(ctx, 256.f + x, 256.f + y);
    vkvg_set_source_rgba(ctx, 0.f, 1.f, 0.f, 0.6f);
    vkvg_stroke(ctx);

    vkvg_restore(ctx);

    depth--;
}

TEST(save_restrore) {
    VkvgContext ctx = vkvg_create(test->surf);
    recurse_draw(ctx, 0);
    vkvg_destroy(ctx);
}
TEST(save_restrore_clipped3) {
    VkvgContext ctx = vkvg_create(test->surf);
    imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("img512.png"));
    recurse_draw_clipped(ctx, 0, 3);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(save_restrore_clipped5) {
    VkvgContext ctx = vkvg_create(test->surf);
    imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("img512.png"));
    recurse_draw_clipped(ctx, 0, 5);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(save_restrore_clipped20) {
    VkvgContext ctx = vkvg_create(test->surf);
    imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("img512.png"));
    recurse_draw_clipped(ctx, 0, 20);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(save_restrore_nz) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);

    vkvg_rectangle(ctx, 50, 50, 200, 200);
    vkvg_clip(ctx);

    vkvg_set_source_rgb(ctx, 0, 0, 1);
    vkvg_paint(ctx);

    vkvg_rectangle(ctx, 0, 0, 220, 220);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}
TEST(save_restrore_nz2) {
    VkvgContext ctx = vkvg_create(test->surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);

    vkvg_rectangle(ctx, 50, 50, 200, 200);
    vkvg_clip(ctx);

    vkvg_set_source_rgb(ctx, 0, 0, 1);
    vkvg_paint(ctx);

    vkvg_save(ctx);

        vkvg_rectangle(ctx, 75, 75, 150, 150);
        vkvg_clip(ctx);

        vkvg_set_source_rgb(ctx, 0, 1, 0);
        vkvg_paint(ctx);

        vkvg_save(ctx);

            vkvg_rectangle(ctx, 100, 100, 100, 100);
            vkvg_clip(ctx);

            vkvg_set_source_rgb(ctx, 1, 0, 0);
            vkvg_paint(ctx);

            vkvg_set_line_width(ctx, 40);
            vkvg_set_source_rgb(ctx, 0, 0, 1);
            vkvg_move_to(ctx, 0,0);
            vkvg_line_to(ctx, 512,512);
            vkvg_stroke_preserve(ctx);

        vkvg_restore(ctx);

        vkvg_set_line_width(ctx, 30);
        vkvg_set_source_rgb(ctx, 0, 0, 0);
        vkvg_stroke_preserve(ctx);

    vkvg_restore(ctx);

    vkvg_set_line_width(ctx, 20);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_stroke_preserve(ctx);

    /*vkvg_restore(ctx);

    vkvg_set_line_width(ctx, 10);
    vkvg_set_source_rgb(ctx, 1, 1, 0);
    vkvg_stroke(ctx);*/

    vkvg_destroy(ctx);
}


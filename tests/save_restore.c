#include "test.h"

const char* imgPath  = TESTS_DATA_ROOT "img512.png";
VkvgSurface imgSurf;

void recurse_draw(VkvgContext ctx, int depth, int maxDepth) {
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
        recurse_draw(ctx, depth, maxDepth);

    vkvg_restore(ctx);
}

void test() {
    VkvgContext ctx = vkvg_create(surf);
    imgSurf = vkvg_surface_create_from_image(device, imgPath);

    recurse_draw(ctx, 0, 13);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}

int main(int argc, char* argv[]) {
    no_test_size = true;
    PERFORM_TEST(test, argc, argv);
    return 0;
}

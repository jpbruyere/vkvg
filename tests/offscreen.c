#include "vkvg.h"

int main(int argc, char *argv[]) {
    vkvg_device_create_info_t info = {VK_SAMPLE_COUNT_1_BIT, false};
    VkvgDevice                dev  = vkvg_device_create(&info);
    VkvgSurface               surf = vkvg_surface_create(dev, 512, 512);
    VkvgContext               ctx  = vkvg_create(surf);

    vkvg_clear(ctx);
    vkvg_rectangle(ctx, 10, 10, 250, 200);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);

    vkvg_surface_write_to_png(surf, "offscreen.png");
    vkvg_surface_destroy(surf);

    vkvg_device_destroy(dev);
    return 0;
}

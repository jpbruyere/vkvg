#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);

    VkvgSurface imgSurf;// = vkvg_surface_create_from_image(device, "/mnt/data/images/blason.png");
    //VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "/mnt/data/images/2000px-Tux.svg.png");
    //VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "/mnt/data/images/path2674.png");
    //VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "/mnt/data/images/horse-black-head-shape-of-a-chess-piece_318-52446.jpg");
    /*vkvg_set_source_surface(ctx, imgSurf, 200, 200);
    vkvg_paint(ctx);
    vkvg_set_source_surface(ctx, imgSurf, 400, 400);
    vkvg_paint(ctx);
    vkvg_flush(ctx);
    vkvg_surface_destroy(imgSurf);*/

    imgSurf = vkvg_surface_create_from_image(device, "data/miroir.jpg");
    fflush(stdout);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);
    //vkvg_flush(ctx);
    //vkvg_set_source_rgba(ctx,1,0,0,1);

    vkvg_surface_destroy(imgSurf);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

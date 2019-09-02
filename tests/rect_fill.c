#include "test.h"

void test(){
    vkvg_matrix_t mat;
    vkvg_matrix_init_identity(&mat);
    vkvg_matrix_scale(&mat,zoom,zoom);
    vkvg_matrix_translate(&mat, panX, panY);

    vkvg_surface_clear(surf);

    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,1.0f,1.0f,1.0f,1);
    vkvg_paint(ctx);

    vkvg_set_matrix(ctx,&mat);

    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
    vkvg_set_line_width(ctx,10);
    vkvg_set_source_rgba(ctx,0,0,1,1);
    vkvg_rectangle(ctx,100,100,200,200);
    /*vkvg_fill(ctx);
    vkvg_rectangle(ctx,200,200,200,200);
    vkvg_set_source_rgba(ctx,1,0,0,0.5);*/
    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

#include "test.h"

void test(){
    vkvg_surface_clear(surf);

    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);

    vkvg_set_source_rgba(ctx,0,0,1,0.5);
    vkvg_rectangle(ctx,100,100,200,200);
    vkvg_fill(ctx);

    vkvg_rectangle(ctx,200,200,200,200);
    vkvg_set_source_rgba(ctx,1,0,0,0.5);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}
void test_evenodd(){
    vkvg_surface_clear(surf);

    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);

    vkvg_set_source_rgba(ctx,0,0,1,0.5);
    vkvg_rectangle(ctx,100,100,200,200);
    vkvg_fill(ctx);

    vkvg_rectangle(ctx,200,200,200,200);
    vkvg_set_source_rgba(ctx,1,0,0,0.5);
    vkvg_fill(ctx);

    vkvg_destroy(ctx);
}
int main(int argc, char *argv[]) {

    PERFORM_TEST (test);
    PERFORM_TEST (test_evenodd);
    return 0;
}

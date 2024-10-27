#include "test.h"

void draw(VkvgContext ctx) {
    vkvg_set_source_rgba(ctx, 0, 0, 1, 0.5);
    vkvg_rectangle(ctx, 100, 100, 200, 200);
    vkvg_fill(ctx);

    vkvg_rectangle(ctx, 200, 200, 200, 200);
    vkvg_set_source_rgba(ctx, 1, 0, 0, 0.5);
    vkvg_fill(ctx);
}
void test() {

    VkvgContext ctx = vkvg_create(surf);
    vkvg_clear(ctx);

    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_NON_ZERO);

    draw(ctx);

    vkvg_destroy(ctx);
}
void test_evenodd() {

    VkvgContext ctx = vkvg_create(surf);
    vkvg_clear(ctx);

    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);

    draw(ctx);

    vkvg_destroy(ctx);
}
int main(int argc, char *argv[]) {
    no_test_size = true;
    PERFORM_TEST(test, argc, argv);
    PERFORM_TEST(test_evenodd, argc, argv);
    return 0;
}

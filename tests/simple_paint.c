#include "test.h"

void test(){
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test, 1024, 768);

    return 0;
}

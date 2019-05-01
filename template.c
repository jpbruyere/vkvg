#include "test.h"

void test(){
	VkvgContext ctx = vkvg_create(surf);

	vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

	perform_test (test, 600, 800);

	return 0;
}

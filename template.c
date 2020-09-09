/*
 * test template
 */

#include "test.h"

void test_name(){
	VkvgContext ctx = vkvg_create(surf);

	vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
	PERFORM_TEST (test_name, argc, argv);
	return 0;
}

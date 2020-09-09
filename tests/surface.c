#include "test.h"

void create_destroy_multi_512(){	
	VkvgSurface* surfs = (VkvgSurface*)malloc(sizeof(VkvgSurface)*test_size);
	for (uint32_t i = 0; i < test_size; i++)
		surfs[i] = vkvg_surface_create (device, 512, 512);
	for (uint32_t i = 0; i < test_size; i++)	
		vkvg_surface_destroy(surfs[i]);
	free(surf);
}

void create_destroy_single_512(){
	VkvgSurface s = vkvg_surface_create (device, 512, 512);
	vkvg_surface_destroy (s);
}

int main(int argc, char *argv[]) {
	PERFORM_TEST (create_destroy_multi_512, argc, argv);
	no_test_size = true;
	PERFORM_TEST (create_destroy_single_512, argc, argv);
	return 0;
}

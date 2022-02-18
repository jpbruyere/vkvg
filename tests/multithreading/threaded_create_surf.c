#include "test.h"
#include "tinycthread.h"

#define THREAD_COUNT 16


static int finishedThreadCount = 0;
static mtx_t* pmutex;

int create_surfs () {
	for (uint32_t i=0; i<test_size; i++) {
		VkvgSurface s = vkvg_surface_create(device, test_width, test_height);
		vkvg_surface_destroy (s);
	}

	mtx_lock(pmutex);
	finishedThreadCount++;
	mtx_unlock(pmutex);
	return 0;
}

void fixedSizeRects(){
	mtx_t mutex;
	pmutex = &mutex;

	vkvg_device_set_thread_aware (device, 1);

	thrd_t threads[THREAD_COUNT];

	finishedThreadCount = 0;
	mtx_init (pmutex, mtx_plain);
	for (uint32_t i=0; i<THREAD_COUNT; i++)
		thrd_create (&threads[i], create_surfs, NULL);

	const struct timespec ts = {1,0};
	while (finishedThreadCount < THREAD_COUNT)
		thrd_sleep(&ts, NULL);

	mtx_lock(pmutex);
	mtx_unlock(pmutex);
	mtx_destroy (pmutex);
	pmutex = NULL;

	vkvg_device_set_thread_aware (device, 0);
}

int main(int argc, char *argv[]) {
	PERFORM_TEST (fixedSizeRects, argc, argv);
	return 0;
}

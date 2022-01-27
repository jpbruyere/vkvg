#include "test.h"
#include "tinycthread.h"

#define THREAD_COUNT 16


static int finishedThreadCount = 0;
static mtx_t* pmutex;

void drawRandomRect (VkvgContext ctx, float s) {
	float w = (float)test_width;
	float h = (float)test_height;
	randomize_color(ctx);

	float x = truncf(w*rndf());
	float y = truncf(h*rndf());

	vkvg_rectangle(ctx, x, y, s, s);
}

int drawRectsThread () {

	VkvgSurface s = vkvg_surface_create(device, test_width, test_height);
	VkvgContext ctx = vkvg_create(s);
	for (uint32_t i=0; i<test_size; i++) {
		drawRandomRect(ctx, 14.0f);
		vkvg_fill (ctx);
	}
	vkvg_destroy(ctx);

	mtx_lock(pmutex);

	ctx = vkvg_create(surf);
		vkvg_set_source_surface (ctx, s, 0, 0);
		vkvg_paint(ctx);
	vkvg_destroy(ctx);

	finishedThreadCount++;
	mtx_unlock(pmutex);

	vkvg_surface_destroy (s);
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
		thrd_create (&threads[i], drawRectsThread, NULL);

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

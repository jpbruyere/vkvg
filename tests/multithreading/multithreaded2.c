#include "test.h"
#include "tinycthread.h"

#define THREAD_COUNT 32


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
void _before_submit (void* data) {
	mtx_lock((mtx_t*)data);
}
void _after_submit (void* data) {
	mtx_unlock((mtx_t*)data);
}

int drawRectsThread () {
	VkvgContext ctx = vkvg_create(surf);
	for (uint32_t i=0; i<test_size; i++) {
		drawRandomRect(ctx, 14.0f);
		vkvg_fill (ctx);
	}
	vkvg_destroy(ctx);
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

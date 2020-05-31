#include "test.h"
#include "string.h"

#if defined(_WIN32) || defined(_WIN64)
int gettimeofday(struct timeval * tp, void * tzp)
{
	// FILETIME Jan 1 1970 00:00:00
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

	SYSTEMTIME  nSystemTime;
	FILETIME    nFileTime;
	uint64_t    nTime;

	GetSystemTime( &nSystemTime );
	SystemTimeToFileTime( &nSystemTime, &nFileTime );
	nTime =  ((uint64_t)nFileTime.dwLowDateTime )      ;
	nTime += ((uint64_t)nFileTime.dwHighDateTime) << 32;

	tp->tv_sec  = (long) ((nTime - EPOCH) / 10000000L);
	tp->tv_usec = (long) (nSystemTime.wMilliseconds * 1000);
	return 0;
}
#endif

float panX	= 0.f;
float panY	= 0.f;
float lastX = 0.f;
float lastY = 0.f;
float zoom	= 1.0f;
bool mouseDown = false;

VkvgDevice device	= NULL;
VkvgSurface surf	= NULL;

uint32_t test_size	= 100;	// items drawn in one run, or complexity
uint32_t iterations	= 40000;// repeat test n times
uint32_t test_width	= 1024;
uint32_t test_height= 768;
bool	 test_vsync = false;


static bool paused = false;
static VkSampleCountFlags samples = VK_SAMPLE_COUNT_8_BIT;
static vk_engine_t* e;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS)
		return;
	switch (key) {
	case GLFW_KEY_SPACE:
		 paused = !paused;
		break;
	case GLFW_KEY_ESCAPE :
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	}
}
static void char_callback (GLFWwindow* window, uint32_t c){}
static void mouse_move_callback(GLFWwindow* window, double x, double y){
	if (mouseDown) {
		panX += ((float)x-lastX);
		panY += ((float)y-lastY);
	}
	lastX = (float)x;
	lastY = (float)y;
}
static void scroll_callback(GLFWwindow* window, double x, double y){
	if (y<0.f)
		zoom *= 0.5f;
	else
		zoom *= 2.0f;
}
static void mouse_button_callback(GLFWwindow* window, int but, int state, int modif){
	if (but != GLFW_MOUSE_BUTTON_1)
		return;
	if (state == GLFW_TRUE)
		mouseDown = true;
	else
		mouseDown = false;
}

double time_diff(struct timeval x , struct timeval y)
{
	double x_ms , y_ms , diff;

	x_ms = (double)x.tv_sec*1000000 + (double)x.tv_usec;
	y_ms = (double)y.tv_sec*1000000 + (double)y.tv_usec;

	diff = (double)y_ms - (double)x_ms;

	return diff;
}

void randomize_color (VkvgContext ctx) {
	vkvg_set_source_rgba(ctx,
		(float)rand()/RAND_MAX,
		(float)rand()/RAND_MAX,
		(float)rand()/RAND_MAX,
		(float)rand()/RAND_MAX
	);
}
/* from caskbench */
double
get_tick (void)
{
	struct timeval now;
	gettimeofday (&now, NULL);
	return (double)now.tv_sec + (double)now.tv_usec / 1000000.0;
}
double median_run_time (double data[], int n)
{
	double temp;
	int i, j;
	for (i = 0; i < n; i++)
		for (j = i+1; j < n; j++)
		{
			if (data[i] > data[j])
			{
				temp = data[j];
				data[j] = data[i];
				data[i] = temp;
			}
		}
	if (n % 2 == 0)
		return (data[n/2] + data[n/2-1])/2;
	else
		return data[n/2];
}
double standard_deviation (const double data[], int n, double mean)
{
	double sum_deviation = 0.0;
	int i;
	for (i = 0; i < n; ++i)
	sum_deviation += (data[i]-mean) * (data[i]-mean);
	return sqrt (sum_deviation / n);
}
/***************/

void init_test (uint32_t width, uint32_t height){
	if (test_vsync)
		e = vkengine_create (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_FIFO_KHR, width, height);
	else
		e = vkengine_create (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_MAILBOX_KHR, width, height);

	VkhPresenter r = e->renderer;
	vkengine_set_key_callback (e, key_callback);
	vkengine_set_mouse_but_callback(e, mouse_button_callback);
	vkengine_set_cursor_pos_callback(e, mouse_move_callback);
	vkengine_set_scroll_callback(e, scroll_callback);

	bool deferredResolve = false;

	device = vkvg_device_create_multisample(vkh_app_get_inst(e->app), r->dev->phy, r->dev->dev, r->qFam, 0, samples, deferredResolve);

	vkvg_device_set_dpy(device, 96, 96);

	surf = vkvg_surface_create(device, width, height);

	vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), width, height);
}

//	printf ("size:%d iter:%d  avgFps: %f avg: %4.2f%% med: %4.2f%% sd: %4.2f%% \n", test_size, i, avg_frames_per_second, avg_run_time, med_run_time, standard_dev);

void clear_test () {
	vkDeviceWaitIdle(e->dev->dev);

	vkvg_surface_destroy    (surf);
	vkvg_device_destroy     (device);

	vkengine_destroy (e);
}

#ifdef VKVG_TEST_DIRECT_DRAW
VkvgSurface* surfaces;
#endif

void perform_test (void(*testfunc)(void), const char *testName, int argc, char* argv[]) {
	//init random gen
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	srand((unsigned) currentTime.tv_usec);

	//dumpLayerExts();
	if (argc > 1)
		iterations = atoi (argv[1]);
	if (argc > 2)
		test_size = atoi (argv[2]);
	if (iterations == 0 || test_size == 0) {
		printf("usage: test [iterations] [size]\n");
		return;
	}

	char* whoami;
	(whoami = strrchr(argv[0], '/')) ? ++whoami : (whoami = argv[0]);

	if (test_vsync)
		e = vkengine_create (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_FIFO_KHR, test_width, test_height);
	else
		e = vkengine_create (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_MAILBOX_KHR, test_width, test_height);

	VkhPresenter r = e->renderer;
	vkengine_set_key_callback (e, key_callback);
	vkengine_set_mouse_but_callback(e, mouse_button_callback);
	vkengine_set_cursor_pos_callback(e, mouse_move_callback);
	vkengine_set_scroll_callback(e, scroll_callback);

	bool deferredResolve = false;

	device  = vkvg_device_create_multisample(vkh_app_get_inst(e->app), r->dev->phy, r->dev->dev, r->qFam, 0, samples, deferredResolve);

	vkvg_device_set_dpy(device, 96, 96);

#ifdef VKVG_TEST_DIRECT_DRAW
	surfaces = (VkvgSurface*)malloc(r->imgCount * sizeof (VkvgSurface));
	for (uint32_t i=0; i < r->imgCount;i++)
		surfaces[i] = vkvg_surface_create_for_VkhImage (device, r->ScBuffers[i]);
#else
	surf = vkvg_surface_create(device, test_width, test_height);
	vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), test_width, test_height);
#endif


	double start_time = 0.0, stop_time = 0.0, run_time = 0.0, run_total = 0.0, min_run_time = -1, max_run_time = 0.0;
	double* run_time_values = (double*)malloc(iterations*sizeof(double));

	uint32_t i = 0;

	vkengine_set_title(e, testName);

	while (!vkengine_should_close (e) && i < iterations) {
		glfwPollEvents();

		start_time = get_tick();

#ifdef VKVG_TEST_DIRECT_DRAW

		if (!vkh_presenter_acquireNextImage(r, NULL, NULL)) {
			for (uint32_t i=0; i < r->imgCount;i++)
				vkvg_surface_destroy (surfaces[i]);

			vkh_presenter_create_swapchain (r);

			for (uint32_t i=0; i < r->imgCount;i++)
				surfaces[i] = vkvg_surface_create_for_VkhImage (device, r->ScBuffers[i]);
		}else{
			surf = surfaces[r->currentScBufferIndex];

			testfunc();

			if (deferredResolve)
				vkvg_multisample_surface_resolve(surf);

			VkPresentInfoKHR present = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
										 .swapchainCount = 1,
										 .pSwapchains = &r->swapChain,
										 .pImageIndices = &r->currentScBufferIndex };

			vkQueuePresentKHR(r->queue, &present);
		}
#else
		if (!paused)
			testfunc();

		if (deferredResolve)
			vkvg_multisample_surface_resolve(surf);
		if (!vkh_presenter_draw (r)){
			vkh_presenter_get_size (r, &test_width, &test_height);
			vkvg_surface_destroy (surf);
			surf = vkvg_surface_create(device, test_width, test_height);
			vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), test_width, test_height);
			vkDeviceWaitIdle(r->dev->dev);
			continue;
		}
#endif

		if (paused)
			continue;

		stop_time = get_tick();
		run_time = stop_time - start_time;
		run_time_values[i] = run_time;

		if (min_run_time < 0)
			min_run_time = run_time;
		else
			min_run_time = MIN(run_time, min_run_time);
		max_run_time = MAX(run_time, max_run_time);
		run_total += run_time;
		i++;
	}

	double avg_run_time = run_total / (double)i;
	double med_run_time = median_run_time (run_time_values, i);
	double standard_dev = standard_deviation (run_time_values, i, avg_run_time);
	double avg_frames_per_second = (1.0 / avg_run_time);
	avg_frames_per_second = (avg_frames_per_second<9999) ? avg_frames_per_second:9999;

	free (run_time_values);


	//printf ("size:%d iter:%d  avgFps: %f avg: %4.2f%% med: %4.2f%% sd: %4.2f%% \n", test_size, i, avg_frames_per_second, avg_run_time, med_run_time, standard_dev);
	printf ("| %-15s | %-25s | ",whoami + 5, testName);
	printf ("%4d | %4d | %7.2f | %6.5f | %6.5f | %6.5f |\n",
			test_size, i, avg_frames_per_second, avg_run_time, med_run_time, standard_dev);

	//printf ("%s size:%d iter:%d  avgFps: %f avg: %4.2f%% med: %4.2f%% sd: %4.2f%% \n", whoami+5, test_size, i, avg_frames_per_second, avg_run_time, med_run_time, standard_dev);

	vkDeviceWaitIdle(e->dev->dev);

#ifdef VKVG_TEST_DIRECT_DRAW
	for (uint32_t i=0; i<r->imgCount;i++)
		vkvg_surface_destroy (surfaces[i]);

	free (surfaces);
#else
	vkvg_surface_destroy    (surf);
#endif

	vkvg_device_destroy     (device);

	vkengine_destroy (e);
}
const int star_points[11][2] = {
	{ 0, 85 },
	{ 75, 75 },
	{ 100, 10 },
	{ 125, 75 },
	{ 200, 85 },
	{ 150, 125 },
	{ 160, 190 },
	{ 100, 150 },
	{ 40, 190 },
	{ 50, 125 },
	{ 0, 85 }
};

void draw_random_shape (VkvgContext ctx, shape_t shape, float sizeFact) {
	float w = (float)test_width;
	float h = (float)test_height;

	float x, y, z, v, r;

	randomize_color (ctx);

	switch (shape) {
	case SHAPE_LINE:
		x = (float)rand()/RAND_MAX * w;
		y = (float)rand()/RAND_MAX * h;
		z = (float)rand()/RAND_MAX * w;
		v = (float)rand()/RAND_MAX * h;

		vkvg_move_to(ctx, x, y);
		vkvg_line_to(ctx, z, v);
		vkvg_stroke(ctx);
		break;
	case SHAPE_RECTANGLE:
		z = truncf((sizeFact*w*rand()/RAND_MAX)+1.f);
		v = truncf((sizeFact*h*rand()/RAND_MAX)+1.f);
		x = truncf((w-z)*rand()/RAND_MAX);
		y = truncf((h-v)*rand()/RAND_MAX);

		vkvg_rectangle(ctx, x+1, y+1, z, v);
		break;
	case SHAPE_ROUNDED_RECTANGLE:
		z = truncf((sizeFact*w*rand()/RAND_MAX)+1.f);
		v = truncf((sizeFact*h*rand()/RAND_MAX)+1.f);
		x = truncf((w-z)*rand()/RAND_MAX);
		y = truncf((h-v)*rand()/RAND_MAX);
		r = truncf((0.2f*z*rand()/RAND_MAX)+1.f);

		if ((r > v / 2) || (r > z / 2))
			r = MIN(v / 2, z / 2);

		vkvg_move_to(ctx, x, y + r);
		vkvg_arc(ctx, x + r, y + r, r, (float)M_PI, (float)-M_PI_2);
		vkvg_line_to(ctx, x + z - r, y);
		vkvg_arc(ctx, x + z - r, y + r, r, (float)-M_PI_2, 0);
		vkvg_line_to(ctx, x + z, y + v - r);
		vkvg_arc(ctx, x + z - r, y + v - r, r, 0, (float)M_PI_2);
		vkvg_line_to(ctx, x + r, y + v);
		vkvg_arc(ctx, x + r, y + v - r, r, (float)M_PI_2, (float)M_PI);
		vkvg_line_to(ctx, x, y + r);
		vkvg_close_path(ctx);
		break;
	case SHAPE_CIRCLE:
		/*x = truncf((float)w * rnd()/RAND_MAX);
		y = truncf((float)h * rnd()/RAND_MAX);
		v = truncf((float)w * rnd()/RAND_MAX * 0.2f);*/
		x = (float)rand()/RAND_MAX * w;
		y = (float)rand()/RAND_MAX * h;

		r = truncf((sizeFact*MIN(w,h)*rand()/RAND_MAX)+1.f);

		/*float r = 0.5f*w*rand()/RAND_MAX;
		float x = truncf(0.5f * w*rand()/RAND_MAX + r);
		float y = truncf(0.5f * w*rand()/RAND_MAX + r);*/

		vkvg_arc(ctx, x, y, r, 0, (float)M_PI * 2.0f);
		break;
	case SHAPE_TRIANGLE:
	case SHAPE_STAR:
		x = (float)rand()/RAND_MAX * w;
		y = (float)rand()/RAND_MAX * h;
		z = (float)rand()/RAND_MAX * sizeFact + 0.15; //scale

		vkvg_move_to (ctx, x+star_points[0][0]*z, y+star_points[0][1]*z);
		for (int s=1; s<11; s++)
			vkvg_line_to (ctx, x+star_points[s][0]*z, y+star_points[s][1]*z);
		vkvg_close_path (ctx);
		break;
	case SHAPE_RANDOM:
		draw_random_shape(ctx, 1 + rand()%4, sizeFact);
		break;
	}
}

/*void draw_random_shape (VkvgContext ctx, shape_t shape) {
	float w = (float)test_width;
	float h = (float)test_height;
	randomize_color(ctx);
	float z = truncf((0.5f*w*rand()/RAND_MAX)+1.f);
	float v = truncf((0.5f*w*rand()/RAND_MAX)+1.f);
	float x = truncf((w-z)*rand()/RAND_MAX);
	float y = truncf((h-v)*rand()/RAND_MAX);
	vkvg_rectangle(ctx, x, y, z, v);
}*/

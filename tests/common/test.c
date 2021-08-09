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

uint32_t test_size	= 500;	// items drawn in one run, or complexity
uint32_t iterations	= 500;// repeat test n times
uint32_t test_width	= 512;
uint32_t test_height= 512;
bool	test_vsync	= false;
bool 	quiet		= false;//if true, don't print details and head row
bool 	first_test	= true;//if multiple tests, dont print header row.
bool 	no_test_size= false;//several test consist of a single draw sequence without looping 'size' times
							  //those test must be preceded by setting no_test_size to 'true'
int 	test_index	= 0;
int		single_test = -1;	//if not < 0, contains the index of the single test to run


static bool paused = false;
static VkSampleCountFlags samples = VK_SAMPLE_COUNT_1_BIT;
static VkPhysicalDeviceType preferedPhysicalDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
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

/* from caskbench */
double get_tick (void)
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
		e = vkengine_create (preferedPhysicalDeviceType, VK_PRESENT_MODE_FIFO_KHR, width, height);
	else
		e = vkengine_create (preferedPhysicalDeviceType, VK_PRESENT_MODE_MAILBOX_KHR, width, height);

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
void clear_test () {
	vkDeviceWaitIdle(e->dev->dev);

	vkvg_surface_destroy    (surf);
	vkvg_device_destroy     (device);

	vkengine_destroy (e);
}

#ifdef VKVG_TEST_DIRECT_DRAW
VkvgSurface* surfaces;
#endif
void _print_usage_and_exit () {
	printf("\nUsage: test [options]\n\n");
	printf("\t-i iterations:\tSpecify the repeat count for the test.\n");
	printf("\t-s size:\tWhen applicable, specify the size of the test.\n");
	printf("\t-x width:\tOutput surface width.\n");
	printf("\t-y height:\tOutput surface height.\n");
	printf("\t-S num samples:\tOutput surface filter, default is 1.\n");
	printf("\t-G gpu_type:\tSet prefered GPU type:\n");
	printf("\t\t\t\t- 0: Other\n");
	printf("\t\t\t\t- 1: Integrated\n");
	printf("\t\t\t\t- 2: Discrete\n");
	printf("\t\t\t\t- 3: Virtual\n");
	printf("\t\t\t\t- 4: Cpu\n");
	printf("\t-n index:\tRun only a single test, zero based index.\n");
	printf("\t-q:\t\tQuiet, don't print measures table head row, usefull for batch tests\n");
	printf("\t-p:\t\tPrint test details and exit without performing test, usefull to print details in logs\n");
	printf("\t-vsync:\t\tEnable VSync, disabled by default\n");
	printf("\t-help:\t\tthis help message.\n");
	printf("\n");
	exit(-1);
}
void _parse_args (int argc, char* argv[]) {
	bool printTestDetailsAndExit = false;
	for (int i = 1; i < argc; i++) {
		if (strcmp (argv[i], "-h\0") == 0)
			_print_usage_and_exit ();
		if (strcmp (argv[i], "-p\0") == 0)
			printTestDetailsAndExit = true;
		else if (strcmp (argv[i], "-vsync\0") == 0)
			test_vsync = true;		
		else if (strcmp (argv[i], "-q\0") == 0)
			quiet = true;
		else if (strcmp (argv[i], "-i\0") == 0) {
			if (argc -1 < ++i)
				_print_usage_and_exit();
			iterations = atoi (argv[i]);
		}else if (strcmp (argv[i], "-x\0") == 0) {
			if (argc -1 < ++i)
				_print_usage_and_exit();
			test_width = atoi (argv[i]);
		}else if (strcmp (argv[i], "-y\0") == 0) {
			if (argc -1 < ++i)
				_print_usage_and_exit();
			test_height = atoi (argv[i]);
		}else if (strcmp (argv[i], "-n\0") == 0) {
			if (argc -1 < ++i)
				_print_usage_and_exit();
			single_test = atoi (argv[i]);
		}else if (strcmp (argv[i], "-s\0") == 0) {
			if (argc -1 < ++i)
				_print_usage_and_exit();
			test_size = atoi (argv[i]);			
		}else if (strcmp (argv[i], "-S\0") == 0) {
			if (argc -1 < ++i)
				_print_usage_and_exit();
			samples = (VkSampleCountFlags)atoi (argv[i]);
		}else if (strcmp (argv[i], "-g\0") == 0) {
			if (argc -1 < ++i)
				_print_usage_and_exit();
			preferedPhysicalDeviceType = (VkPhysicalDeviceType)atoi (argv[i]);
		}
		if (printTestDetailsAndExit) {
			#ifdef DEBUG
			printf("Debug build\n");
			#else
			printf("Release build\n");
			#endif
			#ifdef VKVG_USE_RENDERDOC
			printf("Render doc enabled\n");
			#endif
			#ifdef VKVG_USE_VALIDATION
			printf("Validation enabled\n");
			#endif
			printf("surf dims:\t%d x %d\n", test_width, test_height);
			printf("Samples:\t%d\n", samples);
			printf("Gpu type:\t");
			switch (preferedPhysicalDeviceType) {
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:
				printf("Other\n");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				printf("Integrated\n");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				printf("Discrete\n");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				printf("Virtual\n");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				printf("CPU\n");
				break;
			}

			#ifdef VKVG_TEST_OFFSCREEN
			printf("Offscreen:\ttrue\n");
			#else
			printf("Offscreen:\tfalse\n");
			#endif
			printf("\n");
			exit(0);
		}
	}
}

void _print_results (const char *testName, int argc, char* argv[], uint32_t i, double run_total, double* run_time_values) {
	char* whoami;
	(whoami = strrchr(argv[0], '/')) ? ++whoami : (whoami = argv[0]);

	double avg_run_time = run_total / (double)i;
	double med_run_time = median_run_time (run_time_values, i);
	double standard_dev = standard_deviation (run_time_values, i, avg_run_time);
	double avg_frames_per_second = (1.0 / avg_run_time);
	avg_frames_per_second = (avg_frames_per_second<9999) ? avg_frames_per_second:9999;

	if (!quiet && (test_index == 0 || test_index == single_test)) {
#if VKVG_DBG_STATS
		printf ("_____________________________________________________________________________________________________________________________\n");
		printf ("| N° | Test File Name  |       Sub Test            | Iter | Size |  Pts  |Pathes| Vx cache | Ix cache |   VBO    |   IBO    |\n");
		printf ("|----|-----------------|---------------------------|------|------|-------|------|----------|----------|----------|----------|\n");
#else
		printf ("__________________________________________________________________________________________________________\n");
		printf ("| N° | Test File Name  |       Sub Test            | Iter | Size |   FPS   | Average | Median  | Sigma   |\n");
		printf ("|----|-----------------|---------------------------|------|------|---------|---------|---------|---------|\n");
#endif
	}
	
	printf ("| %2d | %-15s | %-25s | %4d | ", test_index, whoami + 5, testName, i);
	if (no_test_size)
		printf ("%4d | ", 1);
	else
		printf ("%4d | ", test_size);

#if VKVG_DBG_STATS
	vkvg_debug_stats_t dbgStats = vkvg_device_get_stats (device);	
	printf ("%5d | %4d | %8d | %8d | %8d | %8d |\n",
			dbgStats.sizePoints, dbgStats.sizePathes, dbgStats.sizeVertices,
			dbgStats.sizeIndices, dbgStats.sizeVBO, dbgStats.sizeIBO);
#else	
	printf ("%7.2f | %6.5f | %6.5f | %6.5f |\n",
		avg_frames_per_second, avg_run_time, med_run_time, standard_dev);
#endif	
}

#if VKVG_DBG_STATS
void _print_debug_stats () {

	vkvg_debug_stats_t dbgStats = vkvg_device_get_stats (device);
	printf ("| %8d | %8d | %8d | %8d | %8d | %8d |\n", dbgStats.sizePoints, dbgStats.sizePathes, dbgStats.sizeVertices,
			dbgStats.sizeIndices, dbgStats.sizeVBO, dbgStats.sizeIBO);
}
#endif

void perform_test_offscreen (void(*testfunc)(void), const char *testName, int argc, char* argv[]) {
	//init random gen
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	srand((unsigned) currentTime.tv_usec);

	//dumpLayerExts();
	
	_parse_args (argc, argv);

	if (single_test >= 0 && test_index != single_test){
		test_index++;
		return;
	}
	
	uint32_t enabledExtsCount = 0, phyCount = 0;
	const char* enabledExts [10];
#ifdef VKVG_USE_RENDERDOC
	const uint32_t enabledLayersCount = 2;
	const char* enabledLayers[] = {"VK_LAYER_KHRONOS_validation", "VK_LAYER_RENDERDOC_Capture"};
#elif defined (VKVG_USE_VALIDATION)
	const uint32_t enabledLayersCount = 1;
	const char* enabledLayers[] = {"VK_LAYER_KHRONOS_validation"};
#else
	const uint32_t enabledLayersCount = 0;
	const char* enabledLayers[] = {NULL};
#endif
#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	enabledExts[enabledExtsCount] = "VK_EXT_debug_utils";
	enabledExtsCount++;
#endif

	VkhApp app = vkh_app_create("vkvgTest", enabledLayersCount, enabledLayers, enabledExtsCount, enabledExts);
#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_app_enable_debug_messenger(app
								   , VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
								   | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
								   | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
								   , VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
								   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
								   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
								   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
								   , NULL);
#endif
	bool deferredResolve = false;
	VkhPhyInfo* phys = vkh_app_get_phyinfos (app, &phyCount, VK_NULL_HANDLE);
	VkhPhyInfo pi = 0;
	for (uint32_t i=0; i<phyCount; i++){
		pi = phys[i];
		if (pi->properties.deviceType == preferedPhysicalDeviceType)
			break;
	}

	uint32_t qCount = 0;
	float qPriorities[] = {0.0};
	VkDeviceQueueCreateInfo pQueueInfos[] = { {0},{0},{0} };
	if (vkh_phyinfo_create_queues (pi, pi->gQueue, 1, qPriorities, &pQueueInfos[qCount]))
		qCount++;
	VkPhysicalDeviceFeatures enabledFeatures = {
		.fillModeNonSolid = true,
	};

	VkDeviceCreateInfo device_info = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
									   .queueCreateInfoCount = qCount,
									   .pQueueCreateInfos = (VkDeviceQueueCreateInfo*)&pQueueInfos,
									   .pEnabledFeatures = &enabledFeatures};

	VkhDevice dev = vkh_device_create(app, pi, &device_info);


	device  = vkvg_device_create_multisample(vkh_app_get_inst(app), dev->phy, dev->dev, pi->gQueue, 0, samples, deferredResolve);
	vkvg_device_set_dpy(device, 96, 96);

	vkh_app_free_phyinfos (phyCount, phys);

	surf = vkvg_surface_create(device, test_width, test_height);

	double start_time = 0.0, stop_time = 0.0, run_time = 0.0, run_total = 0.0, min_run_time = -1, max_run_time = 0.0;
	double* run_time_values = (double*)malloc(iterations*sizeof(double));

	uint32_t i = 0;
	while (i < iterations) {
		start_time = get_tick();

		testfunc();

		if (deferredResolve)
			vkvg_multisample_surface_resolve(surf);

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

	_print_results (testName, argc, argv, i, run_total, run_time_values);

	free (run_time_values);	

	vkDeviceWaitIdle(dev->dev);

	vkvg_surface_destroy    (surf);
	vkvg_device_destroy     (device);

	vkh_device_destroy (dev);
	vkh_app_destroy (app);

	test_index++;
}

void perform_test (void(*testfunc)(void), const char *testName, int argc, char* argv[]) {
	//init random gen
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	srand((unsigned) currentTime.tv_usec);

	//dumpLayerExts();
	_parse_args (argc, argv);

	if (single_test >= 0 && test_index != single_test){
		test_index++;
		return;
	}

	if (test_vsync)
		e = vkengine_create (preferedPhysicalDeviceType, VK_PRESENT_MODE_FIFO_KHR, test_width, test_height);
	else
		e = vkengine_create (preferedPhysicalDeviceType, VK_PRESENT_MODE_MAILBOX_KHR, test_width, test_height);

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

	_print_results (testName, argc, argv, i, run_total, run_time_values);

	free (run_time_values);

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
	
	test_index++;
}


/* common context init for several tests */
vkvg_fill_rule_t	fill_rule	= VKVG_FILL_RULE_NON_ZERO;
vkvg_line_cap_t		line_cap	= VKVG_LINE_CAP_BUTT;
vkvg_line_join_t	line_join	= VKVG_LINE_JOIN_MITER;
float		dashes[]	= {20.0f, 10.0f};
uint32_t	dashes_count= 0;
float		dash_offset	= 0;
float		line_width	= 10.f;

VkvgContext _initCtx() {
	VkvgContext ctx = vkvg_create(surf);

	vkvg_set_line_width	(ctx,line_width);
	vkvg_set_line_join	(ctx, line_join);
	vkvg_set_line_cap	(ctx, line_cap);
	vkvg_set_dash		(ctx, dashes, dashes_count, dash_offset);
	vkvg_set_fill_rule	(ctx, fill_rule);

	vkvg_clear			(ctx);
	return ctx;
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
void randomize_color(VkvgContext ctx) {
	vkvg_set_source_rgba(ctx,
		rndf(),
		rndf(),
		rndf(),
		rndf()
	);
}
void draw_random_shape (VkvgContext ctx, shape_t shape, double sizeFact) {
	double w = (double)test_width;
	double h = (double)test_height;

	double x, y, z, v, r;

	randomize_color (ctx);

	switch (shape) {
	case SHAPE_LINE:
		x = rndf() * w;
		y = rndf() * h;
		z = rndf() * w;
		v = rndf() * h;

		vkvg_move_to(ctx, x, y);
		vkvg_line_to(ctx, z, v);
		vkvg_stroke(ctx);
		break;
	case SHAPE_RECTANGLE:
		z = truncf((sizeFact*w*rndf())+1.f);
		v = truncf((sizeFact*h*rndf())+1.f);
		x = truncf((w-z)*rndf());
		y = truncf((h-v)*rndf());

		vkvg_rectangle(ctx, x+1, y+1, z, v);
		break;
	case SHAPE_ROUNDED_RECTANGLE:
		z = truncf((sizeFact*w*rndf())+1.f);
		v = truncf((sizeFact*h*rndf())+1.f);
		x = truncf((w-z)*rndf());
		y = truncf((h-v)*rndf());
		r = truncf((0.2f*z*rndf())+1.f);

		if ((r > v / 2) || (r > z / 2))
			r = MIN(v / 2, z / 2);

		vkvg_move_to(ctx, x, y + r);
		vkvg_arc(ctx, x + r, y + r, r, M_PI, -M_PI_2);
		vkvg_line_to(ctx, x + z - r, y);
		vkvg_arc(ctx, x + z - r, y + r, r, -M_PI_2, 0);
		vkvg_line_to(ctx, x + z, y + v - r);
		vkvg_arc(ctx, x + z - r, y + v - r, r, 0, M_PI_2);
		vkvg_line_to(ctx, x + r, y + v);
		vkvg_arc(ctx, x + r, y + v - r, r, M_PI_2, M_PI);
		vkvg_line_to(ctx, x, y + r);
		vkvg_close_path(ctx);
		break;
	case SHAPE_CIRCLE:
		/*x = truncf((float)w * rnd()/RAND_MAX);
		y = truncf((float)h * rnd()/RAND_MAX);
		v = truncf((float)w * rnd()/RAND_MAX * 0.2f);*/
		x = rndf() * w;
		y = rndf() * h;

		r = trunc((sizeFact*MIN(w,h)*rndf())+1.0);

		/*float r = 0.5f*w*rand()/RAND_MAX;
		float x = truncf(0.5f * w*rand()/RAND_MAX + r);
		float y = truncf(0.5f * w*rand()/RAND_MAX + r);*/

		vkvg_arc(ctx, x, y, r, 0, M_PI * 2.0);
		break;
	case SHAPE_TRIANGLE:
	case SHAPE_STAR:
		x = rndf() * w;
		y = rndf() * h;
		z = rndf() * sizeFact + 0.15; //scale

		vkvg_move_to (ctx, x+star_points[0][0]*z, y+star_points[0][1]*z);
		for (int s=1; s<11; s++)
			vkvg_line_to (ctx, x+star_points[s][0]*z, y+star_points[s][1]*z);
		vkvg_close_path (ctx);
		break;
	case SHAPE_RANDOM:
		draw_random_shape(ctx, 1 + (rndf() * 4), sizeFact);
		break;
	}
}
void draw_random_curve (VkvgContext ctx) {
	double w = (double)test_width;
	double h = (double)test_height;

	double x2 = w*rndf();
	double y2 = h*rndf();
	double cp_x1 = w*rndf();
	double cp_y1 = h*rndf();
	double cp_x2 = w*rndf();
	double cp_y2 = h*rndf();

	vkvg_curve_to(ctx, cp_x1, cp_y1, cp_x2, cp_y2, x2, y2);
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

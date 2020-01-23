#include "test.h"

float panX = 0.f;
float panY = 0.f;
float lastX = 0.f;
float lastY = 0.f;
float zoom = 1.0f;
bool mouseDown = false;

VkvgDevice device = NULL;
VkvgSurface surf = NULL;

uint32_t test_size = 100;  // items drawn in one run, or complexity
int iterations = 10000;   // repeat test n times

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
    e = vkengine_create (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_MAILBOX_KHR, width, height);
    VkhPresenter r = e->renderer;
    vkengine_set_key_callback (e, key_callback);
    vkengine_set_mouse_but_callback(e, mouse_button_callback);
    vkengine_set_cursor_pos_callback(e, mouse_move_callback);
    vkengine_set_scroll_callback(e, scroll_callback);

    bool deferredResolve = false;

    device  = vkvg_device_create_multisample(vkh_app_get_inst(e->app), r->dev->phy, r->dev->dev, r->qFam, 0, samples, deferredResolve);

    vkvg_device_set_dpy(device, 96, 96);

    surf    = vkvg_surface_create(device, width, height);

    vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), width, height);
}
void run_test_func (void(*testfunc)(void),uint32_t width, uint32_t height) {
    bool deferredResolve = false;
    VkhPresenter r = e->renderer;

    double start_time, stop_time, run_time, run_total, min_run_time = -1, max_run_time;
    double run_time_values[iterations];

    int i = 0;

    while (!vkengine_should_close (e) && i < iterations) {
        glfwPollEvents();

        start_time = get_tick();

        if (!paused)
            testfunc();

        if (deferredResolve)
            vkvg_multisample_surface_resolve(surf);
        if (!vkh_presenter_draw (r))
            vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), width, height);

        vkDeviceWaitIdle(e->dev->dev);

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

    double avg_run_time = run_total / (double)iterations;
    double med_run_time = median_run_time (run_time_values, iterations);
    double standard_dev = standard_deviation (run_time_values, iterations, avg_run_time);
    double avg_frames_per_second = (1.0 / avg_run_time);
    avg_frames_per_second = (avg_frames_per_second<9999) ? avg_frames_per_second:9999;

    printf ("size:%d iter:%d  avgFps: %f avg: %4.2f%% med: %4.2f%% sd: %4.2f%% \n", test_size, iterations, avg_frames_per_second, avg_run_time, med_run_time, standard_dev);

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

void perform_test (void(*testfunc)(void),uint32_t width, uint32_t height) {
    //dumpLayerExts();

    e = vkengine_create (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_MAILBOX_KHR, width, height);
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
    surf    = vkvg_surface_create(device, width, height);
    vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), width, height);
#endif


    double start_time, stop_time, run_time, run_total, min_run_time = -1, max_run_time;
    double run_time_values[iterations];

    int i = 0;

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
        if (!vkh_presenter_draw (r))
            vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), width, height);
#endif

        vkDeviceWaitIdle(e->dev->dev);

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

    double avg_run_time = run_total / (double)iterations;
    double med_run_time = median_run_time (run_time_values, iterations);
    double standard_dev = standard_deviation (run_time_values, iterations, avg_run_time);
    double avg_frames_per_second = (1.0 / avg_run_time);
    avg_frames_per_second = (avg_frames_per_second<9999) ? avg_frames_per_second:9999;

    printf ("size:%d iter:%d  avgFps: %f avg: %4.2f%% med: %4.2f%% sd: %4.2f%% \n", test_size, iterations, avg_frames_per_second, avg_run_time, med_run_time, standard_dev);

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

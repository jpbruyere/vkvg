#include "test.h"

float panX = 0.f;
float panY = 0.f;
float lastX = 0.f;
float lastY = 0.f;
float zoom = 1.0f;
bool mouseDown = false;

VkvgDevice device = NULL;
VkvgSurface surf = NULL;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;
    switch (key) {
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

void perform_test (void(*testfunc)(void),uint width, uint height) {
    //dumpLayerExts();

    vk_engine_t* e = vkengine_create (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_MAILBOX_KHR, width, height);
    VkhPresenter r = e->renderer;
    vkengine_set_key_callback (e, key_callback);
    vkengine_set_mouse_but_callback(e, mouse_button_callback);
    vkengine_set_cursor_pos_callback(e, mouse_move_callback);
    vkengine_set_scroll_callback(e, scroll_callback);

    bool deferredResolve = false;

    device  = vkvg_device_create_multisample(vkh_app_get_inst(e->app), r->dev->phy, r->dev->dev, r->qFam, 0, VK_SAMPLE_COUNT_8_BIT, deferredResolve);

    vkvg_device_set_dpy(device, 96, 96);

    surf    = vkvg_surface_create(device, width, height);

    vkvg_surface_clear(surf);

    vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), width, height);

    struct timeval before , after;
    double frameTime = 0, frameTimeAccum = 0, frameCount = 0;

    while (!vkengine_should_close (e)) {
        glfwPollEvents();

        gettimeofday(&before , NULL);

        testfunc();

        if (deferredResolve)
            vkvg_multisample_surface_resolve(surf);
        if (!vkh_presenter_draw (r))
            vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), width, height);

        vkDeviceWaitIdle(e->dev->dev);

        gettimeofday(&after , NULL);

        frameTimeAccum += time_diff(before , after);
        frameCount++;
    }

    frameTime = frameTimeAccum / frameCount;
    printf ("frame (µs): %.0lf\nfps: %lf\n", frameTime, floor(1000000 / frameTime));

    vkDeviceWaitIdle(e->dev->dev);

    vkvg_surface_destroy    (surf);
    vkvg_device_destroy     (device);

    vkengine_destroy (e);

}

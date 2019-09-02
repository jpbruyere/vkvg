#include "test.h"

float panX = 0.f;
float panY = 0.f;
float lastX = 0.f;
float lastY = 0.f;
float zoom = 1.0f;
bool mouseDown = false;

VkvgDevice device = NULL;
VkvgSurface surf = NULL;

uint iterations = 250;  // items drawn in one run, or complexity
uint runs       = 10;   // repeat test n times

static vk_engine_t* e;

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

void init_test (uint width, uint height){
    e = vkengine_create (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_MAILBOX_KHR, width, height);
    VkhPresenter r = e->renderer;
    vkengine_set_key_callback (e, key_callback);
    vkengine_set_mouse_but_callback(e, mouse_button_callback);
    vkengine_set_cursor_pos_callback(e, mouse_move_callback);
    vkengine_set_scroll_callback(e, scroll_callback);

    bool deferredResolve = true;

    device  = vkvg_device_create_multisample(vkh_app_get_inst(e->app), r->dev->phy, r->dev->dev, r->qFam, 0, VK_SAMPLE_COUNT_4_BIT, deferredResolve);

    vkvg_device_set_dpy(device, 96, 96);

    surf    = vkvg_surface_create(device, width, height);

    vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), width, height);
}
void run_test_func (void(*testfunc)(void),uint width, uint height) {
    bool deferredResolve = false;
    VkhPresenter r = e->renderer;

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

void perform_test (void(*testfunc)(void),uint width, uint height) {
    //dumpLayerExts();

    e = vkengine_create (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_MAILBOX_KHR, width, height);
    VkhPresenter r = e->renderer;
    vkengine_set_key_callback (e, key_callback);
    vkengine_set_mouse_but_callback(e, mouse_button_callback);
    vkengine_set_cursor_pos_callback(e, mouse_move_callback);
    vkengine_set_scroll_callback(e, scroll_callback);

    bool deferredResolve = false;

    device  = vkvg_device_create_multisample(vkh_app_get_inst(e->app), r->dev->phy, r->dev->dev, r->qFam, 0, VK_SAMPLE_COUNT_4_BIT, deferredResolve);

    vkvg_device_set_dpy(device, 96, 96);

#ifdef VKVG_TEST_DIRECT_DRAW
    VkFence* fences = (VkFence*)calloc(r->imgCount, sizeof (VkFence));
    surfaces = (VkvgSurface*)malloc(r->imgCount * sizeof (VkvgSurface));
    for (uint i=0; i < r->imgCount;i++)
        surfaces[i] = vkvg_surface_create_for_VkhImage (device, r->ScBuffers[i]);
#else
    surf    = vkvg_surface_create(device, width, height);
    vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), width, height);
#endif
    struct timeval before , after;
    double frameTime = 0, frameTimeAccum = 0, frameCount = 0;

    while (!vkengine_should_close (e)) {
        glfwPollEvents();

        gettimeofday(&before , NULL);

#ifdef VKVG_TEST_DIRECT_DRAW
        VkFence fence = vkh_fence_create (e->dev);

        if (!vkh_presenter_acquireNextImage(r, fence)) {
            for (uint i=0; i < r->imgCount;i++){
                if (fences[i]!=NULL){
                    vkDestroyFence (e->dev->dev, fences[i], NULL);
                    fences[i] = NULL;
                }
                vkvg_surface_destroy(surfaces[i]);
                surfaces[i] = vkvg_surface_create_for_VkhImage (device, r->ScBuffers[i]);
            }
            vkDestroyFence (e->dev->dev, fence, NULL);
        }else{
            surf = surfaces[r->currentScBufferIndex];
            if (fences[r->currentScBufferIndex] != NULL){
                vkWaitForFences (e->dev->dev, 1, &fences[r->currentScBufferIndex], VK_TRUE, UINT64_MAX);
                vkDestroyFence (e->dev->dev, fences[r->currentScBufferIndex], NULL);
            }
            fences[r->currentScBufferIndex] = fence;

            testfunc();

            if (deferredResolve)
                vkvg_multisample_surface_resolve(surf);

            VkPresentInfoKHR present = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                         .swapchainCount = 1,
                                         .pSwapchains = &r->swapChain,
                                         .pImageIndices = &r->currentScBufferIndex };

            /* Make sure command buffer is finished before presenting */
            VK_CHECK_RESULT(vkQueuePresentKHR(r->queue, &present));
        }
#else
        testfunc();

        if (deferredResolve)
            vkvg_multisample_surface_resolve(surf);
        if (!vkh_presenter_draw (r))
            vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), width, height);
#endif

        //vkDeviceWaitIdle(e->dev->dev);

        gettimeofday(&after , NULL);

        frameTimeAccum += time_diff(before , after);
        frameCount++;
        //fflush(stdout);
    }

    frameTime = frameTimeAccum / frameCount;
    printf ("frame (µs): %.0lf\nfps: %lf\n", frameTime, floor(1000000 / frameTime));

    vkDeviceWaitIdle(e->dev->dev);

#ifdef VKVG_TEST_DIRECT_DRAW
    for (int i=0; i<r->imgCount;i++){
        vkvg_surface_destroy (surfaces[i]);
        if (fences[i]!=NULL)
            vkDestroyFence (e->dev->dev, fences[i], NULL);
    }
    free (fences);
    free (surfaces);
#else
    vkvg_surface_destroy    (surf);
#endif

    vkvg_device_destroy     (device);

    vkengine_destroy (e);

}

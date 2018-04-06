#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "vke.h"
#include "compute.h"
#include "vkhelpers.h"
#include "vkcrow.h"


bool vkeCheckPhyPropBlitSource (VkEngine *e) {
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(e->phy, e->renderer.format, &formatProps);
    assert((formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) && "Format cannot be used as transfer source");
}

void initPhySurface(VkEngine* e, VkFormat preferedFormat, VkPresentModeKHR presentMode){
    vkh_presenter* r = &e->renderer;

    uint32_t count;
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR (e->phy, r->surface, &count, NULL));
    assert (count>0);
    VkSurfaceFormatKHR formats[count];
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR (e->phy, r->surface, &count, formats));

    for (int i=0; i<count; i++){
        if (formats[i].format == preferedFormat) {
            r->format = formats[i].format;
            r->colorSpace = formats[i].colorSpace;
            break;
        }
    }
    assert (r->format != VK_FORMAT_UNDEFINED);

    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(e->phy, r->surface, &count, NULL));
    assert (count>0);
    VkPresentModeKHR presentModes[count];
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(e->phy, r->surface, &count, presentModes));
    r->presentMode = -1;
    for (int i=0; i<count; i++){
        if (presentModes[i] == presentMode) {
            r->presentMode = presentModes[i];
            break;
        }
    }
    assert (r->presentMode >= 0);
}

void createSwapChain (VkEngine* e){
    // Ensure all operations on the device have been finished before destroying resources
    vkDeviceWaitIdle(e->dev);
    vkh_presenter* r = &e->renderer;

    VkSurfaceCapabilitiesKHR surfCapabilities;
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(e->phy, r->surface, &surfCapabilities));
    assert (surfCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT);


    // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
    if (surfCapabilities.currentExtent.width == 0xFFFFFFFF) {
        // If the surface size is undefined, the size is set to
        // the size of the images requested
        if (r->width < surfCapabilities.minImageExtent.width)
            r->width = surfCapabilities.minImageExtent.width;
        else if (r->width > surfCapabilities.maxImageExtent.width)
            r->width = surfCapabilities.maxImageExtent.width;

        if (r->height < surfCapabilities.minImageExtent.height)
            r->height = surfCapabilities.minImageExtent.height;
        else if (r->height > surfCapabilities.maxImageExtent.height)
            r->height = surfCapabilities.maxImageExtent.height;
    } else {
        // If the surface size is defined, the swap chain size must match
        r->width = surfCapabilities.currentExtent.width;
        r->height= surfCapabilities.currentExtent.height;
    }

    VkSwapchainKHR oldSwapchain = r->swapChain;
    VkSwapchainCreateInfoKHR createInfo = { .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                            .surface = r->surface,
                                            .minImageCount = surfCapabilities.minImageCount,
                                            .imageFormat = r->format,
                                            .imageColorSpace = r->colorSpace,
                                            .imageExtent = {r->width,r->height},
                                            .imageArrayLayers = 1,
                                            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                            .preTransform = surfCapabilities.currentTransform,
                                            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                            .presentMode = r->presentMode,
                                            .clipped = VK_TRUE,
                                            .oldSwapchain = oldSwapchain};

    VK_CHECK_RESULT(vkCreateSwapchainKHR (e->dev, &createInfo, NULL, &r->swapChain));

    if (oldSwapchain != VK_NULL_HANDLE)
    {
        for (uint32_t i = 0; i < r->imgCount; i++)
        {
            vkDestroyImageView(e->dev, r->ScBuffers[i].view, NULL);
            vkFreeCommandBuffers (e->dev, e->renderer.cmdPool, 1, &r->cmdBuffs[i]);
        }
        vkDestroySwapchainKHR(e->dev, oldSwapchain, NULL);
        free(r->ScBuffers);
        free(r->cmdBuffs);
    }

    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(e->dev, r->swapChain, &r->imgCount, NULL));
    assert (r->imgCount>0);

    VkImage images[r->imgCount];
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(e->dev, r->swapChain, &r->imgCount,images));

    r->ScBuffers = (ImageBuffer*)malloc(sizeof(ImageBuffer)*r->imgCount);
    r->cmdBuffs = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer)*r->imgCount);

    for (int i=0; i<r->imgCount; i++) {
        ImageBuffer sc_buffer;

        VkImageViewCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                             .image = images[i],
                                             .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                             .format = r->format,
                                             .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}};
        assert (vkCreateImageView(e->dev, &createInfo, NULL, &sc_buffer.view) == VK_SUCCESS);
        sc_buffer.image = images[i];
        r->ScBuffers [i] = sc_buffer;
        r->cmdBuffs [i] = vkh_cmd_buff_create(e->dev, e->renderer.cmdPool);
    }
    r->currentScBufferIndex = 0;
}

void EngineInit (VkEngine* e) {
    glfwInit();
    assert (glfwVulkanSupported()==GLFW_TRUE);
    e->ExtensionNames = glfwGetRequiredInstanceExtensions (&e->EnabledExtensionsCount);

    e->infos.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    e->infos.pNext = NULL;
    e->infos.pApplicationName = APP_SHORT_NAME;
    e->infos.applicationVersion = 1;
    e->infos.pEngineName = APP_SHORT_NAME;
    e->infos.engineVersion = 1;
    e->infos.apiVersion = VK_API_VERSION_1_0;
    e->renderer.width = 1024;
    e->renderer.height = 800;

    const uint32_t enabledLayersCount = 1;
    const char* enabledLayers[] = {"VK_LAYER_LUNARG_core_validation"};

    VkInstanceCreateInfo inst_info = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                       .pNext = NULL,
                                       .flags = 0,
                                       .pApplicationInfo = &e->infos,
                                       .enabledExtensionCount = e->EnabledExtensionsCount,
                                       .ppEnabledExtensionNames = e->ExtensionNames,
                                       .enabledLayerCount = enabledLayersCount,
                                       .ppEnabledLayerNames = enabledLayers };

    VK_CHECK_RESULT(vkCreateInstance (&inst_info, NULL, &e->inst));

    e->phy = vkh_find_phy (e->inst, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);

    vkGetPhysicalDeviceMemoryProperties(e->phy, &e->memory_properties);
    vkGetPhysicalDeviceProperties(e->phy, &e->gpu_props);

    uint32_t queue_family_count = 0;
    int cQueue = -1, gQueue = -1, tQueue = -1;
    vkGetPhysicalDeviceQueueFamilyProperties (e->phy, &queue_family_count, NULL);
    VkQueueFamilyProperties qfams[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties (e->phy, &queue_family_count, &qfams);

    //try to find dedicated queues
    for (int j=0; j<queue_family_count; j++){
        switch (qfams[j].queueFlags) {
        case VK_QUEUE_GRAPHICS_BIT:
            if (gQueue<0)
                gQueue = j;
            break;
        case VK_QUEUE_COMPUTE_BIT:
            if (cQueue<0)
                cQueue = j;
            break;
        case VK_QUEUE_TRANSFER_BIT:
            if (tQueue<0)
                tQueue = j;
            break;
        }
    }
    //try to find suitable queue if no dedicated one found
    for (int j=0; j<queue_family_count; j++){
        if ((qfams[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (gQueue < 0))
            gQueue = j;
        if ((qfams[j].queueFlags & VK_QUEUE_COMPUTE_BIT) && (cQueue < 0))
            cQueue = j;
        if ((qfams[j].queueFlags & VK_QUEUE_TRANSFER_BIT) && (tQueue < 0))
            tQueue = j;
    }
    if (gQueue<0||cQueue<0||tQueue<0){
        fprintf (stderr, "Missing Queue type\n");
        exit (-1);
    }

    uint32_t qCount = 0;
    VkDeviceQueueCreateInfo pQueueInfos[3];
    float queue_priorities[1] = {0.0};

    VkDeviceQueueCreateInfo qiG = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                   .queueCount = 1,
                                   .queueFamilyIndex = gQueue,
                                   .pQueuePriorities = queue_priorities };
    VkDeviceQueueCreateInfo qiC = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                   .queueCount = 1,
                                   .queueFamilyIndex = cQueue,
                                   .pQueuePriorities = queue_priorities };
    VkDeviceQueueCreateInfo qiT = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                   .queueCount = 1,
                                   .queueFamilyIndex = tQueue,
                                   .pQueuePriorities = queue_priorities };

    if (gQueue == cQueue){
        if(gQueue == tQueue){
            qCount=1;
            pQueueInfos[0] = qiG;
        }else{
            qCount=2;
            pQueueInfos[0] = qiG;
            pQueueInfos[1] = qiT;
        }
    }else{
        if((gQueue == tQueue) || (cQueue==tQueue)){
            qCount=2;
            pQueueInfos[0] = qiG;
            pQueueInfos[1] = qiC;
        }else{
            qCount=3;
            pQueueInfos[0] = qiG;
            pQueueInfos[1] = qiC;
            pQueueInfos[2] = qiT;
        }
    }

    VkDeviceCreateInfo device_info = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                       .queueCreateInfoCount = qCount,
                                       .pQueueCreateInfos = &pQueueInfos};

    VK_CHECK_RESULT(vkCreateDevice(e->phy, &device_info, NULL, &e->dev));

    assert (glfwGetPhysicalDevicePresentationSupport (e->inst, e->phy, gQueue)==GLFW_TRUE);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    vkh_presenter* r = &e->renderer;

    r->window = glfwCreateWindow(r->width, r->height, "Window Title", NULL, NULL);

    assert (glfwCreateWindowSurface(e->inst, r->window, NULL, &r->surface)==VK_SUCCESS);

    VkBool32 isSupported;
    vkGetPhysicalDeviceSurfaceSupportKHR(e->phy, gQueue, r->surface, &isSupported);
    assert (isSupported && "vkGetPhysicalDeviceSurfaceSupportKHR");

    vkGetDeviceQueue(e->dev, gQueue, 0, &e->renderer.queue);
    vkGetDeviceQueue(e->dev, cQueue, 0, &e->computer.queue);
    vkGetDeviceQueue(e->dev, tQueue, 0, &e->loader.queue);

    e->renderer.cmdPool = vkh_cmd_pool_create (e->dev, gQueue, 0);
    e->computer.cmdPool = vkh_cmd_pool_create (e->dev, cQueue, 0);
    e->loader.cmdPool = vkh_cmd_pool_create (e->dev, tQueue, 0);

    r->semaPresentEnd = vkh_semaphore_create(e->dev);
    r->semaDrawEnd = vkh_semaphore_create(e->dev);

    initPhySurface(e,VK_FORMAT_B8G8R8A8_UNORM,VK_PRESENT_MODE_FIFO_KHR);

    createSwapChain(e);
}
void EngineTerminate (VkEngine* e) {
    vkDeviceWaitIdle(e->dev);
    vkcrow_terminate();
    vkh_presenter* r = &e->renderer;

    vkDestroySemaphore(e->dev, r->semaDrawEnd, NULL);
    vkDestroySemaphore(e->dev, r->semaPresentEnd, NULL);

    for (int i=0; i<r->imgCount; i++){
        vkDestroyImageView (e->dev, r->ScBuffers[i].view, NULL);
        vkFreeCommandBuffers (e->dev, r->cmdPool, 1, &r->cmdBuffs[i]);
    }
    free (r->ScBuffers);
    free (r->cmdBuffs);

    vkDestroyCommandPool (e->dev, e->renderer.cmdPool, NULL);
    vkDestroyCommandPool (e->dev, e->computer.cmdPool, NULL);
    vkDestroyCommandPool (e->dev, e->loader.cmdPool, NULL);

    vkDestroySwapchainKHR(e->dev, r->swapChain, NULL);
    vkDestroyDevice (e->dev, NULL);
    vkDestroySurfaceKHR (e->inst, r->surface, NULL);
    glfwDestroyWindow (r->window);
    glfwTerminate ();

    vkDestroyInstance (e->inst, NULL);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS){
        vkcrow_key_up(scancode);
    }else if (action == GLFW_RELEASE)
        vkcrow_key_up(scancode);

    if (action != GLFW_PRESS)
        return;
    switch (key) {
    case GLFW_KEY_ESCAPE :
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case GLFW_KEY_F3 :
        vkcrow_load("ifaces/0.crow");
        break;
    case GLFW_KEY_F4 :
        vkcrow_load("ifaces/1.crow");
        break;
    case GLFW_KEY_F5 :
        vkcrow_load("ifaces/2.crow");
        break;
    }
}

static void char_callback (GLFWwindow* window, uint32_t c){
    vkcrow_key_press(c);
}

static void mouse_move_callback(GLFWwindow* window, double x, double y){
    vkcrow_mouse_move((int)x,(int)y);
}
static void mouse_button_callback(GLFWwindow* window, int but, int state, int modif){
    if (state == GLFW_PRESS)
        vkcrow_mouse_down(but);
    else
        vkcrow_mouse_up(but);
}


void buildCommandBuffers(vkh_presenter* r){
    for (int i=0;i<r->imgCount;i++) {
        vkcrow_cmd_copy_create(r->cmdBuffs[i],r->ScBuffers[i].image,r->width,r->height);
    }
}


void draw(VkEngine* e) {
    vkh_presenter* r = &e->renderer;
    // Get the index of the next available swapchain image:
    VkResult err = vkAcquireNextImageKHR(e->dev, r->swapChain, UINT64_MAX, r->semaPresentEnd, VK_NULL_HANDLE,
                                &r->currentScBufferIndex);
    if ((err == VK_ERROR_OUT_OF_DATE_KHR) || (err == VK_SUBOPTIMAL_KHR)){
        createSwapChain(e);
        vkcrow_resize(e->dev,e->memory_properties,e->renderer.width,e->renderer.height);
        buildCommandBuffers(r);
    }else{
        VK_CHECK_RESULT(err);
        vkcrow_cmd_copy_submit (r->queue, &r->cmdBuffs[r->currentScBufferIndex], &r->semaPresentEnd, &r->semaDrawEnd);

        /* Now present the image in the window */
        VkPresentInfoKHR present = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                     .swapchainCount = 1,
                                     .pSwapchains = &r->swapChain,
                                     .waitSemaphoreCount = 1,
                                     .pWaitSemaphores = &r->semaDrawEnd,
                                     .pImageIndices = &r->currentScBufferIndex };

        /* Make sure command buffer is finished before presenting */
        VK_CHECK_RESULT(vkQueuePresentKHR(r->queue, &present));
    }
    vkcrow_buffer_update();
}

int main(int argc, char *argv[]) {
    dumpLayerExts();

    VkEngine e = {};

    EngineInit(&e);

    vkcrow_start();

    vkeCheckPhyPropBlitSource (&e);


    glfwSetKeyCallback(e.renderer.window, key_callback);
    glfwSetCharCallback(e.renderer.window, char_callback);
    glfwSetCursorPosCallback(e.renderer.window, mouse_move_callback);
    glfwSetMouseButtonCallback(e.renderer.window, mouse_button_callback);
    //glfwSetWindowSizeCallback(e.renderer.window, win_resize_callback);

    vkcrow_resize(e.dev,e.memory_properties,e.renderer.width,e.renderer.height);
    buildCommandBuffers(&e.renderer);
    vkcrow_load("/mnt/devel/gts/libvk/crow/Tests/Interfaces/Divers/0.crow");

    while (!glfwWindowShouldClose(e.renderer.window)) {
        glfwPollEvents();
        draw(&e);
    }

    EngineTerminate (&e);

    return 0;
}

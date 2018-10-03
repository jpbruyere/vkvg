/*
 * Copyright (c) 2018 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "vkh.h"
#include "vkengine.h"
#include "vkh_app.h"
#include "vkh_phyinfo.h"
#include "vkh_presenter.h"
#include "vkh_image.h"
#include "vkh_device.h"

bool vkeCheckPhyPropBlitSource (VkEngine e) {
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(e->dev->phy, e->renderer->format, &formatProps);
    assert((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) && "Format cannot be used as transfer source");
}

VkSampleCountFlagBits getMaxUsableSampleCount(VkSampleCountFlags counts)
{
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
    return VK_SAMPLE_COUNT_1_BIT;
}

void vkengine_dump_Infos (VkEngine e){
    printf("max samples = %d\n", getMaxUsableSampleCount(e->gpu_props.limits.framebufferColorSampleCounts));
    printf("max tex2d size = %d\n", e->gpu_props.limits.maxImageDimension2D);
    printf("max tex array layers = %d\n", e->gpu_props.limits.maxImageArrayLayers);
    printf("max mem alloc count = %d\n", e->gpu_props.limits.maxMemoryAllocationCount);

    for (int i = 0; i < e->memory_properties.memoryHeapCount; i++) {
        printf("Mem Heap %d\n", i);
        printf("\tflags= %d\n", e->memory_properties.memoryHeaps[i].flags);
        printf("\tsize = %d Mo\n", e->memory_properties.memoryHeaps[i].size/ (1024*1024));
    }
    for (int i = 0; i < e->memory_properties.memoryTypeCount; i++) {
        printf("Mem type %d\n", i);
        printf("\theap %d: ", e->memory_properties.memoryTypes[i].heapIndex);
        if (e->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            printf("VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|");
        if (e->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            printf("VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|");
        if (e->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            printf("VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|");
        if (e->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
            printf("VK_MEMORY_PROPERTY_HOST_CACHED_BIT|");
        if (e->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
            printf("VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT|");
        printf("\n");
    }
}
vk_engine_t* vkengine_create (VkPhysicalDeviceType preferedGPU, uint32_t width, uint32_t height) {
    vk_engine_t* e = (vk_engine_t*)calloc(1,sizeof(vk_engine_t));

    glfwInit();
    assert (glfwVulkanSupported()==GLFW_TRUE);

    uint32_t enabledExtsCount = 0, phyCount = 0;
    const char ** enabledExts = glfwGetRequiredInstanceExtensions (&enabledExtsCount);

    e->app = vkh_app_create("vkvgTest", enabledExtsCount, enabledExts);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE,  GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING,   GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED,  GLFW_TRUE);

    e->window = glfwCreateWindow (width, height, "Window Title", NULL, NULL);
    VkSurfaceKHR surf;

    assert (glfwCreateWindowSurface(e->app->inst, e->window, NULL, &surf)==VK_SUCCESS);


    VkhPhyInfo* phys = vkh_app_get_phyinfos (e->app, &phyCount, surf);

    VkhPhyInfo pi = NULL;
    for (int i=0; i<phyCount; i++){
        pi = phys[i];
        if (pi->properties.deviceType == preferedGPU)
            break;
    }

    e->memory_properties = pi->memProps;
    e->gpu_props = pi->properties;

    uint32_t qCount = 0;
    VkDeviceQueueCreateInfo pQueueInfos[3];
    float queue_priorities[] = {0.0};

    VkDeviceQueueCreateInfo qiG = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                   .queueCount = 1,
                                   .queueFamilyIndex = pi->gQueue,
                                   .pQueuePriorities = queue_priorities };
    VkDeviceQueueCreateInfo qiC = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                   .queueCount = 1,
                                   .queueFamilyIndex = pi->cQueue,
                                   .pQueuePriorities = queue_priorities };
    VkDeviceQueueCreateInfo qiT = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                   .queueCount = 1,
                                   .queueFamilyIndex = pi->tQueue,
                                   .pQueuePriorities = queue_priorities };

    if (pi->gQueue == pi->cQueue){
        if(pi->gQueue == pi->tQueue){
            qCount=1;
            pQueueInfos[0] = qiG;
        }else{
            qCount=2;
            pQueueInfos[0] = qiG;
            pQueueInfos[1] = qiT;
        }
    }else{
        if((pi->gQueue == pi->tQueue) || (pi->cQueue==pi->tQueue)){
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

    char const * dex [] = {"VK_KHR_swapchain"};
#if VKVG_USE_VALIDATION
    uint32_t dlayCpt = 1;
    static char const * dlay [] = {"VK_LAYER_LUNARG_standard_validation"};
#else
    uint32_t dlayCpt = 0;
    static char const * dlay [] = {};
#endif
    VkPhysicalDeviceFeatures enabledFeatures = {
        .fillModeNonSolid = true,
        .sampleRateShading = true
    };

    VkDeviceCreateInfo device_info = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                       .queueCreateInfoCount = qCount,
                                       .pQueueCreateInfos = &pQueueInfos,
                                       .enabledLayerCount = dlayCpt,
                                       .ppEnabledLayerNames = dlay,
                                       .enabledExtensionCount = 1,
                                       .ppEnabledExtensionNames = dex,
                                       .pEnabledFeatures = &enabledFeatures
                                     };

    VkDevice dev;
    VK_CHECK_RESULT(vkCreateDevice (pi->phy, &device_info, NULL, &dev));
    e->dev = vkh_device_create(pi->phy, dev);

    e->renderer = vkh_presenter_create
            (e->dev, pi->pQueue, surf, width, height, VK_FORMAT_B8G8R8A8_UNORM, VK_PRESENT_MODE_MAILBOX_KHR);


    vkh_app_free_phyinfos (phyCount, phys);

    return e;
}

void vkengine_destroy (VkEngine e) {
    vkDeviceWaitIdle(e->dev->dev);

    VkSurfaceKHR surf = e->renderer->surface;

    vkh_presenter_destroy (e->renderer);
    vkDestroySurfaceKHR (e->app->inst, surf, NULL);

    vkDestroyDevice (e->dev->dev, NULL);

    glfwDestroyWindow (e->window);
    glfwTerminate ();

    vkh_app_destroy (e->app);

    free(e);
}
void vkengine_close (VkEngine e) {
    glfwSetWindowShouldClose(e->window, GLFW_TRUE);
}
void vkengine_blitter_run (VkEngine e, VkImage img, uint32_t width, uint32_t height) {
    VkhPresenter p = e->renderer;
    vkh_presenter_build_blit_cmd (p, img, width, height);

    while (!vkengine_should_close (e)) {
        glfwPollEvents();
        if (!vkh_presenter_draw (p))
            vkh_presenter_build_blit_cmd (p, img, width, height);
    }
}
inline bool vkengine_should_close (VkEngine e) {
    return glfwWindowShouldClose (e->window);
}

VkDevice vkengine_get_device (VkEngine e){
    return e->dev->dev;
}
VkPhysicalDevice vkengine_get_physical_device (VkEngine e){
    return e->dev->phy;
}
VkQueue vkengine_get_queue (VkEngine e){
    return e->renderer->queue;
}
uint32_t vkengine_get_queue_fam_idx (VkEngine e){
    return e->renderer->qFam;
}

void vkengine_set_key_callback (VkEngine e, GLFWkeyfun key_callback){
    glfwSetKeyCallback (e->window, key_callback);
}
void vkengine_set_mouse_but_callback (VkEngine e, GLFWmousebuttonfun onMouseBut){
    glfwSetMouseButtonCallback(e->window, onMouseBut);
}
void vkengine_set_cursor_pos_callback (VkEngine e, GLFWcursorposfun onMouseMove){
    glfwSetCursorPosCallback(e->window, onMouseMove);
}
void vkengine_set_scroll_callback (VkEngine e, GLFWscrollfun onScroll){
    glfwSetScrollCallback(e->window, onScroll);
}
void vkengine_set_char_callback (VkEngine e, GLFWcharfun onChar){
    glfwSetCharCallback(e->window, onChar);
}


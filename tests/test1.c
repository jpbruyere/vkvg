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

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

#include "vke.h"
//#include "compute.h"
#include "vkh.h"
#include "vkh_app.h"
#include "vkh_phyinfo.h"

#include "vkvg.h"

VkvgDevice device;
VkvgSurface surf = NULL;

void vke_swapchain_destroy (vkh_presenter* r);
void vke_swapchain_create (VkEngine* e);

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

void vke_swapchain_create (VkEngine* e){
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

    VkSwapchainKHR newSwapchain;
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
                                            .oldSwapchain = r->swapChain};

    VK_CHECK_RESULT(vkCreateSwapchainKHR (e->dev, &createInfo, NULL, &newSwapchain));
    if (r->swapChain != VK_NULL_HANDLE)
        vke_swapchain_destroy(r);
    r->swapChain = newSwapchain;

    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(e->dev, r->swapChain, &r->imgCount, NULL));
    assert (r->imgCount>0);

    VkImage images[r->imgCount];
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(e->dev, r->swapChain, &r->imgCount,images));

    r->ScBuffers = (ImageBuffer*)malloc(sizeof(ImageBuffer)*r->imgCount);
    r->cmdBuffs = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer)*r->imgCount);

    for (int i=0; i<r->imgCount; i++) {
        ImageBuffer sc_buffer = {};
        VkImageViewCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                             .image = images[i],
                                             .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                             .format = r->format,
                                             .components = {VK_COMPONENT_SWIZZLE_R,VK_COMPONENT_SWIZZLE_G,VK_COMPONENT_SWIZZLE_B,VK_COMPONENT_SWIZZLE_A},
                                             .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}};
        VK_CHECK_RESULT(vkCreateImageView(e->dev, &createInfo, NULL, &sc_buffer.view));
        sc_buffer.image = images[i];
        r->ScBuffers [i] = sc_buffer;
        r->cmdBuffs [i] = vkh_cmd_buff_create(e->dev, e->renderer.cmdPool,VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }
    r->currentScBufferIndex = 0;
}
void vke_swapchain_destroy (vkh_presenter* r){
    for (uint32_t i = 0; i < r->imgCount; i++)
    {
        vkDestroyImageView (r->dev, r->ScBuffers[i].view, NULL);
        vkFreeCommandBuffers (r->dev, r->cmdPool, 1, &r->cmdBuffs[i]);
    }
    vkDestroySwapchainKHR(r->dev, r->swapChain, NULL);
    free(r->ScBuffers);
    free(r->cmdBuffs);
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

void vkengine_dump_Infos (VkEngine* e){
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
void vkengine_get_queues_properties (VkEngine* e, VkQueueFamilyProperties** qFamProps, uint32_t* count){
    vkGetPhysicalDeviceQueueFamilyProperties (e->phy, count, NULL);
    (*qFamProps) = (VkQueueFamilyProperties*)malloc((*count) * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties (e->phy, count, (*qFamProps));
}
void EngineInit (VkEngine* e) {
    glfwInit();
    assert (glfwVulkanSupported()==GLFW_TRUE);

    uint32_t enabledExtsCount = 0, phyCount = 0;
    const char** enabledExts = glfwGetRequiredInstanceExtensions (&enabledExtsCount);

    e->app = vkh_app_create("vkvgTest", enabledExtsCount, enabledExts);

    VkhPhyInfo* phys = vkh_app_get_phyinfos(e->app, &phyCount);

    VkhPhyInfo pi = NULL;
    for (int i=0; i<phyCount; i++){
        pi = phys[i];
        if (pi->properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            e->phy = pi->phy;
            break;
        }
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

    VkDeviceCreateInfo device_info = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                       .queueCreateInfoCount = qCount,
                                       .pQueueCreateInfos = &pQueueInfos};

    VK_CHECK_RESULT(vkCreateDevice(e->phy, &device_info, NULL, &e->dev));


    assert (glfwGetPhysicalDevicePresentationSupport (e->app->inst, e->phy, pi->gQueue)==GLFW_TRUE);

    e->renderer.width = 1024;
    e->renderer.height = 800;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE,  GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING,   GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED,  GLFW_FALSE);

    vkh_presenter* r = &e->renderer;
    r->dev = e->dev;

    r->window = glfwCreateWindow(r->width, r->height, "Window Title", NULL, NULL);

    assert (glfwCreateWindowSurface(e->app->inst, r->window, NULL, &r->surface)==VK_SUCCESS);

    VkBool32 isSupported;
    vkGetPhysicalDeviceSurfaceSupportKHR(e->phy, pi->gQueue, r->surface, &isSupported);
    assert (isSupported && "vkGetPhysicalDeviceSurfaceSupportKHR");

    vkGetDeviceQueue(e->dev, pi->gQueue, 0, &e->renderer.queue);
    e->renderer.qFam = pi->gQueue;
    vkGetDeviceQueue(e->dev, pi->cQueue, 0, &e->computer.queue);
    vkGetDeviceQueue(e->dev, pi->tQueue, 0, &e->loader.queue);

    e->renderer.cmdPool = vkh_cmd_pool_create (e->dev, pi->gQueue, 0);
    e->computer.cmdPool = vkh_cmd_pool_create (e->dev, pi->cQueue, 0);
    e->loader.cmdPool = vkh_cmd_pool_create (e->dev, pi->tQueue, 0);

    vkh_app_free_phyinfos (phyCount, phys);

    r->semaPresentEnd = vkh_semaphore_create(e->dev);
    r->semaDrawEnd = vkh_semaphore_create(e->dev);

    initPhySurface(e,VK_FORMAT_B8G8R8A8_UNORM,VK_PRESENT_MODE_FIFO_KHR);
}

void EngineTerminate (VkEngine* e) {
    vkDeviceWaitIdle(e->dev);
    vkh_presenter* r = &e->renderer;

    vkDestroySemaphore(e->dev, r->semaDrawEnd, NULL);
    vkDestroySemaphore(e->dev, r->semaPresentEnd, NULL);

    vkDestroyCommandPool (e->dev, e->renderer.cmdPool, NULL);
    vkDestroyCommandPool (e->dev, e->computer.cmdPool, NULL);
    vkDestroyCommandPool (e->dev, e->loader.cmdPool, NULL);

    vkDestroyDevice (e->dev, NULL);
    vkDestroySurfaceKHR (e->app->inst, r->surface, NULL);
    glfwDestroyWindow (r->window);
    glfwTerminate ();

    vkh_app_destroy (e->app);
}

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
static void mouse_move_callback(GLFWwindow* window, double x, double y){}
static void mouse_button_callback(GLFWwindow* window, int but, int state, int modif){}


void setupSimpleBlit(vkh_presenter* r){
    for (int32_t i = 0; i < r->imgCount; ++i)
    {
        VkImage bltDstImage = r->ScBuffers[i].image;
        VkImage bltSrcImage = vkvg_surface_get_vk_image(surf);

        VkCommandBuffer cb = r->cmdBuffs[i];
        vkh_cmd_begin(cb,VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        set_image_layout(cb, bltDstImage, VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        set_image_layout(cb, bltSrcImage, VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        VkImageCopy cregion = { .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                                .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                                .srcOffset = {},
                                .dstOffset = {0,0,0},
                                .extent = {1024,800,1}};

        vkCmdCopyImage(cb, bltSrcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, bltDstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &cregion);

        set_image_layout(cb, bltDstImage, VK_IMAGE_ASPECT_COLOR_BIT,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

        vkh_cmd_end(cb);
    }
}
void submitCommandBuffer(VkQueue queue, VkCommandBuffer *pCmdBuff, VkSemaphore* pWaitSemaphore, VkSemaphore* pSignalSemaphore){
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                 .commandBufferCount = 1,
                                 .signalSemaphoreCount = 1,
                                 .pSignalSemaphores = pSignalSemaphore,
                                 .waitSemaphoreCount = 1,
                                 .pWaitSemaphores = pWaitSemaphore,
                                 .pWaitDstStageMask = &dstStageMask,
                                 .pCommandBuffers = pCmdBuff};
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submit_info, NULL));
}
void draw(VkEngine* e) {
    vkh_presenter* r = &e->renderer;
    // Get the index of the next available swapchain image:
    VkResult err = vkAcquireNextImageKHR(e->dev, r->swapChain, UINT64_MAX, r->semaPresentEnd, VK_NULL_HANDLE,
                                &r->currentScBufferIndex);
    if ((err == VK_ERROR_OUT_OF_DATE_KHR) || (err == VK_SUBOPTIMAL_KHR)){
        vke_swapchain_create(e);
        setupSimpleBlit(r);
    }else{
        VK_CHECK_RESULT(err);
        submitCommandBuffer (r->queue, &r->cmdBuffs[r->currentScBufferIndex], &r->semaPresentEnd, &r->semaDrawEnd);

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
}

void vkvg_test_gradient (VkvgContext ctx) {
    VkvgPattern pat = vkvg_pattern_create_linear(100,0,300,0);
    vkvg_set_line_width(ctx, 20);
    vkvg_patter_add_color_stop(pat, 0, 1, 0, 0, 1);
    vkvg_patter_add_color_stop(pat, 0.5, 0, 1, 0, 1);
    vkvg_patter_add_color_stop(pat, 1, 0, 0, 1, 1);
    vkvg_set_source (ctx, pat);
    vkvg_rectangle(ctx,100,100,200,200);
    vkvg_fill (ctx);
    //vkvg_stroke (ctx);
    vkvg_pattern_destroy (pat);
}

void vkvg_test_clip(VkvgContext ctx){
    vkvg_move_to(ctx,10,10);
    vkvg_line_to(ctx,400,150);
    vkvg_line_to(ctx,900,10);
    vkvg_line_to(ctx,700,450);
    vkvg_line_to(ctx,900,750);
    vkvg_line_to(ctx,500,650);
    vkvg_line_to(ctx,100,800);
    vkvg_line_to(ctx,150,400);
    vkvg_clip(ctx);
}
void vkvg_test_fill(VkvgContext ctx){
    vkvg_set_source_rgba(ctx,0.1,0.1,0.8,1.0);
    vkvg_move_to(ctx,100,100);
    vkvg_line_to(ctx,400,350);
    vkvg_line_to(ctx,900,150);
    vkvg_line_to(ctx,700,450);
    vkvg_line_to(ctx,900,750);
    vkvg_line_to(ctx,500,650);
    vkvg_line_to(ctx,100,800);
    vkvg_line_to(ctx,150,400);
    vkvg_close_path(ctx);
    vkvg_fill(ctx);
}

void vkvg_test_curves (VkvgContext ctx) {
    vkvg_set_source_rgba   (ctx, 0.5,0.0,1.0,0.5);
    vkvg_set_line_width(ctx, 10);


    vkvg_move_to    (ctx, 100, 400);
    vkvg_curve_to   (ctx, 100, 100, 600,700,600,400);

    vkvg_move_to    (ctx, 100, 100);
    vkvg_curve_to   (ctx, 1000, 100, 100, 800, 1000, 800);
    vkvg_move_to    (ctx, 100, 150);
    vkvg_curve_to   (ctx, 1000, 500, 700, 500, 700, 100);

    vkvg_stroke     (ctx);
}

void vkvg_test_stroke(VkvgContext ctx){
    vkvg_set_line_width(ctx, 2);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_move_to(ctx,200.5,200.5);
    vkvg_line_to(ctx,400.5,200.5);
    vkvg_line_to(ctx,400.5,400.5);
    vkvg_line_to(ctx,200.5,400.5);
    vkvg_close_path(ctx);
    vkvg_save (ctx);
    vkvg_stroke_preserve(ctx);
    vkvg_set_source_rgba(ctx,0,0.2,0.35,1);
    vkvg_fill(ctx);
    vkvg_set_source_rgba(ctx,0.5,1,0,1);
    vkvg_move_to(ctx,300.5,300.5);
    vkvg_line_to(ctx,500.5,300.5);
    vkvg_line_to(ctx,500.5,500.5);
    vkvg_line_to(ctx,300.5,500.5);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);
    vkvg_set_line_width(ctx, 40);
    vkvg_restore(ctx);
    vkvg_set_source_rgba(ctx,0.5,0.6,1,1.0);
    vkvg_move_to(ctx,700,475);
    vkvg_line_to(ctx,400,475);
    vkvg_stroke(ctx);
    vkvg_set_source_rgba(ctx,0,0.5,0.5,0.5);
    vkvg_move_to(ctx,300,200);
    vkvg_arc(ctx, 200,200,100,0, M_PI);
    vkvg_stroke(ctx);

    vkvg_set_line_width(ctx, 20);
    vkvg_set_source_rgba(ctx,0.1,0.1,0.1,0.5);
    vkvg_move_to(ctx,100,60);
    vkvg_line_to(ctx,400,600);
    vkvg_stroke(ctx);
}

void test_text (VkvgContext ctx) {
    int size = 19;
    int penY = 50;
    int penX = 10;

    /*vkvg_rectangle(ctx,30,0,100,400);
    vkvg_clip(ctx);*/

    //vkvg_select_font_face(ctx, "/usr/local/share/fonts/DroidSansMono.ttf");
    //vkvg_select_font_face(ctx, "/usr/share/fonts/truetype/unifont/unifont.ttf");

    vkvg_set_font_size(ctx,size-10);
    vkvg_select_font_face(ctx, "droid");
    vkvg_move_to(ctx, penX,penY);
    vkvg_set_source_rgba(ctx,0.7,0.7,0.7,1);
    vkvg_show_text (ctx,"abcdefghijk");
    penY+=size;


    vkvg_select_font_face(ctx, "times");
    vkvg_set_source_rgba(ctx,0.9,0.7,0.7,1);
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"abcdefghijklmnopqrstuvwxyz");
    penY+=size;



    vkvg_select_font_face(ctx, "droid");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"lmnopqrstuvwxyz123456789");
    penY+=size;



    vkvg_select_font_face(ctx, "times:bold");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"abcdefghijklmnopqrstuvwxyz");
    penY+=size;


    vkvg_select_font_face(ctx, "droid");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    penY+=size;



    vkvg_select_font_face(ctx, "arial:italic");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"abcdefghijklmnopqrstuvwxyz");
    penY+=size;


    vkvg_select_font_face(ctx, "arial");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    penY+=size;
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"this is a test");
    penY+=size;
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"this is another test to see if label is working");
    penY+=size;

    vkvg_select_font_face(ctx, "mono");
    vkvg_move_to(ctx, penX,penY);
    vkvg_show_text (ctx,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    penY+=size;

    vkvg_move_to(ctx, 80,400);
    vkvg_show_text (ctx,"Ленивый рыжий кот");


    /*vkvg_move_to(ctx, 150,250);
    vkvg_show_text (ctx,"test string é€");
    vkvg_move_to(ctx, 150,300);
    vkvg_show_text (ctx,"كسول الزنجبيل القط");
    vkvg_move_to(ctx, 150,350);
    vkvg_show_text (ctx,"懶惰的姜貓");*/

    //vkvg_show_text (ctx,"ABCDABCD");
    //vkvg_show_text (ctx,"j");
}

void vkvg_test_stroke2(VkvgContext ctx){
    vkvg_set_line_width(ctx,20);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_move_to(ctx,200,200);
    vkvg_line_to(ctx,400,200);
    vkvg_line_to(ctx,400,400);
    vkvg_line_to(ctx,200,400);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);
    vkvg_set_source_rgba(ctx,0.5,1,0,1);
    vkvg_move_to(ctx,300,300);
    vkvg_line_to(ctx,500,300);
    vkvg_line_to(ctx,500,500);
    vkvg_line_to(ctx,300,500);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);
    vkvg_set_line_width(ctx,10);
    vkvg_set_source_rgba(ctx,0.5,0.6,1,1);
    vkvg_move_to(ctx,700,475);
    vkvg_line_to(ctx,400,475);
    vkvg_stroke(ctx);
    vkvg_set_source_rgba(ctx,1,0,1,1);
    vkvg_move_to(ctx,700,500);

    vkvg_arc(ctx, 600,500,100,M_PI, 2.0*M_PI);
    vkvg_stroke(ctx);


    vkvg_set_line_width(ctx,20);
    vkvg_set_source_rgba(ctx,1,1,0,1);
    vkvg_move_to(ctx,100,50);
    vkvg_line_to(ctx,400,50);
    vkvg_stroke(ctx);
}
void vkvg_test_fill2(VkvgContext ctx){
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_move_to(ctx,200,200);
    vkvg_line_to(ctx,250,150);
    vkvg_line_to(ctx,200,100);
    vkvg_line_to(ctx,300,150);
    vkvg_line_to(ctx,700,100);
    vkvg_line_to(ctx,400,200);
    vkvg_line_to(ctx,400,400);
    vkvg_line_to(ctx,200,400);
    vkvg_line_to(ctx,300,300);
    vkvg_close_path(ctx);
    vkvg_fill(ctx);
}

void test_img_surface (VkvgContext ctx) {
    VkvgSurface imgSurf;// = vkvg_surface_create_from_image(device, "/mnt/data/images/blason.png");
    //VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "/mnt/data/images/2000px-Tux.svg.png");
    //VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "/mnt/data/images/path2674.png");
    //VkvgSurface imgSurf = vkvg_surface_create_from_image(device, "/mnt/data/images/horse-black-head-shape-of-a-chess-piece_318-52446.jpg");
    /*vkvg_set_source_surface(ctx, imgSurf, 200, 200);
    vkvg_paint(ctx);
    vkvg_set_source_surface(ctx, imgSurf, 400, 400);
    vkvg_paint(ctx);
    vkvg_flush(ctx);
    vkvg_surface_destroy(imgSurf);*/

    imgSurf = vkvg_surface_create_from_image(device, "/mnt/data/images/miroir.jpg");
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);
    vkvg_flush(ctx);
    vkvg_surface_destroy(imgSurf);
}

void test_line_caps (VkvgContext ctx) {

    float x = 20, y = 20, dx = 30, dy = 60;

    //vkvg_scale(ctx,5,5);
    vkvg_set_line_width(ctx,26);
    vkvg_set_source_rgba(ctx,0,0,0,1);
    vkvg_move_to(ctx,x,y);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_stroke(ctx);
    vkvg_set_line_cap(ctx,VKVG_LINE_CAP_SQUARE);
    vkvg_rel_move_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_stroke(ctx);
    vkvg_set_line_cap(ctx,VKVG_LINE_CAP_ROUND);
    vkvg_rel_move_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_rel_move_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,dx,dy);
    vkvg_rel_move_to(ctx,dx,-dy/2);
    vkvg_rel_line_to(ctx,dx,0);
    vkvg_rel_move_to(ctx,dx,dy/2);
    vkvg_rel_line_to(ctx,dx,-dy);
    vkvg_rel_move_to(ctx,dx,dy);
    vkvg_rel_line_to(ctx,0,-dy);
    vkvg_rel_move_to(ctx,2*dx,dy);
    vkvg_rel_line_to(ctx,-dx,-dy);
    vkvg_rel_move_to(ctx,3*dx,dy/2);
    vkvg_rel_line_to(ctx,-dx,0);
    //vkvg_rel_line_to(ctx,0,-dy);
    //vkvg_rel_move_to(ctx,dx,dy/2);
    //vkvg_rel_line_to(ctx,dx,0);
    vkvg_stroke(ctx);

    vkvg_set_line_cap(ctx,VKVG_LINE_CAP_BUTT);
    vkvg_set_line_width(ctx,1);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_move_to(ctx,x,y);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_rel_move_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_rel_move_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,0,dy);
    vkvg_stroke(ctx);
}

void test_line_join (VkvgContext ctx){
    float x = 50, y = 150, dx = 150, dy = 140;

    //vkvg_scale(ctx,2,2);

    vkvg_set_line_width(ctx,40);
    vkvg_set_source_rgba(ctx,0,0,0,1);


    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_ROUND);
    //vkvg_rectangle(ctx,x,y,dx,dy);

    vkvg_move_to(ctx,x,y);
    vkvg_rel_line_to(ctx,50,-30);
    vkvg_rel_line_to(ctx,50,0);
    vkvg_rel_line_to(ctx,50,30);
    vkvg_rel_line_to(ctx,0,60);
    vkvg_rel_line_to(ctx,-50,70);
    vkvg_rel_line_to(ctx,-50,0);
    vkvg_rel_line_to(ctx,-50,-70);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);

    vkvg_move_to(ctx,x+200,y);
    vkvg_rel_line_to(ctx,50,70);
    vkvg_rel_line_to(ctx,50,0);
    vkvg_rel_line_to(ctx,50,-70);
    vkvg_rel_line_to(ctx,0,-60);
    vkvg_rel_line_to(ctx,-50,-30);
    vkvg_rel_line_to(ctx,-50,0);
    vkvg_rel_line_to(ctx,-50,30);
    vkvg_close_path(ctx);
    vkvg_stroke(ctx);

    vkvg_rel_line_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,dx,dy);
    vkvg_stroke(ctx);
    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_BEVEL);
    vkvg_rel_move_to(ctx,-dx*2,abs(dy*1.5));
    vkvg_rel_line_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,dx,dy);
    vkvg_stroke(ctx);
    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_ROUND);
    vkvg_rel_move_to(ctx,-dx*2,abs(dy*1.5));
    vkvg_rel_line_to(ctx,dx,-dy);
    vkvg_rel_line_to(ctx,dx,dy);
    vkvg_stroke(ctx);
    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_MITER);
}

void multi_test1 () {
    VkvgSurface surf2 = vkvg_surface_create (device,1024,800);;
    VkvgContext ctx = vkvg_create(surf2);

    vkvg_set_source_rgba(ctx,0.01,0.1,0.3,1.0);
    vkvg_paint(ctx);
//    vkvg_set_line_join(ctx,VKVG_LINE_JOIN_ROUND);


    //test_line_join(ctx);

    //vkvg_test_clip(ctx);

    vkvg_set_source_rgba (ctx,0.02,0.8,0.3,1.0);
    vkvg_rectangle (ctx,200,200,300,300);
    vkvg_fill (ctx);

    test_line_caps(ctx);


    test_text(ctx);
    vkvg_test_fill2(ctx);
    //vkvg_test_fill(ctx);

//    vkvg_translate(ctx, 10,10);
//    vkvg_rotate(ctx, 0.2);
    //vkvg_scale(ctx, 2,2);

    //vkvg_test_stroke(ctx);
//    vkvg_test_gradient (ctx);
    vkvg_test_curves(ctx);

    //test_img_surface(ctx);

    vkvg_destroy(ctx);
    ctx = vkvg_create(surf);

    vkvg_set_source_rgba(ctx,0.0,0.0,0.0,1);
    vkvg_paint(ctx);

    vkvg_set_source_surface(ctx, surf2, 0, 0);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
    vkvg_surface_destroy(surf2);
}

void cairo_print_arc (VkvgContext cr) {
    float xc = 128.0;
    float yc = 128.0;
    float radius = 100.0;
    float angle1 = 45.0  * (M_PI/180.0);  /* angles are specified */
    float angle2 = 180.0 * (M_PI/180.0);  /* in radians           */

    vkvg_set_source_rgba(cr, 0, 0, 0, 1);
    vkvg_set_line_width (cr, 10.0);
    vkvg_arc (cr, xc, yc, radius, angle1, angle2);
    vkvg_stroke (cr);

    /* draw helping lines */
    vkvg_set_source_rgba(cr, 1, 0.2, 0.2, 0.6);
    vkvg_set_line_width (cr, 6.0);

    vkvg_arc (cr, xc, yc, 10.0, 0, 2*M_PI);
    vkvg_fill (cr);

    vkvg_arc (cr, xc, yc, radius, angle1, angle1);
    vkvg_line_to (cr, xc, yc);
    vkvg_arc (cr, xc, yc, radius, angle2, angle2);
    //vkvg_line_to (cr, xc, yc);
    vkvg_stroke (cr);
}
void cairo_tests () {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,0.7,0.7,0.7,1);
    vkvg_paint(ctx);
    cairo_print_arc(ctx);
    vkvg_destroy(ctx);
}

void test_grad_transforms () {
    VkvgContext ctx = vkvg_create(surf);

    vkvg_translate(ctx,-100,-100);
    vkvg_rotate(ctx,1.5);
    //vkvg_translate(ctx,100,100);

    //vkvg_scale(ctx,0.2,0.2);
    VkvgPattern pat = vkvg_pattern_create_linear(0,0,200,0);
    vkvg_set_line_width(ctx, 20);
    vkvg_patter_add_color_stop(pat, 0, 1, 0, 0, 1);
    vkvg_patter_add_color_stop(pat, 0.5, 0, 1, 0, 1);
    vkvg_patter_add_color_stop(pat, 1, 0, 0, 1, 1);
    vkvg_set_source (ctx, pat);
    vkvg_rectangle(ctx,0,0,200,200);
    vkvg_fill (ctx);
    //vkvg_stroke (ctx);
    vkvg_pattern_destroy (pat);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
    dumpLayerExts();

    VkEngine e = {};

    EngineInit(&e);

    device = vkvg_device_create(e.phy, e.dev, e.renderer.queue, e.renderer.qFam);

    surf = vkvg_surface_create (device,1024,800);

    vkeCheckPhyPropBlitSource (&e);
    glfwSetKeyCallback(e.renderer.window, key_callback);

    vke_swapchain_create(&e);

    //multi_test1();

    //test_grad_transforms();
    cairo_tests();

    setupSimpleBlit(&e.renderer);

    while (!glfwWindowShouldClose(e.renderer.window)) {
        glfwPollEvents();
        draw(&e);
    }

    vkDeviceWaitIdle(e.dev);
    vke_swapchain_destroy(&e.renderer);

    vkvg_surface_destroy(surf);

    vkvg_device_destroy(device);

    EngineTerminate (&e);

    return 0;
}

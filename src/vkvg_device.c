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

#include "vkvg_device_internal.h"
#include "vkh_queue.h"
#include "vkh_phyinfo.h"
#include "vk_mem_alloc.h"
/**
 * @brief Create VkvgDevice with default multisampling configuration
 * @param Vulkan instance, usefull to retrieve function pointers
 * @param Vulkan physical device
 * @param Vulkan Device
 * @param Queue familly index
 * @param Queue index in selected familly
 * @return
 */
VkvgDevice vkvg_device_create(VkInstance inst, VkPhysicalDevice phy, VkDevice vkdev, uint32_t qFamIdx, uint32_t qIndex)
{
    return vkvg_device_create_multisample (inst,phy,vkdev,qFamIdx,qIndex, VK_SAMPLE_COUNT_4_BIT, false);
}
/**
 * @brief Create VkvgDevice with default multisampling configuration
 * @param Vulkan instance, usefull to retrieve function pointers
 * @param Vulkan physical device
 * @param Vulkan Device
 * @param Queue familly index
 * @param Queue index in selected familly
 * @param multisample count
 * @param When set to false, surface is resolve after each renderpasses on resolve attachment of surface.
 * If set to true, multisample surface image is resolve with vkvg_multisample_surface_resolve. This function
 * is called automatically when surface's VkImage is querried with vkvg_surface_get_vk_image.
 * @return
 */
VkvgDevice vkvg_device_create_multisample(VkInstance inst, VkPhysicalDevice phy, VkDevice vkdev, uint32_t qFamIdx, uint32_t qIndex, VkSampleCountFlags samples, bool deferredResolve)
{
    LOG(LOG_INFO, "CREATE Device: qFam = %d; qIdx = %d\n", qFamIdx, qIndex);

    VkvgDevice dev = (vkvg_device*)malloc(sizeof(vkvg_device));

    dev->instance = inst;
    dev->hdpi   = 72;
    dev->vdpi   = 72;
    dev->samples= samples;
    dev->deferredResolve = deferredResolve;
    dev->vkDev  = vkdev;
    dev->phy    = phy;

    _init_function_pointers (dev);

    VkhPhyInfo phyInfos = vkh_phyinfo_create (dev->phy, NULL);

    dev->phyMemProps = phyInfos->memProps;
    dev->gQueue = vkh_queue_create ((VkhDevice)dev, qFamIdx, qIndex, phyInfos->queues[qFamIdx].queueFlags);
    MUTEX_INIT (&dev->gQMutex);

    vkh_phyinfo_destroy (phyInfos);

    VmaAllocatorCreateInfo allocatorInfo = {
        .physicalDevice = phy,
        .device = vkdev
    };
    vmaCreateAllocator(&allocatorInfo, &dev->allocator);

    dev->lastCtx= NULL;

    dev->cmdPool= vkh_cmd_pool_create       ((VkhDevice)dev, dev->gQueue->familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    dev->cmd    = vkh_cmd_buff_create       ((VkhDevice)dev, dev->cmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    dev->fence  = vkh_fence_create_signaled ((VkhDevice)dev);

    _create_pipeline_cache      (dev);
    _init_fonts_cache           (dev);
    if (dev->deferredResolve)
        _setupRenderPassDeferredResolve(dev);
    else
        _setupRenderPass        (dev);
    _createDescriptorSetLayout  (dev);
    _setupPipelines             (dev);

    _create_empty_texture       (dev);

    dev->references = 1;

    return dev;
}

void vkvg_device_destroy (VkvgDevice dev)
{
    dev->references--;
    if (dev->references > 0)
        return;

    LOG(LOG_INFO, "DESTROY Device\n");

    vkh_image_destroy               (dev->emptyImg);

    vkDestroyDescriptorSetLayout    (dev->vkDev, dev->dslGrad,NULL);
    vkDestroyDescriptorSetLayout    (dev->vkDev, dev->dslFont,NULL);
    vkDestroyDescriptorSetLayout    (dev->vkDev, dev->dslSrc, NULL);

    vkDestroyPipeline               (dev->vkDev, dev->pipelinePolyFill, NULL);
    vkDestroyPipeline               (dev->vkDev, dev->pipelineClipping, NULL);

    vkDestroyPipeline               (dev->vkDev, dev->pipe_OVER,    NULL);
    vkDestroyPipeline               (dev->vkDev, dev->pipe_SUB,     NULL);
    vkDestroyPipeline               (dev->vkDev, dev->pipe_CLEAR,   NULL);

#ifdef VKVG_WIRED_DEBUG
    vkDestroyPipeline               (dev->vkDev, dev->pipelineWired, NULL);
    vkDestroyPipeline               (dev->vkDev, dev->pipelineLineList, NULL);
#endif

    vkDestroyPipelineLayout         (dev->vkDev, dev->pipelineLayout, NULL);
    vkDestroyPipelineCache          (dev->vkDev, dev->pipelineCache, NULL);
    vkDestroyRenderPass             (dev->vkDev, dev->renderPass, NULL);

    vkWaitForFences                 (dev->vkDev, 1, &dev->fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence                  (dev->vkDev, dev->fence,NULL);
    vkFreeCommandBuffers            (dev->vkDev, dev->cmdPool, 1, &dev->cmd);
    vkDestroyCommandPool            (dev->vkDev, dev->cmdPool, NULL);

    _destroy_font_cache(dev);

    vmaDestroyAllocator (dev->allocator);

    MUTEX_DESTROY (&dev->gQMutex);

    free(dev);
}

VkvgDevice vkvg_device_reference (VkvgDevice dev) {
    dev->references++;
    return dev;
}
uint32_t vkvg_device_get_reference_count (VkvgDevice dev) {
    return dev->references;
}

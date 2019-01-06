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

VkvgDevice vkvg_device_create(VkInstance inst, VkPhysicalDevice phy, VkDevice vkdev, uint32_t qFamIdx, uint32_t qIndex)
{
    LOG(LOG_INFO, "CREATE Device: qFam = %d; qIdx = %d\n", qFamIdx, qIndex);

    VkvgDevice dev = (vkvg_device*)malloc(sizeof(vkvg_device));

    dev->instance = inst;
    dev->hdpi   = 72;
    dev->vdpi   = 72;
    dev->vkDev  = vkdev;
    dev->phy    = phy;

    CmdBindPipeline         = vkGetInstanceProcAddr(inst, "vkCmdBindPipeline");
    CmdBindDescriptorSets   = vkGetInstanceProcAddr(inst, "vkCmdBindDescriptorSets");
    CmdBindIndexBuffer      = vkGetInstanceProcAddr(inst, "vkCmdBindIndexBuffer");
    CmdBindVertexBuffers    = vkGetInstanceProcAddr(inst, "vkCmdBindVertexBuffers");
    CmdDrawIndexed          = vkGetInstanceProcAddr(inst, "vkCmdDrawIndexed");
    CmdDraw                 = vkGetInstanceProcAddr(inst, "vkCmdDraw");
    CmdSetStencilCompareMask= vkGetInstanceProcAddr(inst, "vkCmdSetStencilCompareMask");
    CmdBeginRenderPass      = vkGetInstanceProcAddr(inst, "vkCmdBeginRenderPass");
    CmdEndRenderPass        = vkGetInstanceProcAddr(inst, "vkCmdEndRenderPass");
    CmdSetViewport          = vkGetInstanceProcAddr(inst, "vkCmdSetViewport");
    CmdSetScissor           = vkGetInstanceProcAddr(inst, "vkCmdSetScissor");
    CmdPushConstants        = vkGetInstanceProcAddr(inst, "vkCmdPushConstants");
    CmdPushDescriptorSet    = vkGetInstanceProcAddr(inst, "vkCmdDescriptorSet");

    VkhPhyInfo phyInfos = vkh_phyinfo_create (dev->phy, NULL);

    dev->phyMemProps = phyInfos->memProps;
    dev->gQueue = vkh_queue_create (dev, qFamIdx, qIndex, phyInfos->queues[qFamIdx].queueFlags);
    MUTEX_INIT (&dev->gQMutex);

    vkh_phyinfo_destroy (phyInfos);

    VmaAllocatorCreateInfo allocatorInfo = {
        .physicalDevice = phy,
        .device = vkdev
    };
    vmaCreateAllocator(&allocatorInfo, &dev->allocator);

    dev->lastCtx= NULL;

    dev->cmdPool= vkh_cmd_pool_create       (dev, dev->gQueue->familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    dev->cmd    = vkh_cmd_buff_create       (dev, dev->cmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    dev->fence  = vkh_fence_create_signaled (dev);

    _create_pipeline_cache      (dev);
    _init_fonts_cache           (dev);
    _setupRenderPass            (dev);
    _createDescriptorSetLayout  (dev);
    _setupPipelines             (dev);

    //create empty image to bind to source descriptor when not in use
    dev->emptyImg = vkh_image_create(dev,FB_COLOR_FORMAT,16,16,VKVG_TILING,VMA_MEMORY_USAGE_GPU_ONLY,
                                     VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkh_image_create_descriptor(dev->emptyImg, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

    _wait_and_reset_device_fence (dev);

    vkh_cmd_begin (dev->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkh_image_set_layout (dev->cmd, dev->emptyImg, VK_IMAGE_ASPECT_COLOR_BIT,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    vkh_cmd_end (dev->cmd);
    _submit_cmd (dev, &dev->cmd, dev->fence);

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

#if DEBUG
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

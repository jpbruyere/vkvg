﻿#include "vkvg_device_internal.h"

VkvgDevice vkvg_device_create(VkPhysicalDevice phy, VkDevice vkdev, VkQueue queue, uint32_t qFam)
{
    VkvgDevice dev = (vkvg_device*)malloc(sizeof(vkvg_device));

    dev->hdpi = 96;
    dev->vdpi = 96;

    dev->vkDev = vkdev;
    vkGetPhysicalDeviceMemoryProperties (phy, &dev->phyMemProps);

    for (int i=0; i<120; i++){
        VkFormatProperties formatProps = {};
        vkGetPhysicalDeviceFormatProperties (phy, (VkFormat)i, &formatProps);
        printf("%d => Buff = %d, Lin = %d, Opt = %d\n", i, formatProps.bufferFeatures, formatProps.linearTilingFeatures, formatProps.optimalTilingFeatures);
    }

    dev->queue  = queue;
    dev->qFam   = qFam;
    dev->lastCtx= NULL;

    dev->cmdPool= vkh_cmd_pool_create       (dev->vkDev, qFam, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    dev->cmd    = vkh_cmd_buff_create       (dev->vkDev, dev->cmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    dev->fence  = vkh_fence_create_signaled (dev->vkDev);

    _create_pipeline_cache      (dev);
    _init_fonts_cache           (dev);
    _setupRenderPass            (dev);
    _createDescriptorSetLayout  (dev);
    _setupPipelines             (dev);

    return dev;
}

void vkvg_device_destroy(VkvgDevice dev)
{
    vkDestroyDescriptorSetLayout    (dev->vkDev, dev->dslFont,NULL);
    vkDestroyDescriptorSetLayout    (dev->vkDev, dev->dslSrc, NULL);

    vkDestroyPipeline               (dev->vkDev, dev->pipeline, NULL);
    vkDestroyPipeline               (dev->vkDev, dev->pipelineClipping, NULL);
    vkDestroyPipeline               (dev->vkDev, dev->pipeline_OP_SUB, NULL);
    vkDestroyPipeline               (dev->vkDev, dev->pipelineWired, NULL);
    vkDestroyPipeline               (dev->vkDev, dev->pipelineLineList, NULL);

    vkDestroyPipelineLayout         (dev->vkDev, dev->pipelineLayout, NULL);
    vkDestroyPipelineCache          (dev->vkDev, dev->pipelineCache, NULL);
    vkDestroyRenderPass             (dev->vkDev, dev->renderPass, NULL);

    vkWaitForFences                 (dev->vkDev, 1, &dev->fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence                  (dev->vkDev, dev->fence,NULL);
    vkFreeCommandBuffers            (dev->vkDev, dev->cmdPool, 1, &dev->cmd);
    vkDestroyCommandPool            (dev->vkDev, dev->cmdPool, NULL);

    _destroy_font_cache(dev);
    free(dev);
}

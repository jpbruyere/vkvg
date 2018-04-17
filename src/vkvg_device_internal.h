#ifndef VKVG_DEVICE_INTERNAL_H
#define VKVG_DEVICE_INTERNAL_H

#include "vkvg_internal.h"
#include "vkvg.h"
#include "vkvg_fonts.h"

typedef struct _grad_compute_t {
    int width;
    int height;
    int work_size;

    VkPipeline          pipeline;
    VkPipelineLayout    pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    VkhImage            output;
    //ubo_t*  ubo;
}grad_compute_t;

typedef struct _vkvg_device_t{
    VkDevice				vkDev;
    VkPhysicalDeviceMemoryProperties phyMemProps;
    VkRenderPass			renderPass;
    VkPhysicalDevice        phy;

    VkhQueue                gQueue;
    VkhQueue                cQueue;

    VkCommandPool			cmdPool;
    VkCommandBuffer         cmd;
    VkFence                 fence;

    VkPipeline				pipeline;
    VkPipeline				pipelineClipping;
    VkPipeline				pipeline_OP_SUB;
    VkPipeline				pipelineWired;
    VkPipeline				pipelineLineList;

    VkPipelineCache			pipelineCache;
    VkPipelineLayout		pipelineLayout;
    VkDescriptorPool		descriptorPool;
    VkDescriptorSetLayout	dslFont;
    VkDescriptorSetLayout	dslSrc;

    grad_compute_t          gradientPipeline;
    int		hdpi,
            vdpi;

    _font_cache_t*	fontCache;
    VkvgContext     lastCtx;    //double linked list last elmt
}vkvg_device;

void _create_pipeline_cache     (VkvgDevice dev);
void _setupRenderPass           (VkvgDevice dev);
void _setupPipelines            (VkvgDevice dev);
void _createDescriptorSetLayout (VkvgDevice dev);
void _flush_all_contexes        (VkvgDevice dev);
void _init_all_contexes         (VkvgDevice dev);
void _wait_device_fence         (VkvgDevice dev);
#endif

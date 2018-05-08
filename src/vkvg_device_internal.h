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
#ifndef VKVG_DEVICE_INTERNAL_H
#define VKVG_DEVICE_INTERNAL_H

#include "vkvg_internal.h"
#include "vkvg.h"
#include "vkvg_fonts.h"

#define STENCIL_FILL_BIT    0x1
#define STENCIL_CLIP_BIT    0x2
#define STENCIL_ALL_BIT     0x3

typedef struct _vkvg_device_t{
    VkDevice				vkDev;
    VkPhysicalDeviceMemoryProperties phyMemProps;
    VkPhysicalDevice        phy;
    VmaAllocator            allocator;

    VkhQueue                gQueue;
    VkRenderPass			renderPass;

    uint32_t                references;
    VkCommandPool			cmdPool;
    VkCommandBuffer         cmd;
    VkFence                 fence;

    VkPipeline				pipe_OVER;
    VkPipeline				pipe_SUB;
    VkPipeline				pipe_CLEAR;

    VkPipeline				pipelinePolyFill;
    VkPipeline				pipelineClipping;
    VkPipeline				pipelineWired;
    VkPipeline				pipelineLineList;

    VkPipelineCache			pipelineCache;
    VkPipelineLayout		pipelineLayout;
    VkDescriptorPool		descriptorPool;
    VkDescriptorSetLayout	dslFont;
    VkDescriptorSetLayout	dslSrc;
    VkDescriptorSetLayout	dslGrad;

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

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

PFN_vkCmdBindPipeline           CmdBindPipeline;
PFN_vkCmdBindDescriptorSets     CmdBindDescriptorSets;
PFN_vkCmdBindIndexBuffer        CmdBindIndexBuffer;
PFN_vkCmdBindVertexBuffers      CmdBindVertexBuffers;

PFN_vkCmdDrawIndexed    CmdDrawIndexed;
PFN_vkCmdDraw           CmdDraw;

PFN_vkCmdSetStencilCompareMask  CmdSetStencilCompareMask;
PFN_vkCmdBeginRenderPass        CmdBeginRenderPass;
PFN_vkCmdEndRenderPass          CmdEndRenderPass;
PFN_vkCmdSetViewport            CmdSetViewport;
PFN_vkCmdSetScissor             CmdSetScissor;

PFN_vkCmdPushConstants          CmdPushConstants;
PFN_vkCmdPushDescriptorSetKHR   CmdPushDescriptorSet;


typedef struct _vkvg_device_t{
    VkDevice				vkDev;
    VkPhysicalDeviceMemoryProperties phyMemProps;
    VkPhysicalDevice        phy;
    VmaAllocator            allocator;

    VkhQueue                gQueue;
    MUTEX                   gQMutex;//queue submission has to be externally syncronized
    VkRenderPass			renderPass;

    uint32_t                references;
    VkCommandPool			cmdPool;
    VkCommandBuffer         cmd;
    //this fence is kept signaled when idle, wait and reset are called before each recording.
    VkFence                 fence;

    VkPipeline				pipe_OVER;  //default operator
    VkPipeline				pipe_SUB;
    VkPipeline				pipe_CLEAR; //clear operator

    VkPipeline				pipelinePolyFill;   //
    VkPipeline				pipelineClipping;   //to update clip

#if DEBUG
    VkPipeline				pipelineWired;
    VkPipeline				pipelineLineList;
#endif

    VkPipelineCache			pipelineCache;
    VkPipelineLayout		pipelineLayout;
    VkDescriptorPool		descriptorPool;
    VkDescriptorSetLayout	dslFont;
    VkDescriptorSetLayout	dslSrc;
    VkDescriptorSetLayout	dslGrad;

    int		hdpi,
            vdpi;
    VkInstance              instance;

    VkhImage                emptyImg;//prevent unbound descriptor to trigger Validation error 61
    VkSampleCountFlags      samples;//samples count for all surfaces

    _font_cache_t*	fontCache;
    VkvgContext     lastCtx;    //double linked list last elmt
}vkvg_device;

void _init_function_pointers    (VkvgDevice dev);
void _create_empty_texture      (VkvgDevice dev);
void _check_image_format_properties (VkvgDevice dev);
void _create_pipeline_cache     (VkvgDevice dev);
void _setupRenderPass           (VkvgDevice dev);
void _setupPipelines            (VkvgDevice dev);
void _createDescriptorSetLayout (VkvgDevice dev);
void _flush_all_contexes        (VkvgDevice dev);
void _init_all_contexes         (VkvgDevice dev);
void _wait_and_reset_device_fence (VkvgDevice dev);
void _submit_cmd                (VkvgDevice dev, VkCommandBuffer* cmd, VkFence fence);
#endif

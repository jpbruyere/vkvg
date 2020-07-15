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
PFN_vkCmdSetStencilReference    CmdSetStencilReference;
PFN_vkCmdSetStencilWriteMask    CmdSetStencilWriteMask;
PFN_vkCmdBeginRenderPass        CmdBeginRenderPass;
PFN_vkCmdEndRenderPass          CmdEndRenderPass;
PFN_vkCmdSetViewport            CmdSetViewport;
PFN_vkCmdSetScissor             CmdSetScissor;

PFN_vkCmdPushConstants          CmdPushConstants;
PFN_vkCmdPushDescriptorSetKHR   CmdPushDescriptorSet;


typedef struct _vkvg_device_t{
	VkDevice				vkDev;                  /**< Vulkan Logical Device */
	VkPhysicalDeviceMemoryProperties phyMemProps;   /**< Vulkan Physical device memory properties */
	VkPhysicalDevice        phy;                    /**< Vulkan Physical device */
	VmaAllocator            allocator;              /**< Vulkan Memory allocator */
	VkInstance              instance;               /**< Vulkan instance */

	VkhQueue                gQueue;                 /**< Vulkan Queue with Graphic flag */
	MUTEX                   gQMutex;                /**< queue submission has to be externally syncronized */
	VkRenderPass			renderPass;             /**< Vulkan render pass, common for all surfaces */
	VkRenderPass			renderPass_ClearStencil;/**< Vulkan render pass for first draw with context, stencil has to be cleared */
	VkRenderPass			renderPass_ClearAll;    /**< Vulkan render pass for new surface, clear all attacments*/

	uint32_t                references;             /**< Reference count, prevent destroying device if still in use */
	VkCommandPool			cmdPool;                /**< Global command pool for processing on surfaces without context */
	VkCommandBuffer         cmd;                    /**< Global command buffer */
	VkFence                 fence;                  /**< this fence is kept signaled when idle, wait and reset are called before each recording. */

	VkPipeline				pipe_OVER;              /**< default operator */
	VkPipeline				pipe_SUB;
	VkPipeline				pipe_CLEAR;             /**< clear operator */

	VkPipeline				pipelinePolyFill;       /**< even-odd polygon filling first step */
	VkPipeline				pipelineClipping;       /**< draw on stencil to update clipping regions */

#ifdef VKVG_WIRED_DEBUG
	VkPipeline				pipelineWired;
	VkPipeline				pipelineLineList;
#endif
	VkPipelineCache			pipelineCache;          /**< speed up startup by caching configured pipelines on disk */
	VkPipelineLayout		pipelineLayout;         /**< layout common to all pipelines */
	//VkDescriptorSetLayout	dslFont;                /**< font cache descriptors layout */
	VkDescriptorSetLayout	dslSrc;                 /**< context source surface descriptors layout */
	VkDescriptorSetLayout	dslGrad;                /**< context gradient descriptors layout */

	int		hdpi,                                   /**< only used for FreeType fonts and svg loading */
			vdpi;

	VkhImage                emptyImg;               /**< prevent unbound descriptor to trigger Validation error 61 */
	VkSampleCountFlags      samples;                /**< samples count common to all surfaces */
	bool                    deferredResolve;        /**< if true, resolve only on context destruction and set as source */
	vkvg_status_t           status;                 /**< Current status of device, affected by last operation */

	_font_cache_t*	fontCache;                      /**< Store everything relative to common font caching system */
	VkvgContext     lastCtx;                        /**< last element of double linked list of context, used to trigger font caching system update on all contexts*/
}vkvg_device;

void _init_function_pointers    (VkvgDevice dev);
void _create_empty_texture      (VkvgDevice dev, VkFormat format, VkImageTiling tiling);
bool _get_best_image_tiling		(VkvgDevice dev, VkFormat format, VkImageTiling* pTiling);
void _create_pipeline_cache     (VkvgDevice dev);
VkRenderPass _createRenderPassMS(VkvgDevice dev, VkAttachmentLoadOp loadOp, VkAttachmentLoadOp stencilLoadOp);
VkRenderPass _createRenderPassNoResolve(VkvgDevice dev, VkAttachmentLoadOp loadOp, VkAttachmentLoadOp stencilLoadOp);
void _setupPipelines            (VkvgDevice dev);
void _createDescriptorSetLayout (VkvgDevice dev);
void _flush_all_contexes        (VkvgDevice dev);
void _wait_idle                 (VkvgDevice dev);
void _wait_and_reset_device_fence (VkvgDevice dev);
void _submit_cmd                (VkvgDevice dev, VkCommandBuffer* cmd, VkFence fence);
#endif

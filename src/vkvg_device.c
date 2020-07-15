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
	return vkvg_device_create_multisample (inst,phy,vkdev,qFamIdx,qIndex, VK_SAMPLE_COUNT_1_BIT, false);
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
	dev->hdpi   = 96;
	dev->vdpi   = 96;
	dev->samples= samples;
	dev->deferredResolve = deferredResolve;
	dev->vkDev  = vkdev;
	dev->phy    = phy;

	_init_function_pointers (dev);

	VkhPhyInfo phyInfos = vkh_phyinfo_create (dev->phy, NULL);

	dev->phyMemProps = phyInfos->memProps;
	dev->gQueue = vkh_queue_create ((VkhDevice)dev, qFamIdx, qIndex);
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
	if (dev->deferredResolve || dev->samples == VK_SAMPLE_COUNT_1_BIT){
		dev->renderPass = _createRenderPassNoResolve (dev, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_LOAD);
		dev->renderPass_ClearStencil = _createRenderPassNoResolve (dev, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_CLEAR);
		dev->renderPass_ClearAll = _createRenderPassNoResolve (dev, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_CLEAR);
	}else{
		dev->renderPass = _createRenderPassMS (dev, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_LOAD);
		dev->renderPass_ClearStencil = _createRenderPassMS (dev, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_CLEAR);
		dev->renderPass_ClearAll = _createRenderPassMS (dev, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_CLEAR);
	}
	_createDescriptorSetLayout  (dev);
	_setupPipelines             (dev);

	_create_empty_texture       (dev);

	memset(dev->samplers, 0, 10 * sizeof (VkSampler));

	dev->references = 1;

#ifdef DEBUG
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)dev->cmdPool, "Device Cmd Pool");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)dev->cmd, "Device Cmd Buff");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_FENCE, (uint64_t)dev->fence, "Device Fence");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)dev->renderPass, "RP load img/stencil");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)dev->renderPass_ClearStencil, "RP clear stencil");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)dev->renderPass_ClearAll, "RP clear all");

	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)dev->dslSrc, "DSLayout SOURCE");
	//vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)dev->dslFont, "DSLayout FONT");
	//vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)dev->dslGrad, "DSLayout GRADIENT");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_PIPELINE_LAYOUT, (uint64_t)dev->pipelineLayout, "PLLayout dev");

	//vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_PIPELINE, (uint64_t)dev->pipelinePolyFill, "PL Poly fill");
	//vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_PIPELINE, (uint64_t)dev->pipelineClipping, "PL Clipping");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_PIPELINE, (uint64_t)dev->pipe_OVER, "PL draw Over");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_PIPELINE, (uint64_t)dev->pipe_SUB, "PL draw Substract");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_PIPELINE, (uint64_t)dev->pipe_CLEAR, "PL draw Clear");

	vkh_image_set_name(dev->emptyImg, "empty IMG");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)vkh_image_get_view(dev->emptyImg), "empty IMG VIEW");
	//vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_SAMPLER, (uint64_t)vkh_image_get_sampler(dev->emptyImg), "empty IMG SAMPLER");
#endif

	return dev;
}

void vkvg_device_destroy (VkvgDevice dev)
{
	dev->references--;
	if (dev->references > 0)
		return;

	LOG(LOG_INFO, "DESTROY Device\n");

	vkh_image_destroy               (dev->emptyImg);

	_destroy_samplers               (dev);
	//vkDestroyDescriptorSetLayout    (dev->vkDev, dev->dslGrad,NULL);
	//vkDestroyDescriptorSetLayout    (dev->vkDev, dev->dslFont,NULL);
	vkDestroyDescriptorSetLayout    (dev->vkDev, dev->dslSrc, NULL);

	//vkDestroyPipeline               (dev->vkDev, dev->pipelinePolyFill, NULL);
	//vkDestroyPipeline               (dev->vkDev, dev->pipelineClipping, NULL);

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
	vkDestroyRenderPass             (dev->vkDev, dev->renderPass_ClearStencil, NULL);
	vkDestroyRenderPass             (dev->vkDev, dev->renderPass_ClearAll, NULL);

	vkWaitForFences                 (dev->vkDev, 1, &dev->fence, VK_TRUE, UINT64_MAX);

	vkDestroyFence                  (dev->vkDev, dev->fence,NULL);
	vkFreeCommandBuffers            (dev->vkDev, dev->cmdPool, 1, &dev->cmd);
	vkDestroyCommandPool            (dev->vkDev, dev->cmdPool, NULL);

	_destroy_font_cache             (dev);

	vmaDestroyAllocator             (dev->allocator);

	MUTEX_DESTROY (&dev->gQMutex);

	vkh_queue_destroy               (dev->gQueue);

	free(dev);
}

VkvgDevice vkvg_device_reference (VkvgDevice dev) {
	dev->references++;
	return dev;
}
uint32_t vkvg_device_get_reference_count (VkvgDevice dev) {
	return dev->references;
}
/**
 * @brief set horizontal and vertical resolution of device in dot per inch
 * @param vkvg device pointer
 * @param horizontal device resolution in dot per inch
 * @param vertical device resolution in dot per inch
 */
void vkvg_device_set_dpy (VkvgDevice dev, int hdpy, int vdpy) {
	dev->hdpi = hdpy;
	dev->vdpi = vdpy;

	//TODO: reset font cache
}
/**
 * @brief get horizontal and vertical resolution of device in dot per inch
 * @param vkvg device pointer
 * @param return horizontal device resolution in dot per inch
 * @param return vertical device resolution in dot per inch
 */
void vkvg_device_get_dpy (VkvgDevice dev, int* hdpy, int* vdpy) {
	*hdpy = dev->hdpi;
	*vdpy = dev->vdpi;
}

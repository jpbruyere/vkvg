/*
 * Copyright (c) 2018-2020 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
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

#include "vkvg_surface_internal.h"
#include "vkvg_device_internal.h"
#include "vkh_image.h"

void _explicit_ms_resolve (VkvgSurface surf){
	VkvgDevice		dev = surf->dev;
	VkCommandBuffer cmd = dev->cmd;

	_wait_and_reset_device_fence (dev);

	vkh_cmd_begin (cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	vkh_image_set_layout (cmd, surf->imgMS, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	vkh_image_set_layout (cmd, surf->img, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkImageResolve re = {
		.extent = {surf->width, surf->height,1},
		.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1},
		.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1}
	};

	vkCmdResolveImage(cmd,
					  vkh_image_get_vkimage (surf->imgMS), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					  vkh_image_get_vkimage (surf->img) ,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					  1,&re);
	vkh_image_set_layout (cmd, surf->imgMS, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ,
						  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	vkh_cmd_end (cmd);

	_submit_cmd (dev, &cmd, dev->fence);
}

void _clear_surface (VkvgSurface surf, VkImageAspectFlags aspect)
{
	VkvgDevice		dev = surf->dev;
	VkCommandBuffer cmd = dev->cmd;

	_wait_and_reset_device_fence (dev);

	vkh_cmd_begin (cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	if (aspect & VK_IMAGE_ASPECT_COLOR_BIT) {
		VkClearColorValue cclr = {{0,0,0,0}};
		VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};

		VkhImage img = surf->imgMS;
		if (surf->dev->samples == VK_SAMPLE_COUNT_1_BIT)
			img = surf->img;

		vkh_image_set_layout (cmd, img, VK_IMAGE_ASPECT_COLOR_BIT,
							  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		vkCmdClearColorImage(cmd, vkh_image_get_vkimage (img),
									 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &cclr, 1, &range);

		vkh_image_set_layout (cmd, img, VK_IMAGE_ASPECT_COLOR_BIT,
							  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	}
	if (aspect & VK_IMAGE_ASPECT_STENCIL_BIT) {
		VkClearDepthStencilValue clr = {0,0};
		VkImageSubresourceRange range = {VK_IMAGE_ASPECT_STENCIL_BIT,0,1,0,1};

		vkh_image_set_layout (cmd, surf->stencil, VK_IMAGE_ASPECT_STENCIL_BIT,
							  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		vkCmdClearDepthStencilImage (cmd, vkh_image_get_vkimage (surf->stencil),
									 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,&clr,1,&range);

		vkh_image_set_layout (cmd, surf->stencil, VK_IMAGE_ASPECT_STENCIL_BIT,
							  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
							  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
	}
	vkh_cmd_end (cmd);

	_submit_cmd (dev, &cmd, dev->fence);
}

void _create_surface_main_image (VkvgSurface surf){
	surf->img = vkh_image_create((VkhDevice)surf->dev,surf->format,surf->width,surf->height,surf->dev->supportedTiling,VMA_MEMORY_USAGE_GPU_ONLY,
									 VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	vkh_image_create_descriptor(surf->img, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_image_set_name(surf->img, "SURF main color");
	vkh_device_set_object_name((VkhDevice)surf->dev, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)vkh_image_get_view(surf->img), "SURF main color VIEW");
	vkh_device_set_object_name((VkhDevice)surf->dev, VK_OBJECT_TYPE_SAMPLER, (uint64_t)vkh_image_get_sampler(surf->img), "SURF main color SAMPLER");
#endif
}
//create multisample color img if sample count > 1 and the stencil buffer multisampled or not
void _create_surface_secondary_images (VkvgSurface surf) {
	if (surf->dev->samples > VK_SAMPLE_COUNT_1_BIT){
		surf->imgMS = vkh_image_ms_create((VkhDevice)surf->dev,surf->format,surf->dev->samples,surf->width,surf->height,VMA_MEMORY_USAGE_GPU_ONLY,
										  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		vkh_image_create_descriptor(surf->imgMS, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_NEAREST,
									VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
		vkh_image_set_name(surf->imgMS, "SURF MS color IMG");
		vkh_device_set_object_name((VkhDevice)surf->dev, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)vkh_image_get_view(surf->imgMS), "SURF MS color VIEW");
		vkh_device_set_object_name((VkhDevice)surf->dev, VK_OBJECT_TYPE_SAMPLER, (uint64_t)vkh_image_get_sampler(surf->imgMS), "SURF MS color SAMPLER");
#endif
	}
	surf->stencil = vkh_image_ms_create((VkhDevice)surf->dev,surf->dev->stencilFormat,surf->dev->samples,surf->width,surf->height,VMA_MEMORY_USAGE_GPU_ONLY,									 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	vkh_image_create_descriptor(surf->stencil, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_STENCIL_BIT, VK_FILTER_NEAREST,
								VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_image_set_name(surf->stencil, "SURF stencil");
	vkh_device_set_object_name((VkhDevice)surf->dev, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)vkh_image_get_view(surf->stencil), "SURF stencil VIEW");
	vkh_device_set_object_name((VkhDevice)surf->dev, VK_OBJECT_TYPE_SAMPLER, (uint64_t)vkh_image_get_sampler(surf->stencil), "SURF stencil SAMPLER");
#endif
}
void _create_framebuffer (VkvgSurface surf) {
	VkImageView attachments[] = {
		vkh_image_get_view (surf->img),
		vkh_image_get_view (surf->stencil),
		vkh_image_get_view (surf->imgMS),
	};
	VkFramebufferCreateInfo frameBufferCreateInfo = { .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
													  .renderPass = surf->dev->renderPass,
													  .attachmentCount = 3,
													  .pAttachments = attachments,
													  .width = surf->width,
													  .height = surf->height,
													  .layers = 1 };
	if (surf->dev->samples == VK_SAMPLE_COUNT_1_BIT)
		frameBufferCreateInfo.attachmentCount = 2;
	else if (surf->dev->deferredResolve) {
		attachments[0] = attachments[2];
		frameBufferCreateInfo.attachmentCount = 2;
	}
	VK_CHECK_RESULT(vkCreateFramebuffer(surf->dev->vkDev, &frameBufferCreateInfo, NULL, &surf->fb));
#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_device_set_object_name((VkhDevice)surf->dev, VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)surf->fb, "SURF FB");
#endif
}
void _create_surface_images (VkvgSurface surf) {

	_create_surface_main_image		(surf);
	_create_surface_secondary_images(surf);
	_create_framebuffer				(surf);

	//_clear_surface					(surf, VK_IMAGE_ASPECT_STENCIL_BIT);
#if defined(DEBUG) && defined(ENABLE_VALIDATION)
	vkh_image_set_name(surf->img, "surfImg");
	vkh_image_set_name(surf->imgMS, "surfImgMS");
	vkh_image_set_name(surf->stencil, "surfStencil");
#endif
}
VkvgSurface _create_surface (VkvgDevice dev, VkFormat format) {	
	VkvgSurface surf = (vkvg_surface*)calloc(1,sizeof(vkvg_surface));
	if (!surf) {
		dev->status = VKVG_STATUS_NO_MEMORY;
		return NULL;
	}
	surf->references = 1;
	if (dev->status != VKVG_STATUS_SUCCESS) {
		surf->status = VKVG_STATUS_DEVICE_ERROR;
		return surf;
	}
	surf->dev = dev;
	surf->format = format;
	return surf;
}

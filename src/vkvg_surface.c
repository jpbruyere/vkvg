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
#include "vkvg_context_internal.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "vkh_image.h"

#define max(x,y)
void _transition_surf_images (VkvgSurface surf) {
	LOCK_SURFACE(surf)
	VkvgDevice dev = surf->dev;

	//_surface_wait_cmd (surf);

	vkh_cmd_begin (surf->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VkhImage imgMs = surf->imgMS;
	if (imgMs != NULL)
		vkh_image_set_layout(surf->cmd, imgMs, VK_IMAGE_ASPECT_COLOR_BIT,
							 VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

	vkh_image_set_layout(surf->cmd, surf->img, VK_IMAGE_ASPECT_COLOR_BIT,
					 VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	vkh_image_set_layout (surf->cmd, surf->stencil, dev->stencilAspectFlag,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
						  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
	vkh_cmd_end (surf->cmd);

	_surface_submit_cmd (surf);

	UNLOCK_SURFACE(surf)
}
void vkvg_surface_clear (VkvgSurface surf) {
	if (surf->status)
		return;
	_clear_surface(surf, VK_IMAGE_ASPECT_STENCIL_BIT|VK_IMAGE_ASPECT_COLOR_BIT);
}
VkvgSurface vkvg_surface_create (VkvgDevice dev, uint32_t width, uint32_t height){
	VkvgSurface surf = _create_surface(dev, FB_COLOR_FORMAT);
	if (surf->status)
		return surf;

	surf->imgWidth = surf->width = MAX(1, width);
	surf->imgHeight = surf->height = MAX(1, height);
	surf->newSurf = true;//used to clear all attacments on first render pass

	_create_surface_images (surf);

	_transition_surf_images (surf);

	surf->status = VKVG_STATUS_SUCCESS;
	vkvg_device_reference (surf->dev);
	return surf;
}
VkvgSurface vkvg_surface_create_for_VkhImage (VkvgDevice dev, void* vkhImg) {
	VkvgSurface surf = _create_surface(dev, FB_COLOR_FORMAT);
	if (surf->status)
		return surf;

	if (!vkhImg) {
		surf->status = VKVG_STATUS_INVALID_IMAGE;
		return surf;
	}

	VkhImage img = (VkhImage)vkhImg;
	surf->imgWidth = img->infos.extent.width;
	surf->imgHeight = img->infos.extent.height;

	surf->img = img;

	vkh_image_create_sampler(img, VK_FILTER_NEAREST, VK_FILTER_NEAREST,
							 VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

	_create_surface_secondary_images	(surf);
	_create_framebuffer					(surf);

	_transition_surf_images (surf);
	//_clear_surface						(surf, VK_IMAGE_ASPECT_STENCIL_BIT);

	surf->status = VKVG_STATUS_SUCCESS;
	vkvg_device_reference (surf->dev);
	return surf;
}
//TODO: it would be better to blit in original size and create ms final image with dest surf dims
VkvgSurface vkvg_surface_create_from_bitmap (VkvgDevice dev, unsigned char* img, uint32_t width, uint32_t height) {
	VkvgSurface surf = _create_surface(dev, FB_COLOR_FORMAT);
	if (surf->status)
		return surf;
	if (!img || width <= 0 || height <= 0) {
		surf->status = VKVG_STATUS_INVALID_IMAGE;
		return surf;
	}

	surf->imgWidth = surf->width = MAX(1, width);
	surf->imgHeight = surf->height = MAX(1, height);

	_create_surface_images (surf);

	uint32_t imgSize = width * height * 4;
	VkImageSubresourceLayers imgSubResLayers = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};
	//original format image
	VkhImage stagImg= vkh_image_create ((VkhDevice)surf->dev,VK_FORMAT_R8G8B8A8_UNORM,surf->width,surf->height,VK_IMAGE_TILING_LINEAR,
										 VKH_MEMORY_USAGE_GPU_ONLY,
										 VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	//bgra bliting target
	VkhImage tmpImg = vkh_image_create ((VkhDevice)surf->dev,surf->format,surf->width,surf->height,VK_IMAGE_TILING_LINEAR,
										 VKH_MEMORY_USAGE_GPU_ONLY,
										 VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	vkh_image_create_descriptor (tmpImg, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT,
								 VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
	//staging buffer
	vkh_buffer_t buff = {0};
	vkh_buffer_init((VkhDevice)dev,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VKH_MEMORY_USAGE_CPU_TO_GPU,
					imgSize, &buff, true);

	memcpy (vkh_buffer_get_mapped_pointer (&buff), img, imgSize);

	VkCommandBuffer cmd = surf->cmd;

	vkh_cmd_begin (cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	vkh_image_set_layout (cmd, stagImg, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);


	VkBufferImageCopy bufferCopyRegion = { .imageSubresource = imgSubResLayers,
										   .imageExtent = {surf->width,surf->height,1}};

	vkCmdCopyBufferToImage(cmd, buff.buffer,
		vkh_image_get_vkimage (stagImg), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

	vkh_image_set_layout (cmd, stagImg, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	vkh_image_set_layout (cmd, tmpImg, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkImageBlit blit = {
		.srcSubresource = imgSubResLayers,
		.srcOffsets[1] = {(int32_t)surf->width, (int32_t)surf->height, 1},
		.dstSubresource = imgSubResLayers,
		.dstOffsets[1] = {(int32_t)surf->width, (int32_t)surf->height, 1},
	};
	vkCmdBlitImage	(cmd,
					 vkh_image_get_vkimage (stagImg), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					 vkh_image_get_vkimage (tmpImg),  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

	vkh_image_set_layout (cmd, tmpImg, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	vkh_cmd_end		(cmd);

	_surface_submit_cmd (surf);//lock surface?

	vkh_buffer_reset	(&buff);
	vkh_image_destroy	(stagImg);

	surf->newSurf = false;

	//create tmp context with rendering pipeline to create the multisample img
	VkvgContext ctx = vkvg_create (surf);

/*	  VkClearAttachment ca = {VK_IMAGE_ASPECT_COLOR_BIT,0, { 0.0f, 0.0f, 0.0f, 0.0f }};
	VkClearRect cr = {{{0,0},{surf->width,surf->height}},0,1};
	vkCmdClearAttachments(ctx->cmd, 1, &ca, 1, &cr);*/

	vec4 srcRect = {.x=0,.y=0,.width=(float)surf->width,.height=(float)surf->height};
	ctx->pushConsts.source = srcRect;
	ctx->pushConsts.fsq_patternType = (ctx->pushConsts.fsq_patternType & FULLSCREEN_BIT) + VKVG_PATTERN_TYPE_SURFACE;

	//_update_push_constants (ctx);
	_update_descriptor_set (ctx, tmpImg, ctx->dsSrc);
	_ensure_renderpass_is_started  (ctx);

	vkvg_paint			(ctx);
	vkvg_destroy		(ctx);

	vkh_image_destroy	(tmpImg);

	surf->status = VKVG_STATUS_SUCCESS;
	vkvg_device_reference (surf->dev);
	return surf;
}
VkvgSurface vkvg_surface_create_from_image (VkvgDevice dev, const char* filePath) {
	int w = 0,
		h = 0,
		channels = 0;
	unsigned char *img = stbi_load(filePath, &w, &h, &channels, 4);//force 4 components per pixel
	if (!img){
		LOG(VKVG_LOG_ERR, "Could not load texture from %s, %s\n", filePath, stbi_failure_reason());
		return (VkvgSurface)&_no_mem_status;
	}

	VkvgSurface surf = vkvg_surface_create_from_bitmap(dev, img, (uint32_t)w, (uint32_t)h);

	stbi_image_free (img);

	return surf;
}

void vkvg_surface_destroy(VkvgSurface surf)
{
	if (surf->status)
		return;

	LOCK_SURFACE(surf)
	surf->references--;
	if (surf->references > 0) {
		UNLOCK_SURFACE(surf)
		return;
	}
	UNLOCK_SURFACE(surf)

	vkDestroyCommandPool(surf->dev->vkDev, surf->cmdPool, NULL);
	vkDestroyFramebuffer(surf->dev->vkDev, surf->fb, NULL);

	if (!surf->img->imported)
		vkh_image_destroy(surf->img);

	vkh_image_destroy(surf->imgMS);
	vkh_image_destroy(surf->stencil);

	if (surf->dev->threadAware)
		mtx_destroy (&surf->mutex);

#if VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
	vkDestroySemaphore (surf->dev->vkDev, surf->timeline, NULL);
#else
	vkDestroyFence (surf->dev->vkDev, surf->flushFence, NULL);
#endif

	vkvg_device_destroy (surf->dev);
	free(surf);
}

VkvgSurface vkvg_surface_reference (VkvgSurface surf) {
	if (!surf->status) {
		LOCK_SURFACE(surf)
		surf->references++;
		UNLOCK_SURFACE(surf)
	}
	return surf;
}
uint32_t vkvg_surface_get_reference_count (VkvgSurface surf) {
	if (surf->status)
		return 0;
	return surf->references;
}
void vkvg_surface_resize(VkvgSurface surf, uint32_t width, uint32_t height) {
	if (surf->status)
		return;

	surf->width = MAX(1, width);
	surf->height = MAX(1, height);

	vkDestroyFramebuffer(surf->dev->vkDev, surf->fb, NULL);

	if (width > surf->imgWidth || height > surf->imgHeight) {
		if (!surf->img->imported)
			vkh_image_destroy(surf->img);
		vkh_image_destroy(surf->imgMS);
		vkh_image_destroy(surf->stencil);

		surf->newSurf = true;//used to clear all attacments on first render pass
		_create_surface_images (surf);
		_transition_surf_images (surf);

		surf->imgWidth = surf->width;
		surf->imgHeight = surf->height;
	}

	_create_framebuffer (surf);
}
VkImage vkvg_surface_get_vk_image(VkvgSurface surf)
{
	if (surf->status)
		return NULL;
	if (surf->dev->deferredResolve)
		_explicit_ms_resolve(surf);
	return vkh_image_get_vkimage (surf->img);
}
void vkvg_surface_resolve (VkvgSurface surf){
	if (surf->status || !surf->dev->deferredResolve)
		return;
	_explicit_ms_resolve(surf);
}
VkFormat vkvg_surface_get_vk_format(VkvgSurface surf)
{
	if (surf->status)
		return VK_FORMAT_UNDEFINED;
	return surf->format;
}
uint32_t vkvg_surface_get_width (VkvgSurface surf) {
	if (surf->status)
		return 0;
	return surf->width;
}
uint32_t vkvg_surface_get_height (VkvgSurface surf) {
	if (surf->status)
		return 0;
	return surf->height;
}

vkvg_status_t vkvg_surface_write_to_png (VkvgSurface surf, const char* path){
	if (surf->status) {
		LOG(VKVG_LOG_ERR, "vkvg_surface_write_to_png failed, invalid status: %d\n", surf->status);
		return VKVG_STATUS_INVALID_STATUS;
	}
	if (surf->dev->status) {
		LOG(VKVG_LOG_ERR, "vkvg_surface_write_to_png failed, invalid device status: %d\n", surf->dev->status);
		return VKVG_STATUS_INVALID_STATUS;
	}
	if (surf->dev->pngStagFormat == VK_FORMAT_UNDEFINED) {
		LOG(VKVG_LOG_ERR, "no suitable image format for png write\n");
		return VKVG_STATUS_INVALID_FORMAT;
	}
	if (!path) {
		LOG(VKVG_LOG_ERR, "vkvg_surface_write_to_png failed, null path\n");
		return VKVG_STATUS_WRITE_ERROR;
	}
	LOCK_SURFACE(surf)
	VkImageSubresourceLayers imgSubResLayers = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};
	VkvgDevice dev = surf->dev;

	//RGBA to blit to, surf img is bgra
	VkhImage stagImg;

	if (dev->pngStagTiling == VK_IMAGE_TILING_LINEAR)
		stagImg = vkh_image_create ((VkhDevice)surf->dev, dev->pngStagFormat, surf->width, surf->height, dev->pngStagTiling,
										 VKH_MEMORY_USAGE_GPU_TO_CPU,
										 VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	else
		stagImg = vkh_image_create ((VkhDevice)surf->dev, dev->pngStagFormat, surf->width,surf->height, dev->pngStagTiling,
										 VKH_MEMORY_USAGE_GPU_ONLY,
										 VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	VkCommandBuffer cmd = surf->cmd;
	vkh_cmd_begin (cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	vkh_image_set_layout (cmd, stagImg, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	vkh_image_set_layout (cmd, surf->img, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkImageBlit blit = {
		.srcSubresource = imgSubResLayers,
		.srcOffsets[1] = {(int32_t)surf->width, (int32_t)surf->height, 1},
		.dstSubresource = imgSubResLayers,
		.dstOffsets[1] = {(int32_t)surf->width, (int32_t)surf->height, 1},
	};
	vkCmdBlitImage	(cmd,
					 vkh_image_get_vkimage (surf->img), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					 vkh_image_get_vkimage (stagImg),  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);

	vkh_cmd_end		(cmd);

	_surface_submit_cmd (surf);

	VkhImage stagImgLinear = stagImg;

	if (dev->pngStagTiling == VK_IMAGE_TILING_OPTIMAL) {
		stagImgLinear = vkh_image_create ((VkhDevice)surf->dev, dev->pngStagFormat, surf->width, surf->height, VK_IMAGE_TILING_LINEAR,
										  VKH_MEMORY_USAGE_GPU_TO_CPU,
										  VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		VkImageCopy cpy = {
			.srcSubresource = imgSubResLayers,
			.srcOffset = {0},
			.dstSubresource = imgSubResLayers,
			.dstOffset = {0},
			.extent = {(int32_t)surf->width, (int32_t)surf->height, 1}
		};

		vkh_cmd_begin (cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		vkh_image_set_layout (cmd, stagImgLinear, VK_IMAGE_ASPECT_COLOR_BIT,
							  VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		vkh_image_set_layout (cmd, stagImg, VK_IMAGE_ASPECT_COLOR_BIT,
							  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		vkCmdCopyImage(cmd,
					   vkh_image_get_vkimage (stagImg), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					   vkh_image_get_vkimage (stagImgLinear),  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpy);

		vkh_cmd_end		(cmd);

		_surface_submit_cmd (surf);

		vkh_image_destroy (stagImg);
	}

	void* img = vkh_image_map (stagImgLinear);

	uint64_t stride = vkh_image_get_stride(stagImgLinear);

	stbi_write_png (path, (int32_t)surf->width, (int32_t)surf->height, 4, img, (int32_t)stride);

	vkh_image_unmap (stagImgLinear);
	vkh_image_destroy (stagImgLinear);

	UNLOCK_SURFACE(surf)
	return VKVG_STATUS_SUCCESS;
}

vkvg_status_t vkvg_surface_write_to_memory (VkvgSurface surf, unsigned char* const bitmap){
	if (surf->status) {
		LOG(VKVG_LOG_ERR, "vkvg_surface_write_to_memory failed, invalid status: %d\n", surf->status);
		return VKVG_STATUS_INVALID_STATUS;
	}
	if (!bitmap) {
		LOG(VKVG_LOG_ERR, "vkvg_surface_write_to_memory failed, null path\n");
		return VKVG_STATUS_INVALID_IMAGE;
	}

	LOCK_SURFACE(surf)

	VkImageSubresourceLayers imgSubResLayers = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};
	VkvgDevice dev = surf->dev;

	//RGBA to blit to, surf img is bgra
	VkhImage stagImg= vkh_image_create ((VkhDevice)surf->dev,VK_FORMAT_B8G8R8A8_UNORM ,surf->width,surf->height,VK_IMAGE_TILING_LINEAR,
										 VKH_MEMORY_USAGE_GPU_TO_CPU,
										 VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	VkCommandBuffer cmd = surf->cmd;

	vkh_cmd_begin (cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	vkh_image_set_layout (cmd, stagImg, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	vkh_image_set_layout (cmd, surf->img, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkImageBlit blit = {
		.srcSubresource = imgSubResLayers,
		.srcOffsets[1] = {(int32_t)surf->width, (int32_t)surf->height, 1},
		.dstSubresource = imgSubResLayers,
		.dstOffsets[1] = {(int32_t)surf->width, (int32_t)surf->height, 1},
	};
	vkCmdBlitImage	(cmd,
					 vkh_image_get_vkimage (surf->img), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					 vkh_image_get_vkimage (stagImg),  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);

	vkh_cmd_end		(cmd);

	_surface_submit_cmd (surf);

	uint64_t stride = vkh_image_get_stride(stagImg);
	uint32_t dest_stride = surf->width * 4;

	char* img = vkh_image_map (stagImg);
	char* row = (char*)bitmap;
	for (uint32_t y = 0; y < surf->height; y++) {
		memcpy(row, img, dest_stride);
		row += dest_stride;
		img += stride;
	}

	vkh_image_unmap (stagImg);
	vkh_image_destroy (stagImg);

	UNLOCK_SURFACE(surf)

	return VKVG_STATUS_SUCCESS;
}

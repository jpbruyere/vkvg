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

#include "vkvg_surface_internal.h"
#include "vkvg_device_internal.h"
#include "vkvg_context_internal.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "vkh_image.h"
#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "nanosvg.h"

void _explicit_ms_resolve (VkvgSurface surf){
	VkvgDevice      dev = surf->dev;
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
	VkvgDevice      dev = surf->dev;
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
	surf->img = vkh_image_create((VkhDevice)surf->dev,surf->format,surf->width,surf->height,surf->tiling,VMA_MEMORY_USAGE_GPU_ONLY,
									 VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	vkh_image_create_descriptor(surf->img, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
#ifdef DEBUG
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
#ifdef DEBUG
		vkh_image_set_name(surf->imgMS, "SURF MS color IMG");
		vkh_device_set_object_name((VkhDevice)surf->dev, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)vkh_image_get_view(surf->imgMS), "SURF MS color VIEW");
		vkh_device_set_object_name((VkhDevice)surf->dev, VK_OBJECT_TYPE_SAMPLER, (uint64_t)vkh_image_get_sampler(surf->imgMS), "SURF MS color SAMPLER");
#endif
	}
	surf->stencil = vkh_image_ms_create((VkhDevice)surf->dev,FB_STENCIL_FORMAT,surf->dev->samples,surf->width,surf->height,VMA_MEMORY_USAGE_GPU_ONLY,                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	vkh_image_create_descriptor(surf->stencil, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_STENCIL_BIT, VK_FILTER_NEAREST,
								VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
#ifdef DEBUG
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
#ifdef DEBUG
	vkh_device_set_object_name((VkhDevice)surf->dev, VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)surf->fb, "SURF FB");
#endif
}
void _create_surface_images (VkvgSurface surf) {

	_create_surface_main_image      (surf);
	_create_surface_secondary_images(surf);
	_create_framebuffer             (surf);

	_clear_surface              (surf, VK_IMAGE_ASPECT_STENCIL_BIT);
#if defined(DEBUG) && defined(ENABLE_VALIDATION)
	vkh_image_set_name(surf->img, "surfImg");
	vkh_image_set_name(surf->imgMS, "surfImgMS");
	vkh_image_set_name(surf->stencil, "surfStencil");
#endif
}
VkvgSurface _create_surface (VkvgDevice dev, VkFormat format) {
	VkImageTiling tiling;
	if (!_get_best_image_tiling (dev, format, &tiling)) {
		dev->status = VKVG_STATUS_INVALID_FORMAT;
		return NULL;
	}

	VkvgSurface surf = (vkvg_surface*)calloc(1,sizeof(vkvg_surface));
	if (!surf) {
		dev->status = VKVG_STATUS_NO_MEMORY;
		return NULL;
	}

	surf->dev = dev;
	surf->format = format;
	surf->tiling = tiling;

	dev->status = VKVG_STATUS_SUCCESS;
	return surf;
}

void vkvg_surface_clear (VkvgSurface surf) {
	_clear_surface(surf, VK_IMAGE_ASPECT_STENCIL_BIT|VK_IMAGE_ASPECT_COLOR_BIT);
}
VkvgSurface vkvg_surface_create(VkvgDevice dev, uint32_t width, uint32_t height){
	VkvgSurface surf = _create_surface(dev, FB_COLOR_FORMAT);
	if (!surf)
		return NULL;

	surf->width = width;
	surf->height = height;
	surf->new = true;//used to clear all attacments on first render pass

	_create_surface_images (surf);

	surf->references = 1;
	vkvg_device_reference (surf->dev);

	return surf;
}
VkvgSurface vkvg_surface_create_for_VkhImage (VkvgDevice dev, void* vkhImg) {
	VkvgSurface surf = _create_surface(dev, FB_COLOR_FORMAT);
	if (!surf)
		return NULL;

	VkhImage img = (VkhImage)vkhImg;
	surf->width = img->infos.extent.width;
	surf->height= img->infos.extent.height;

	surf->img = img;

	vkh_image_create_sampler(img, VK_FILTER_NEAREST, VK_FILTER_NEAREST,
							 VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

	_create_surface_secondary_images   (surf);
	_create_framebuffer         (surf);
	_clear_surface              (surf, VK_IMAGE_ASPECT_STENCIL_BIT);

	surf->references = 1;
	vkvg_device_reference (surf->dev);

	return surf;
}
//TODO: it would be better to blit in original size and create ms final image with dest surf dims
VkvgSurface vkvg_surface_create_from_bitmap (VkvgDevice dev, unsigned char* img, uint32_t width, uint32_t height) {
	VkvgSurface surf = _create_surface(dev, FB_COLOR_FORMAT);
	if (!surf)
		return NULL;

	surf->width = width;
	surf->height = height;

	_create_surface_images (surf);
	_clear_surface(surf, VK_IMAGE_ASPECT_COLOR_BIT);

	uint32_t imgSize = width * height * 4;
	VkImageSubresourceLayers imgSubResLayers = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};
	//original format image
	VkhImage stagImg= vkh_image_create ((VkhDevice)surf->dev,VK_FORMAT_R8G8B8A8_UNORM,surf->width,surf->height,VK_IMAGE_TILING_LINEAR,
										 VMA_MEMORY_USAGE_GPU_ONLY,
										 VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	//bgra bliting target
	VkhImage tmpImg = vkh_image_create ((VkhDevice)surf->dev,surf->format,surf->width,surf->height,VK_IMAGE_TILING_LINEAR,
										 VMA_MEMORY_USAGE_GPU_ONLY,
										 VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	vkh_image_create_descriptor (tmpImg, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT,
								 VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
	//staging buffer
	vkvg_buff buff = {0};
	vkvg_buffer_create(dev, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, imgSize, &buff);

	memcpy (buff.allocInfo.pMappedData, img, imgSize);

	VkCommandBuffer cmd = dev->cmd;

	_wait_and_reset_device_fence (dev);

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
	vkCmdBlitImage  (cmd,
					 vkh_image_get_vkimage (stagImg), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					 vkh_image_get_vkimage (tmpImg),  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

	vkh_image_set_layout (cmd, tmpImg, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	vkh_cmd_end     (cmd);
	_submit_cmd     (dev, &cmd, dev->fence);

	//don't reset fence after completion as this is the last cmd. (signaled idle fence)
	vkWaitForFences (dev->vkDev, 1, &dev->fence, VK_TRUE, UINT64_MAX);

	vkvg_buffer_destroy (&buff);
	vkh_image_destroy   (stagImg);

	//create tmp context with rendering pipeline to create the multisample img
	VkvgContext ctx = vkvg_create (surf);

/*    VkClearAttachment ca = {VK_IMAGE_ASPECT_COLOR_BIT,0, { 0.0f, 0.0f, 0.0f, 0.0f }};
	VkClearRect cr = {{{0,0},{surf->width,surf->height}},0,1};
	vkCmdClearAttachments(ctx->cmd, 1, &ca, 1, &cr);*/

	vec4 srcRect = {.x=0,.y=0,.width=(float)surf->width,.height=(float)surf->height};
	ctx->pushConsts.source = srcRect;
	ctx->pushConsts.patternType = VKVG_PATTERN_TYPE_SURFACE;

	//_update_push_constants (ctx);
	_update_descriptor_set (ctx, tmpImg, ctx->dsSrc);
	_ensure_renderpass_is_started  (ctx);

	vkvg_paint          (ctx);
	vkvg_destroy        (ctx);

	vkh_image_destroy   (tmpImg);

	surf->references = 1;
	vkvg_device_reference (surf->dev);

	return surf;
}
VkvgSurface vkvg_surface_create_from_image (VkvgDevice dev, const char* filePath) {
	int w = 0,
		h = 0,
		channels = 0;
	unsigned char *img = stbi_load(filePath, &w, &h, &channels, 4);//force 4 components per pixel
	if (img == NULL){
		fprintf (stderr, "Could not load texture from %s, %s\n", filePath, stbi_failure_reason());
		return NULL;
	}

	VkvgSurface surf = vkvg_surface_create_from_bitmap(dev, img, (uint32_t)w, (uint32_t)h);

	stbi_image_free (img);

	return surf;
}

void _svg_set_color (VkvgContext ctx, uint32_t c, float alpha) {
	float a = (c >> 24 & 255) / 255.f;
	float b = (c >> 16 & 255) / 255.f;
	float g = (c >> 8 & 255) / 255.f;
	float r = (c & 255) / 255.f;
	vkvg_set_source_rgba(ctx,r,g,b,a*alpha);
}

VkvgSurface _svg_load (VkvgDevice dev, NSVGimage* svg) {
	if (svg == NULL) {
		LOG(VKVG_LOG_ERR, "nsvg error");
		return NULL;
	}
	VkvgSurface surf = _create_surface(dev, FB_COLOR_FORMAT);
	if (!surf)
		return NULL;

	surf->width = (uint32_t)svg->width;
	surf->height = (uint32_t)svg->height;
	surf->new = true;

	_create_surface_images (surf);

	VkvgContext ctx = vkvg_create(surf);
	vkvg_render_svg(ctx, svg, NULL);
	vkvg_destroy(ctx);

	nsvgDelete(svg);

	surf->references = 1;
	vkvg_device_reference (surf->dev);

	return surf;
}

VkvgSurface vkvg_surface_create_from_svg (VkvgDevice dev, const char* filePath) {
	return _svg_load(dev, nsvgParseFromFile(filePath, "px", (float)dev->hdpi));
}
VkvgSurface vkvg_surface_create_from_svg_fragment (VkvgDevice dev, char* fragment) {
	return _svg_load(dev, nsvgParse(fragment, "px", (float)dev->hdpi));
}
NSVGimage* nsvg_load_file (VkvgDevice dev, const char* filePath) {
	return nsvgParseFromFile(filePath, "px", (float)dev->hdpi);
}
NSVGimage* nsvg_load (VkvgDevice dev, char* fragment) {
	return nsvgParse (fragment, "px", (float)dev->hdpi);
}
void nsvg_destroy (NSVGimage* svg) {
	nsvgDelete(svg);
}
void nsvg_get_size (NSVGimage* svg, int* width, int* height) {
	*width = (int)svg->width;
	*height = (int)svg->height;
}

void vkvg_surface_destroy(VkvgSurface surf)
{
	surf->references--;
	if (surf->references > 0)
		return;
	vkDestroyFramebuffer(surf->dev->vkDev, surf->fb, NULL);

	if (!surf->img->imported)
		vkh_image_destroy(surf->img);

	vkh_image_destroy(surf->imgMS);
	vkh_image_destroy(surf->stencil);

	vkvg_device_destroy (surf->dev);
	free(surf);
}

VkvgSurface vkvg_surface_reference (VkvgSurface surf) {
	surf->references++;
	return surf;
}
uint32_t vkvg_surface_get_reference_count (VkvgSurface surf) {
	return surf->references;
}

VkImage vkvg_surface_get_vk_image(VkvgSurface surf)
{
	if (surf->dev->deferredResolve)
		_explicit_ms_resolve(surf);
	return vkh_image_get_vkimage (surf->img);
}
void vkvg_multisample_surface_resolve (VkvgSurface surf){
	_explicit_ms_resolve(surf);
}
VkFormat vkvg_surface_get_vk_format(VkvgSurface surf)
{
	return surf->format;
}
uint32_t vkvg_surface_get_width (VkvgSurface surf) {
	return surf->width;
}
uint32_t vkvg_surface_get_height (VkvgSurface surf) {
	return surf->height;
}

void vkvg_surface_write_to_png (VkvgSurface surf, const char* path){
	uint32_t stride = surf->width * 4;
	VkImageSubresourceLayers imgSubResLayers = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};
	VkvgDevice dev = surf->dev;

	//RGBA to blit to, surf img is bgra
	VkhImage stagImg= vkh_image_create ((VkhDevice)surf->dev,VK_FORMAT_R8G8B8A8_UNORM,surf->width,surf->height,VK_IMAGE_TILING_LINEAR,
										 VMA_MEMORY_USAGE_GPU_TO_CPU,
										 VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	VkCommandBuffer cmd = dev->cmd;
	_wait_and_reset_device_fence (dev);

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
	vkCmdBlitImage  (cmd,
					 vkh_image_get_vkimage (surf->img), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					 vkh_image_get_vkimage (stagImg),  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);

	vkh_cmd_end     (cmd);
	_submit_cmd     (dev, &cmd, dev->fence);
	vkWaitForFences (dev->vkDev, 1, &dev->fence, VK_TRUE, UINT64_MAX);

	void* img = vkh_image_map (stagImg);

	stbi_write_png (path, (int32_t)surf->width, (int32_t)surf->height, 4, img, (int32_t)stride);

	vkh_image_unmap (stagImg);
	vkh_image_destroy (stagImg);
}

/*VkhImage vkvg_surface_get_vkh_image(VkvgSurface surf)
{
	return surf->img;
}*/

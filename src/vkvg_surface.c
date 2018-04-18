#include "vkvg_surface_internal.h"
#include "vkvg_device_internal.h"
#include "vkvg_context_internal.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void _clear_stencil (VkvgSurface surf)
{
    VkvgDevice      dev = surf->dev;
    VkCommandBuffer cmd = dev->cmd;

    _wait_device_fence (dev);

    vkh_cmd_begin (cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkClearDepthStencilValue clr = {1.0f,0};
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_STENCIL_BIT,0,1,0,1};

    vkh_image_set_layout (cmd, surf->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    vkCmdClearDepthStencilImage (cmd, vkh_image_get_vkimage (surf->stencilMS),
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,&clr,1,&range);

    vkh_image_set_layout (cmd, surf->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    vkh_cmd_end (cmd);

    vkh_cmd_submit (dev->gQueue, &cmd, dev->fence);
}

void _init_surface (VkvgSurface surf) {
    surf->format = FB_COLOR_FORMAT;//force bgra internally
    surf->img = vkh_image_create(surf->dev,surf->format,surf->width,surf->height,VK_IMAGE_TILING_LINEAR,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                     VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    surf->imgMS = vkh_image_ms_create(surf->dev,surf->format,VKVG_SAMPLES,surf->width,surf->height,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    surf->stencilMS = vkh_image_ms_create(surf->dev,VK_FORMAT_S8_UINT,VKVG_SAMPLES,surf->width,surf->height,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    vkh_image_create_descriptor(surf->img, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    vkh_image_create_descriptor(surf->imgMS, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    vkh_image_create_descriptor(surf->stencilMS, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_STENCIL_BIT, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    VkImageView attachments[] = {
        vkh_image_get_view (surf->imgMS),
        vkh_image_get_view (surf->img),
        vkh_image_get_view (surf->stencilMS),
    };
    VkFramebufferCreateInfo frameBufferCreateInfo = { .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                                      .renderPass = surf->dev->renderPass,
                                                      .attachmentCount = 3,
                                                      .pAttachments = attachments,
                                                      .width = surf->width,
                                                      .height = surf->height,
                                                      .layers = 1 };
    VK_CHECK_RESULT(vkCreateFramebuffer(surf->dev->vkDev, &frameBufferCreateInfo, NULL, &surf->fb));

    _clear_stencil(surf);
}
VkvgSurface vkvg_surface_create(VkvgDevice dev, uint32_t width, uint32_t height){
    VkvgSurface surf = (vkvg_surface*)calloc(1,sizeof(vkvg_surface));

    surf->dev = dev;
    surf->width = width;
    surf->height = height;

    _init_surface (surf);

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

    VkvgSurface surf = (vkvg_surface*)calloc(1,sizeof(vkvg_surface));

    surf->dev = dev;
    surf->width = w;
    surf->height = h;

    _init_surface (surf);

    uint32_t imgSize = w * h * 4;
    VkImageSubresourceLayers imgSubResLayers = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};
    //original format image
    VkhImage stagImg= vkh_image_create (surf->dev,VK_FORMAT_R8G8B8A8_UNORM,surf->width,surf->height,VK_IMAGE_TILING_LINEAR,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    //bgra bliting target
    VkhImage tmpImg = vkh_image_create (surf->dev,surf->format,surf->width,surf->height,VK_IMAGE_TILING_LINEAR,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                         VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkh_image_create_descriptor (tmpImg, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT,
                                 VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    //staging buffer
    VkhBuffer buff = vkh_buffer_create (dev, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, imgSize);
    VK_CHECK_RESULT (vkh_buffer_map (buff));
    memcpy (vkh_buffer_get_mapped_pointer (buff), img, imgSize);

    VkCommandBuffer cmd = dev->cmd;

    _wait_device_fence (dev);

    vkh_cmd_begin (cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkh_image_set_layout (cmd, stagImg, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);


    VkBufferImageCopy bufferCopyRegion = { .imageSubresource = imgSubResLayers,
                                           .imageExtent = {surf->width,surf->height,1}};

    vkCmdCopyBufferToImage(cmd, vkh_buffer_get_vkbuffer (buff),
        vkh_image_get_vkimage (stagImg), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

    vkh_image_set_layout (cmd, stagImg, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    vkh_image_set_layout (cmd, tmpImg, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkImageBlit blit = {
        .srcSubresource = imgSubResLayers,
        .srcOffsets[1] = {surf->width, surf->height, 1},
        .dstSubresource = imgSubResLayers,
        .dstOffsets[1] = {surf->width, surf->height, 1},
    };
    vkCmdBlitImage  (cmd,
                     vkh_image_get_vkimage (stagImg), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     vkh_image_get_vkimage (tmpImg),  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

    vkh_cmd_end     (cmd);
    vkh_cmd_submit  (dev->gQueue, &cmd, dev->fence);

    _wait_device_fence (dev);

    vkh_buffer_unmap    (buff);
    vkh_buffer_destroy  (buff);
    vkh_image_destroy   (stagImg);

    //create tmp context with rendering pipeline to create the multisample img
    VkvgContext ctx = vkvg_create (surf);

    VkClearAttachment ca = {VK_IMAGE_ASPECT_COLOR_BIT,0, { 0.0f, 0.0f, 0.0f, 0.0f }};
    VkClearRect cr = {{{0,0},{surf->width,surf->height}},0,1};
    vkCmdClearAttachments(ctx->cmd, 1, &ca, 1, &cr);

    vec4 srcRect = {0,0,surf->width,surf->height};
    ctx->pushConsts.source = srcRect;
    ctx->pushConsts.patternType = VKVG_PATTERN_TYPE_SURFACE;

    _update_push_constants (ctx);
    _update_descriptor_set (ctx, tmpImg, ctx->dsSrc);

    vkvg_paint          (ctx);
    vkvg_destroy        (ctx);
    vkh_image_destroy   (tmpImg);

    return surf;
}

void vkvg_surface_destroy(VkvgSurface surf)
{
    vkDestroyFramebuffer(surf->dev->vkDev, surf->fb, NULL);
    vkh_image_destroy(surf->img);
    vkh_image_destroy(surf->imgMS);
    vkh_image_destroy(surf->stencilMS);
    free(surf);
}

VkImage vkvg_surface_get_vk_image(VkvgSurface surf)
{
    return vkh_image_get_vkimage (surf->img);
}
/*VkhImage vkvg_surface_get_vkh_image(VkvgSurface surf)
{
    return surf->img;
}*/

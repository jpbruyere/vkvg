#include "vkvg_surface_internal.h"
#include "vkvg_device_internal.h"

void _clear_stencil (VkvgSurface surf)
{
    VkvgDevice      dev = surf->dev;
    VkCommandBuffer cmd = dev->cmd;

    vkWaitForFences (dev->vkDev, 1, &dev->fence, VK_TRUE, UINT64_MAX);
    vkResetFences   (dev->vkDev, 1, &dev->fence);

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

    vkh_cmd_submit (dev->queue, &cmd, dev->fence);
}

VkvgSurface vkvg_surface_create(VkvgDevice dev, uint32_t width, uint32_t height){
    VkvgSurface surf = (vkvg_surface*)calloc(1,sizeof(vkvg_surface));

    surf->dev = dev;
    surf->width = width;
    surf->height = height;

    surf->img = vkh_image_create(dev,FB_COLOR_FORMAT,width,height,VK_IMAGE_TILING_LINEAR,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                     VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    surf->imgMS = vkh_image_ms_create(dev,FB_COLOR_FORMAT,VKVG_SAMPLES,width,height,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    surf->stencilMS = vkh_image_ms_create(dev,VK_FORMAT_S8_UINT,VKVG_SAMPLES,width,height,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
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
                                                      .renderPass = dev->renderPass,
                                                      .attachmentCount = 3,
                                                      .pAttachments = attachments,
                                                      .width = width,
                                                      .height = height,
                                                      .layers = 1 };
    VK_CHECK_RESULT(vkCreateFramebuffer(surf->dev->vkDev, &frameBufferCreateInfo, NULL, &surf->fb));

    _clear_stencil(surf);

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

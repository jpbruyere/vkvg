#include "vkvg_surface_internal.h"
#include "vkvg_device_internal.h"

void _clear_stencil (VkvgSurface surf)
{
    vkh_cmd_begin (surf->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkClearDepthStencilValue clr = {1.0f,0};
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_STENCIL_BIT,0,1,0,1};

    vkh_image_set_layout (surf->cmd, surf->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    vkCmdClearDepthStencilImage (surf->cmd, surf->stencilMS->image,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,&clr,1,&range);

    vkh_image_set_layout (surf->cmd, surf->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    vkh_cmd_end (surf->cmd);

    //vkh_cmd_submit_with_semaphores (surf->dev->queue,&surf->cmd,VK_NULL_HANDLE,surf->semaphore,VK_NULL_HANDLE);
    VkFence fence = vkh_fence_create(surf->dev->vkDev);
    vkh_cmd_submit (surf->dev->queue,&surf->cmd,fence);
    vkWaitForFences(surf->dev->vkDev,1,&fence,VK_TRUE,UINT64_MAX);
    vkDestroyFence(surf->dev->vkDev,fence,NULL);
}

VkvgSurface vkvg_surface_create(VkvgDevice dev, int32_t width, uint32_t height){
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
        surf->imgMS->view, surf->img->view,
        surf->stencilMS->view,
    };
    VkFramebufferCreateInfo frameBufferCreateInfo = { .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                                      .renderPass = dev->renderPass,
                                                      .attachmentCount = 3,
                                                      .pAttachments = attachments,
                                                      .width = width,
                                                      .height = height,
                                                      .layers = 1 };
    VK_CHECK_RESULT(vkCreateFramebuffer(surf->dev->vkDev, &frameBufferCreateInfo, NULL, &surf->fb));

    surf->semaphore = vkh_semaphore_create(dev->vkDev);
    surf->cmd = vkh_cmd_buff_create(surf->dev->vkDev, surf->dev->cmdPool,VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    _clear_stencil(surf);

    return surf;
}

void vkvg_surface_destroy(VkvgSurface surf)
{
    vkDestroySemaphore(surf->dev->vkDev,surf->semaphore,NULL);
    vkFreeCommandBuffers(surf->dev->vkDev,surf->dev->cmdPool,1,&surf->cmd);
    vkDestroyFramebuffer(surf->dev->vkDev, surf->fb, NULL);
    vkh_image_destroy(surf->img);
    vkh_image_destroy(surf->imgMS);
    vkh_image_destroy(surf->stencilMS);
    free(surf);
}

VkImage vkvg_surface_get_vk_image(VkvgSurface surf)
{
    return surf->img->image;
}
VkImage vkvg_surface_get_vkh_image(VkvgSurface surf)
{
    return surf->img;
}

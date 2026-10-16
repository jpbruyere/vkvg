/*
 * Copyright (c) 2018-2022 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
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

#define GetInstProcAddress(inst, func) (PFN_##func) vkGetInstanceProcAddr(inst, #func);

#define GetVkProcAddress(dev, inst, func)                                                                              \
    (vkGetDeviceProcAddr(dev, #func) == NULL) ? (PFN_##func)vkGetInstanceProcAddr(inst, #func)                         \
                                              : (PFN_##func)vkGetDeviceProcAddr(dev, #func)

#include "vkvg_device_internal.h"
#include "vkvg_context_internal.h"

uint32_t vkvg_log_level = VKVG_LOG_DEBUG;
#ifdef VKVG_WIRED_DEBUG
vkvg_wired_debug_mode vkvg_wired_debug = vkvg_wired_debug_mode_normal;
#endif

PFN_vkCmdBindPipeline       CmdBindPipeline;
PFN_vkCmdBindDescriptorSets CmdBindDescriptorSets;
PFN_vkCmdBindIndexBuffer    CmdBindIndexBuffer;
PFN_vkCmdBindVertexBuffers  CmdBindVertexBuffers;

PFN_vkCmdDrawIndexed CmdDrawIndexed;
PFN_vkCmdDraw        CmdDraw;

PFN_vkCmdSetStencilCompareMask CmdSetStencilCompareMask;
PFN_vkCmdSetStencilReference   CmdSetStencilReference;
PFN_vkCmdSetStencilWriteMask   CmdSetStencilWriteMask;
PFN_vkCmdBeginRenderPass       CmdBeginRenderPass;
PFN_vkCmdEndRenderPass         CmdEndRenderPass;
PFN_vkCmdSetViewport           CmdSetViewport;
PFN_vkCmdSetScissor            CmdSetScissor;

PFN_vkCmdPushConstants CmdPushConstants;

PFN_vkWaitForFences      WaitForFences;
PFN_vkResetFences        ResetFences;
PFN_vkResetCommandBuffer ResetCommandBuffer;

bool _device_try_get_phyinfo(VkhPhyInfo* phys, uint32_t phyCount, VkPhysicalDeviceType gpuType, VkhPhyInfo* phy) {
    for (uint32_t i = 0; i < phyCount; i++) {
        if (vkh_phyinfo_get_properties(phys[i]).deviceType == gpuType) {
            *phy = phys[i];
            return true;
        }
    }
    return false;
}
// TODO:save/reload cache in user temp directory
void _device_create_pipeline_cache(VkvgDevice dev) {

    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
    VK_CHECK_RESULT(vkCreatePipelineCache(dev->vkDev, &pipelineCacheCreateInfo, NULL, &dev->pipelineCache));
}

void _device_createDescriptorSetLayout(VkvgDevice dev) {

    VkDescriptorSetLayoutBinding    dsLayoutBinding    = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                                                          VK_SHADER_STAGE_FRAGMENT_BIT, NULL};
    VkDescriptorSetLayoutCreateInfo dsLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .bindingCount = 1, .pBindings = &dsLayoutBinding};
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(dev->vkDev, &dsLayoutCreateInfo, NULL, &dev->dslFont));
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(dev->vkDev, &dsLayoutCreateInfo, NULL, &dev->dslSrc));
    dsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(dev->vkDev, &dsLayoutCreateInfo, NULL, &dev->dslGrad));

    VkPushConstantRange pushConstantRange[] = {
        {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constants)},
        //{VK_SHADER_STAGE_FRAGMENT_BIT,0,sizeof(push_constants)}
    };
    VkDescriptorSetLayout dsls[] = {dev->dslFont, dev->dslSrc, dev->dslGrad};

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                           .pushConstantRangeCount = 1,
                                                           .pPushConstantRanges =
                                                               (VkPushConstantRange*)&pushConstantRange,
                                                           .setLayoutCount = 3,
                                                           .pSetLayouts    = dsls};
    VK_CHECK_RESULT(vkCreatePipelineLayout(dev->vkDev, &pipelineLayoutCreateInfo, NULL, &dev->pipelineLayout));
}

void _device_wait_idle(VkvgDevice dev) { vkDeviceWaitIdle(dev->vkDev); }
void _device_wait_and_reset_device_fence(VkvgDevice dev) {
    vkWaitForFences(dev->vkDev, 1, &dev->fence, VK_TRUE, UINT64_MAX);
    ResetFences(dev->vkDev, 1, &dev->fence);
}

bool _device_try_get_cached_context(VkvgDevice dev, VkvgContext* pCtx) {
    LOCK_DEVICE

    if (dev->cachedContextCount) {
        thrd_t       curThread = thrd_current();
        _cached_ctx* prev      = NULL;
        _cached_ctx* cur       = dev->cachedContextLast;
        while (cur) {
            if (thrd_equal(cur->thread, curThread)) {
                if (prev)
                    prev->pNext = cur->pNext;
                else
                    dev->cachedContextLast = cur->pNext;

                dev->cachedContextCount--;

                LOG(VKVG_LOG_THREAD, "get cached context: %p, thd:%lu cached ctx: %d\n", cur->ctx, cur->thread,
                    dev->cachedContextCount);

                *pCtx = cur->ctx;
                free(cur);
                UNLOCK_DEVICE
                return true;
            }
            prev = cur;
            cur  = cur->pNext;
        }
    }
    *pCtx = NULL;
    UNLOCK_DEVICE
    return false;
}
void _device_store_context(VkvgContext ctx) {
    VkvgDevice dev = ctx->dev;

    LOCK_DEVICE

    _cached_ctx* cur = (_cached_ctx*)calloc(1, sizeof(_cached_ctx));
    cur->ctx         = ctx;
    cur->thread      = thrd_current();
    cur->pNext       = dev->cachedContextLast;

    dev->cachedContextLast = cur;
    dev->cachedContextCount++;

    LOG(VKVG_LOG_THREAD, "store context: %p, thd:%lu cached ctx: %d\n", cur->ctx, cur->thread, dev->cachedContextCount);

    ctx->references++;

    UNLOCK_DEVICE
}
void _device_submit_cmd(VkvgDevice dev, VkCommandBuffer* cmd, VkFence fence) {
    LOCK_DEVICE
    vkh_cmd_submit(dev->gQueue, cmd, fence);
    UNLOCK_DEVICE
}

bool _device_init_function_pointers(VkvgDevice dev) {
#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    if (vkGetInstanceProcAddr(dev->instance, "vkSetDebugUtilsObjectNameEXT") == VK_NULL_HANDLE) {
        LOGE("vkvg create device failed: 'VK_EXT_debug_utils' has to be loaded for Debug build\n");
        return false;
    }
    vkh_device_init_debug_utils((VkhDevice)&dev->vkDev);
#endif
    CmdBindPipeline          = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdBindPipeline);
    CmdBindDescriptorSets    = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdBindDescriptorSets);
    CmdBindIndexBuffer       = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdBindIndexBuffer);
    CmdBindVertexBuffers     = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdBindVertexBuffers);
    CmdDrawIndexed           = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdDrawIndexed);
    CmdDraw                  = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdDraw);
    CmdSetStencilCompareMask = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdSetStencilCompareMask);
    CmdSetStencilReference   = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdSetStencilReference);
    CmdSetStencilWriteMask   = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdSetStencilWriteMask);
    CmdBeginRenderPass       = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdBeginRenderPass);
    CmdEndRenderPass         = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdEndRenderPass);
    CmdSetViewport           = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdSetViewport);
    CmdSetScissor            = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdSetScissor);
    CmdPushConstants         = GetVkProcAddress(dev->vkDev, dev->instance, vkCmdPushConstants);
    WaitForFences            = GetVkProcAddress(dev->vkDev, dev->instance, vkWaitForFences);
    ResetFences              = GetVkProcAddress(dev->vkDev, dev->instance, vkResetFences);
    ResetCommandBuffer       = GetVkProcAddress(dev->vkDev, dev->instance, vkResetCommandBuffer);
    return true;
}

void _device_create_empty_texture(VkvgDevice dev, VkFormat format, VkImageTiling tiling) {
    // create empty image to bind to context source descriptor when not in use
    dev->emptyImg = vkh_image_create((VkhDevice)&dev->vkDev, format, 16, 16, tiling, VKH_MEMORY_USAGE_GPU_ONLY,
                                     VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkh_image_create_descriptor(dev->emptyImg, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_NEAREST,
                                VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,
                                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

    _device_wait_and_reset_device_fence(dev);

    vkh_cmd_begin(dev->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkh_image_set_layout(dev->cmd, dev->emptyImg, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    vkh_cmd_end(dev->cmd);
    _device_submit_cmd(dev, &dev->cmd, dev->fence);
}
void _device_check_best_image_tiling(VkvgDevice dev, VkFormat format) {
    VkFlags            stencilFormats[] = {VK_FORMAT_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,
                                           VK_FORMAT_D32_SFLOAT_S8_UINT};
    VkFormatProperties phyStencilProps = {0}, phyImgProps = {0};

    // check png blit format
    VkFlags pngBlitFormats[] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB};
    dev->pngStagFormat       = VK_FORMAT_UNDEFINED;
    for (int i = 0; i < 2; i++) {
        vkGetPhysicalDeviceFormatProperties(dev->phy, pngBlitFormats[i], &phyImgProps);
        if ((phyImgProps.linearTilingFeatures & VKVG_PNG_WRITE_IMG_REQUIREMENTS) == VKVG_PNG_WRITE_IMG_REQUIREMENTS) {
            dev->pngStagFormat = pngBlitFormats[i];
            dev->pngStagTiling = VK_IMAGE_TILING_LINEAR;
            break;
        } else if ((phyImgProps.optimalTilingFeatures & VKVG_PNG_WRITE_IMG_REQUIREMENTS) ==
                   VKVG_PNG_WRITE_IMG_REQUIREMENTS) {
            dev->pngStagFormat = pngBlitFormats[i];
            dev->pngStagTiling = VK_IMAGE_TILING_OPTIMAL;
            break;
        }
    }

    if (dev->pngStagFormat == VK_FORMAT_UNDEFINED)
        LOG(VKVG_LOG_DEBUG, "vkvg create device warning: no suitable image format for png write\n");

    dev->stencilFormat     = VK_FORMAT_UNDEFINED;
    dev->stencilAspectFlag = VK_IMAGE_ASPECT_STENCIL_BIT;
    dev->supportedTiling   = 0xff;

    vkGetPhysicalDeviceFormatProperties(dev->phy, format, &phyImgProps);

    if ((phyImgProps.optimalTilingFeatures & VKVG_SURFACE_IMGS_REQUIREMENTS) == VKVG_SURFACE_IMGS_REQUIREMENTS) {
        for (int i = 0; i < 4; i++) {
            vkGetPhysicalDeviceFormatProperties(dev->phy, stencilFormats[i], &phyStencilProps);
            if (phyStencilProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                dev->stencilFormat = stencilFormats[i];
                if (i > 0)
                    dev->stencilAspectFlag |= VK_IMAGE_ASPECT_DEPTH_BIT;
                dev->supportedTiling = VK_IMAGE_TILING_OPTIMAL;
                return;
            }
        }
    }
    if ((phyImgProps.linearTilingFeatures & VKVG_SURFACE_IMGS_REQUIREMENTS) == VKVG_SURFACE_IMGS_REQUIREMENTS) {
        for (int i = 0; i < 4; i++) {
            vkGetPhysicalDeviceFormatProperties(dev->phy, stencilFormats[i], &phyStencilProps);
            if (phyStencilProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                dev->stencilFormat = stencilFormats[i];
                if (i > 0)
                    dev->stencilAspectFlag |= VK_IMAGE_ASPECT_DEPTH_BIT;
                dev->supportedTiling = VK_IMAGE_TILING_LINEAR;
                return;
            }
        }
    }
    dev->status = VKVG_STATUS_INVALID_FORMAT;
    LOGE("vkvg create device failed: image format not supported: %d\n", format);
}

void _dump_image_format_properties(VkvgDevice dev, VkFormat format) {
    /*VkImageFormatProperties imgProps;
    VK_CHECK_RESULT(vkGetPhysicalDeviceImageFormatProperties(dev->phy,
                                                             format, VK_IMAGE_TYPE_2D, VKVG_TILING,
                                                             VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                             0, &imgProps));
    printf ("tiling			  = %d\n", VKVG_TILING);
    printf ("max extend		  = (%d, %d, %d)\n", imgProps.maxExtent.width, imgProps.maxExtent.height,
    imgProps.maxExtent.depth); printf ("max mip levels	  = %d\n", imgProps.maxMipLevels); printf ("max array layers =
    %d\n", imgProps.maxArrayLayers); printf ("sample counts	  = "); if (imgProps.sampleCounts & VK_SAMPLE_COUNT_1_BIT)
        printf ("1,");
    if (imgProps.sampleCounts & VK_SAMPLE_COUNT_2_BIT)
        printf ("2,");
    if (imgProps.sampleCounts & VK_SAMPLE_COUNT_4_BIT)
        printf ("4,");
    if (imgProps.sampleCounts & VK_SAMPLE_COUNT_8_BIT)
        printf ("8,");
    if (imgProps.sampleCounts & VK_SAMPLE_COUNT_16_BIT)
        printf ("16,");
    if (imgProps.sampleCounts & VK_SAMPLE_COUNT_32_BIT)
        printf ("32,");
    printf ("\n");
    printf ("max resource size= %lu\n", imgProps.maxResourceSize);
*/
}

/*
 * Copyright (c) 2018-2025 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
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
#include "vkvg_context_internal.h"
#include "vkh_queue.h"
#include "vkh_phyinfo.h"
#include "vk_mem_alloc.h"

#define TRY_LOAD_DEVICE_EXT(ext)                                                                                       \
    {                                                                                                                  \
        if (vkh_phyinfo_try_get_extension_properties(pi, #ext, NULL))                                                  \
            enabledExts[enabledExtsCount++] = #ext;                                                                    \
    }
void vkvg_device_set_context_cache_size(VkvgDevice dev, uint32_t maxCount) {
    if (maxCount == dev->cachedContextMaxCount)
        return;

    dev->cachedContextMaxCount = maxCount;

    _cached_ctx *cur = dev->cachedContextLast;
    while (cur && dev->cachedContextCount > dev->cachedContextMaxCount) {
        _release_context_ressources(cur->ctx);
        _cached_ctx *prev = cur;
        cur               = cur->pNext;
        free(prev);
        dev->cachedContextCount--;
    }
    dev->cachedContextLast = cur;
}
void _device_init(VkvgDevice dev, const vkvg_device_create_info_t *info) {
    dev->vkDev    = info->vkdev;
    dev->phy      = info->phy;
    dev->instance = info->inst;
    dev->hdpi     = 72;
    dev->vdpi     = 72;
    dev->samples  = info->samples;
    if (dev->samples == VK_SAMPLE_COUNT_1_BIT)
        dev->deferredResolve = false;
    else
        dev->deferredResolve = info->deferredResolve;

    dev->cachedContextMaxCount = VKVG_MAX_CACHED_CONTEXT_COUNT;

#if VKVG_DBG_STATS
    dev->debug_stats = (vkvg_debug_stats_t){0};
#endif

    VkFormat format = FB_COLOR_FORMAT;

    _device_check_best_image_tiling(dev, format);
    if (dev->status != VKVG_STATUS_SUCCESS)
        return;

    VkhDevice vkhd = (VkhDevice)&dev->vkDev;

    if (!_device_init_function_pointers(dev)) {
        dev->status = VKVG_STATUS_NULL_POINTER;
        return;
    }

    VkhPhyInfo phyInfos = vkh_phyinfo_create(dev->phy, NULL);

    dev->phyMemProps = phyInfos->memProps;
    dev->gQueue      = vkh_queue_create(vkhd, info->qFamIdx, info->qIndex);
    // mtx_init (&dev->gQMutex, mtx_plain);

    vkh_phyinfo_destroy(phyInfos);

#ifdef VKH_USE_VMA
    VmaAllocatorCreateInfo allocatorInfo = {.physicalDevice = info->phy, .device = info->vkdev};
    vmaCreateAllocator(&allocatorInfo, (VmaAllocator *)&dev->allocator);
#endif

    dev->cmdPool = vkh_cmd_pool_create(vkhd, dev->gQueue->familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    dev->cmd     = vkh_cmd_buff_create(vkhd, dev->cmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    dev->fence   = vkh_fence_create_signaled(vkhd);

    _device_create_pipeline_cache(dev);
    _fonts_cache_create(dev);
    if (dev->deferredResolve || dev->samples == VK_SAMPLE_COUNT_1_BIT) {
        dev->renderPass =
            _device_createRenderPassNoResolve(dev, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_LOAD);
        dev->renderPass_ClearStencil =
            _device_createRenderPassNoResolve(dev, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_CLEAR);
        dev->renderPass_ClearAll =
            _device_createRenderPassNoResolve(dev, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_CLEAR);
    } else {
        dev->renderPass = _device_createRenderPassMS(dev, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_LOAD);
        dev->renderPass_ClearStencil =
            _device_createRenderPassMS(dev, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_CLEAR);
        dev->renderPass_ClearAll =
            _device_createRenderPassMS(dev, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_CLEAR);
    }
    _device_createDescriptorSetLayout(dev);
    _device_setupPipelines(dev);

    _device_create_empty_texture(dev, format, dev->supportedTiling);

#ifdef DEBUG
#if defined(__linux__) && defined(__GLIBC__)
    _linux_register_error_handler();
#endif
#ifdef VKVG_DBG_UTILS
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)dev->cmdPool, "Device Cmd Pool");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)dev->cmd, "Device Cmd Buff");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_FENCE, (uint64_t)dev->fence, "Device Fence");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)dev->renderPass, "RP load img/stencil");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)dev->renderPass_ClearStencil,
                               "RP clear stencil");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)dev->renderPass_ClearAll, "RP clear all");

    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)dev->dslSrc, "DSLayout SOURCE");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)dev->dslFont, "DSLayout FONT");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)dev->dslGrad, "DSLayout GRADIENT");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_PIPELINE_LAYOUT, (uint64_t)dev->pipelineLayout, "PLLayout dev");

#ifndef __APPLE__
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_PIPELINE, (uint64_t)dev->pipelinePolyFill, "PL Poly fill");
#endif
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_PIPELINE, (uint64_t)dev->pipelineClipping, "PL Clipping");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_PIPELINE, (uint64_t)dev->pipe_OVER, "PL draw Over");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_PIPELINE, (uint64_t)dev->pipe_SUB, "PL draw Substract");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_PIPELINE, (uint64_t)dev->pipe_CLEAR, "PL draw Clear");

    vkh_image_set_name(dev->emptyImg, "empty IMG");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)vkh_image_get_view(dev->emptyImg),
                               "empty IMG VIEW");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_SAMPLER, (uint64_t)vkh_image_get_sampler(dev->emptyImg),
                               "empty IMG SAMPLER");
#endif
#endif
    dev->status = VKVG_STATUS_SUCCESS;
}

#define _CHECK_INST_EXT(ext)                                                                                           \
    if (vkh_instance_extension_supported(#ext)) {                                                                      \
        if (pExtensions)                                                                                               \
            pExtensions[*pExtCount] = #ext;                                                                            \
        (*pExtCount)++;                                                                                                \
    }
void vkvg_get_required_instance_extensions(const char **pExtensions, uint32_t *pExtCount) {
    *pExtCount = 0;

    vkh_instance_extensions_check_init();

#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    _CHECK_INST_EXT(VK_EXT_debug_utils)
#endif
    _CHECK_INST_EXT(VK_KHR_get_physical_device_properties2)

    vkh_instance_extensions_check_release();
}

bool _get_dev_extension_is_supported(VkExtensionProperties *pExtensionProperties, uint32_t extensionCount,
                                     const char *name) {
    for (uint32_t i = 0; i < extensionCount; i++) {
        if (strcmp(name, pExtensionProperties[i].extensionName) == 0)
            return true;
    }
    return false;
}
#define _CHECK_DEV_EXT(ext)                                                                                            \
    {                                                                                                                  \
        if (_get_dev_extension_is_supported(pExtensionProperties, extensionCount, #ext)) {                             \
            if (pExtensions)                                                                                           \
                pExtensions[*pExtCount] = #ext;                                                                        \
            (*pExtCount)++;                                                                                            \
        }                                                                                                              \
    }

vkvg_status_t vkvg_get_required_device_extensions(VkPhysicalDevice phy, const char **pExtensions, uint32_t *pExtCount) {
    VkExtensionProperties *pExtensionProperties;
    uint32_t               extensionCount;

    *pExtCount = 0;

    VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(phy, NULL, &extensionCount, NULL));
    pExtensionProperties = (VkExtensionProperties *)malloc(extensionCount * sizeof(VkExtensionProperties));
    VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(phy, NULL, &extensionCount, pExtensionProperties));

    // https://vulkan.lunarg.com/doc/view/1.2.162.0/mac/1.2-extensions/vkspec.html#VK_KHR_portability_subset
    _CHECK_DEV_EXT(VK_KHR_portability_subset);
    VkPhysicalDeviceFeatures2 phyFeat2 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

#ifdef VKVG_ENABLE_VK_SCALAR_BLOCK_LAYOUT
    // ensure feature is implemented by driver.
    VkPhysicalDeviceScalarBlockLayoutFeatures scalarBlockLayoutSupport = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES};
    phyFeat2.pNext = &scalarBlockLayoutSupport;
#endif

#ifdef VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
    VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreSupport = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES};
    timelineSemaphoreSupport.pNext = phyFeat2.pNext;
    phyFeat2.pNext                 = &timelineSemaphoreSupport;
#endif

    vkGetPhysicalDeviceFeatures2(phy, &phyFeat2);

#ifdef VKVG_ENABLE_VK_SCALAR_BLOCK_LAYOUT
    if (!scalarBlockLayoutSupport.scalarBlockLayout) {
        LOG(VKVG_LOG_ERR, "CREATE Device failed, vkvg compiled with VKVG_ENABLE_VK_SCALAR_BLOCK_LAYOUT and feature is "
                          "not implemented for physical device.\n");
        return VKVG_STATUS_DEVICE_ERROR;
    }
    _CHECK_DEV_EXT(VK_EXT_scalar_block_layout)
#endif
#ifdef VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
    if (!timelineSemaphoreSupport.timelineSemaphore) {
        LOG(VKVG_LOG_ERR, "CREATE Device failed, VK_SEMAPHORE_TYPE_TIMELINE not supported.\n");
        return VKVG_STATUS_DEVICE_ERROR;
    }
    _CHECK_DEV_EXT(VK_KHR_timeline_semaphore)
#endif

    return VKVG_STATUS_SUCCESS;
}

// enabledFeature12 is guarantied to be the first in pNext chain
const void *vkvg_get_device_requirements(VkPhysicalDeviceFeatures *pEnabledFeatures) {

    pEnabledFeatures->fillModeNonSolid  = VK_TRUE;
    pEnabledFeatures->sampleRateShading = VK_TRUE;
    pEnabledFeatures->logicOp           = VK_TRUE;

    void *pNext = NULL;

#ifdef VK_VERSION_1_2
    static VkPhysicalDeviceVulkan12Features enabledFeatures12 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES
#ifdef VKVG_ENABLE_VK_SCALAR_BLOCK_LAYOUT
        ,
        .scalarBlockLayout = VK_TRUE
#endif
#ifdef VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
        ,
        .timelineSemaphore = VK_TRUE
#endif
    };
    enabledFeatures12.pNext = pNext;
    pNext                   = &enabledFeatures12;
#else
#ifdef VKVG_ENABLE_VK_SCALAR_BLOCK_LAYOUT
    static VkPhysicalDeviceScalarBlockLayoutFeaturesEXT scalarBlockFeat = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES_EXT, .scalarBlockLayout = VK_TRUE};
    scalarBlockFeat.pNext                                                = pNext;
    pNext                                                                = &scalarBlockFeat;
#endif
#ifdef VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
    static VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timelineSemaFeat = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR, .timelineSemaphore = VK_TRUE};
    timelineSemaFeat.pNext = pNext;
    pNext                  = &timelineSemaFeat;
#endif
#endif

    return pNext;
}

VkvgDevice vkvg_device_create(vkvg_device_create_info_t *info) {
    LOG(VKVG_LOG_INFO, "CREATE Device\n");
    if (!info) {
        LOG(VKVG_LOG_ERR, "CREATE Device failed, provided vkvg_device_create_info_t is null\n");
        return (VkvgDevice)&_vkvg_status_invalid_dev_ci;
    }
    VkvgDevice dev = (vkvg_device *)calloc(1, sizeof(vkvg_device));
    if (!dev) {
        LOG(VKVG_LOG_ERR, "CREATE Device failed, no memory\n");
        return (VkvgDevice)&_vkvg_status_no_memory;
    }

    dev->references = 1;

    dev->threadAware = info->threadAware;
    if (dev->threadAware) {
        mtx_init(&dev->mutex, mtx_plain);
        mtx_init(&dev->fontCache->mutex, mtx_plain);
        dev->threadAware = true;
    }

    if (!info->vkdev) {
        const char *enabledExts[10];
        const char *enabledLayers[10];
        uint32_t    enabledExtsCount = 0, enabledLayersCount = 0, phyCount = 0;

        vkh_layers_check_init();

#ifdef VKVG_USE_VALIDATION
        if (vkh_layer_is_present("VK_LAYER_KHRONOS_validation"))
            enabledLayers[enabledLayersCount++] = "VK_LAYER_KHRONOS_validation";
#endif

#ifdef VKVG_USE_RENDERDOC
        if (vkh_layer_is_present("VK_LAYER_RENDERDOC_Capture"))
            enabledLayers[enabledLayersCount++] = "VK_LAYER_RENDERDOC_Capture";
#endif
        vkh_layers_check_release();

        vkvg_get_required_instance_extensions(enabledExts, &enabledExtsCount);

#ifdef VK_VERSION_1_2
        VkhApp app = vkh_app_create(1, 2, "vkvg", enabledLayersCount, enabledLayers, enabledExtsCount, enabledExts);
#else
        VkhApp app = vkh_app_create(1, 1, "vkvg", enabledLayersCount, enabledLayers, enabledExtsCount, enabledExts);
#endif

#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
        vkh_app_enable_debug_messenger(app, VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, NULL);
#endif

        VkhPhyInfo *phys = vkh_app_get_phyinfos(app, &phyCount, VK_NULL_HANDLE);
        if (phyCount == 0) {
            dev->status = VKVG_STATUS_DEVICE_ERROR;
            vkh_app_destroy(app);
            return dev;
        }

        VkhPhyInfo pi = 0;
        if (!_device_try_get_phyinfo(phys, phyCount, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, &pi))
            if (!_device_try_get_phyinfo(phys, phyCount, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, &pi))
                pi = phys[0];

        if (!info->samples) // if not set, default to 1 sample
            info->samples = VK_SAMPLE_COUNT_1_BIT;

        if (!(pi->properties.limits.framebufferColorSampleCounts & info->samples)) {
            LOG(VKVG_LOG_ERR, "CREATE Device failed: sample count not supported: %d\n", info->samples);
            dev->status = VKVG_STATUS_DEVICE_ERROR;
            vkh_app_free_phyinfos(phyCount, phys);
            vkh_app_destroy(app);
            return dev;
        }

        uint32_t                qCount        = 0;
        float                   qPriorities[] = {0.0};
        VkDeviceQueueCreateInfo pQueueInfos[] = {{0}, {0}, {0}};

        if (vkh_phyinfo_create_queues(pi, pi->gQueue, 1, qPriorities, &pQueueInfos[qCount]))
            qCount++;

        enabledExtsCount = 0;

        if (vkvg_get_required_device_extensions(pi->phy, enabledExts, &enabledExtsCount) != VKVG_STATUS_SUCCESS) {
            dev->status = VKVG_STATUS_DEVICE_ERROR;
            vkh_app_free_phyinfos(phyCount, phys);
            vkh_app_destroy(app);
            return dev;
        }

        VkPhysicalDeviceFeatures enabledFeatures = {0};
        const void              *pNext           = vkvg_get_device_requirements(&enabledFeatures);

        VkDeviceCreateInfo device_info = {.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                          .queueCreateInfoCount    = qCount,
                                          .pQueueCreateInfos       = (VkDeviceQueueCreateInfo *)&pQueueInfos,
                                          .enabledExtensionCount   = enabledExtsCount,
                                          .ppEnabledExtensionNames = enabledExts,
                                          .pEnabledFeatures        = &enabledFeatures,
                                          .pNext                   = pNext};
        dev->vkhDev                    = vkh_device_create(app, pi, &device_info);

        vkh_app_free_phyinfos(phyCount, phys);

        info->inst    = vkh_app_get_inst(app);
        info->phy     = vkh_device_get_phy(dev->vkhDev);
        info->vkdev   = vkh_device_get_vkdev(dev->vkhDev);
        info->qFamIdx = pi->gQueue;
        info->qIndex  = 0;
    }

    _device_init(dev, info);

    return dev;
}

void vkvg_device_destroy(VkvgDevice dev) {
    if (vkvg_device_status(dev)) {
        LOG(VKVG_LOG_ERR, "DESTROY Device failed, see Status for info");
        return;
    }
    LOCK_DEVICE
    dev->references--;
    if (dev->references > 0) {
        UNLOCK_DEVICE
        return;
    }
    UNLOCK_DEVICE

    LOG(VKVG_LOG_INFO, "DESTROY Device\n");

    if (dev->cachedContextCount > 0) {
        _cached_ctx *cur = dev->cachedContextLast;
        while (cur) {
            assert(cur->ctx->status == VKVG_STATUS_IN_CACHE);
            cur->ctx->status = VKVG_STATUS_SUCCESS;
            _release_context_ressources(cur->ctx);
            _cached_ctx *prev = cur;
            cur               = cur->pNext;
            free(prev);
        }
    }

    vkDeviceWaitIdle(dev->vkDev);

    vkh_image_destroy(dev->emptyImg);

    vkDestroyDescriptorSetLayout(dev->vkDev, dev->dslGrad, NULL);
    vkDestroyDescriptorSetLayout(dev->vkDev, dev->dslFont, NULL);
    vkDestroyDescriptorSetLayout(dev->vkDev, dev->dslSrc, NULL);
#ifndef __APPLE__
    vkDestroyPipeline(dev->vkDev, dev->pipelinePolyFill, NULL);
#endif
    vkDestroyPipeline(dev->vkDev, dev->pipelineClipping, NULL);

    vkDestroyPipeline(dev->vkDev, dev->pipe_OVER, NULL);
    vkDestroyPipeline(dev->vkDev, dev->pipe_SUB, NULL);
    vkDestroyPipeline(dev->vkDev, dev->pipe_CLEAR, NULL);

#ifdef VKVG_WIRED_DEBUG
    vkDestroyPipeline(dev->vkDev, dev->pipelineWired, NULL);
    vkDestroyPipeline(dev->vkDev, dev->pipelineLineList, NULL);
#endif

    vkDestroyPipelineLayout(dev->vkDev, dev->pipelineLayout, NULL);
    vkDestroyPipelineCache(dev->vkDev, dev->pipelineCache, NULL);
    vkDestroyRenderPass(dev->vkDev, dev->renderPass, NULL);
    vkDestroyRenderPass(dev->vkDev, dev->renderPass_ClearStencil, NULL);
    vkDestroyRenderPass(dev->vkDev, dev->renderPass_ClearAll, NULL);

    vkWaitForFences(dev->vkDev, 1, &dev->fence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(dev->vkDev, dev->fence, NULL);

    vkFreeCommandBuffers(dev->vkDev, dev->cmdPool, 1, &dev->cmd);
    vkDestroyCommandPool(dev->vkDev, dev->cmdPool, NULL);

    vkh_queue_destroy(dev->gQueue);

    _font_cache_destroy(dev);

#ifdef VKH_USE_VMA
    vmaDestroyAllocator(dev->allocator);
#endif

    if (dev->threadAware) {
        mtx_destroy(&dev->mutex);
        mtx_destroy(&dev->fontCache->mutex);
    }

    if (dev->vkhDev) {
        VkhApp app = vkh_device_get_app(dev->vkhDev);
        vkh_device_destroy(dev->vkhDev);
        vkh_app_destroy(app);
    }

    free(dev);
}

vkvg_status_t vkvg_device_status(VkvgDevice dev) { return !dev ? VKVG_STATUS_NULL_POINTER : dev->status; }
VkvgDevice    vkvg_device_reference(VkvgDevice dev) {
    if (!vkvg_device_status(dev)) {
        LOCK_DEVICE
        dev->references++;
        UNLOCK_DEVICE
    }
    return dev;
}
uint32_t vkvg_device_get_reference_count(VkvgDevice dev) { return vkvg_device_status(dev) ? 0 : dev->references; }
// TODO dpy reorganisation
void vkvg_device_set_dpy(VkvgDevice dev, int hdpy, int vdpy) {
    if (vkvg_device_status(dev))
        return;
    dev->hdpi = hdpy;
    dev->vdpi = vdpy;

    // TODO: reset font cache
}
void vkvg_device_get_dpy(VkvgDevice dev, int *hdpy, int *vdpy) {
    if (vkvg_device_status(dev))
        return;
    *hdpy = dev->hdpi;
    *vdpy = dev->vdpi;
}
#if VKVG_DBG_STATS
vkvg_debug_stats_t vkvg_device_get_stats(VkvgDevice dev) {
    return vkvg_device_status(dev) ? (vkvg_debug_stats_t){0} : dev->debug_stats;
}
void vkvg_device_reset_stats(VkvgDevice dev) {
    if (vkvg_device_status(dev))
        return;
    dev->debug_stats = (vkvg_debug_stats_t){0};
}
#endif

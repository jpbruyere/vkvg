// Copyright (c) 2018-2026 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
#ifndef SURFACE_INTERNAL_H
#define SURFACE_INTERNAL_H

#include "vkvg_internal.h"
#include "vkvg.h"
#include "vkh.h"
#include "vkh_buffer.h"

#define MAX_GRAD 5

typedef struct _surface_class {
    atomic_int      references;
    VkvgDevice      dev;
    VkFormat        format;
    VkSampleCountFlags samples;             /**< samples count */
    VkRenderPass    renderPass;                /**< Vulkan render pass, common for all surfaces */
    VkRenderPass    renderPass_ClearStencil;   /**< Vulkan render pass for first draw with context, stencil has to be cleared */
    VkRenderPass    renderPass_ClearAll;       /**< Vulkan render pass for new surface, clear all attacments*/
    VkPipeline      pipe_OVER;                 /**< default operator */
    VkPipeline      pipe_SUB;
    VkPipeline      pipe_CLEAR;                /**< clear operator */
    VkPipeline      pipelinePolyFill;          /**< even-odd polygon filling first step */
    VkPipeline      pipelineClipping;          /**< draw on stencil to update clipping regions */
} surface_class;

typedef struct _vkvg_surface_t {
    vkvg_status_t   status; /**< Current status of surface, affected by last operation */
    atomic_int      references;
    VkvgDevice      dev;
    uint32_t        width;
    uint32_t        height;
    VkFramebuffer   fb;
    VkhImage        img;
    VkhImage        imgMS;
    VkhImage        stencil;
    VkCommandPool   cmdPool; // local pools ensure thread safety
    VkCommandBuffer cmd;     // surface local command buffer.
    bool            newSurf;
    mtx_t           mutex;
#ifdef VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
    VkSemaphore timeline; /**< Timeline semaphore */
    uint64_t    timelineStep;
#else
    VkFence             flushFence; // unsignaled idle.
#endif
    int hdpi,                       // only used for FreeType fonts and svg loading */
        vdpi;
    surface_class      *surfClass;
    VkDescriptorPool    descPool;
    VkDescriptorSet     dsGradient;
    vkh_buffer_t        uboGrad;    // uniform buff obj holdings gradient infos
    uint32_t            gradCount;  // recorded grad draws in current command
} vkvg_surface;

#define LOCK_SURFACE(surf)                                                                                             \
    if (surf->dev->threadAware) {                                                                                      \
        mtx_lock(&surf->mutex);                                                                                        \
    }
#define UNLOCK_SURFACE(surf)                                                                                           \
    if (surf->dev->threadAware) {                                                                                      \
        mtx_unlock(&surf->mutex);                                                                                      \
    }

void        _explicit_ms_resolve(VkvgSurface surf);
void        _transition_surf_images(VkvgSurface surf);
void        _clear_surface(VkvgSurface surf, VkImageAspectFlags aspect);
void        _create_surface_main_image(VkvgSurface surf);
void        _create_surface_secondary_images(VkvgSurface surf);
void        _create_framebuffer(VkvgSurface surf);
void        _create_surface_images(VkvgSurface surf);
VkvgSurface _create_surface(VkvgDevice dev, VkFormat format, VkSampleCountFlags samples);
void        _surface_submit_cmd(VkvgSurface surf);

void        _destroy_surface_class (surface_class* surf);
// bool _surface_wait_cmd (VkvgSurface surf);
#endif

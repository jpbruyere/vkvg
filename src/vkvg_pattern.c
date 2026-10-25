// Copyright (c) 2018-2026 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
#include "vkvg_pattern.h"
#include "vkvg_device_internal.h"
#include "vkvg_surface_internal.h"

void _update_descriptor (surface_pattern_t* pat) {
    VkDescriptorImageInfo diiImg = vkh_image_get_descriptor(pat->surf->img, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VkDescriptorImageInfo diiSampler = {pat->sampler};
    VkWriteDescriptorSet  writeDescriptorSet[] = {
        {
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = pat->dsSource,
            .dstBinding      = 0,
            .descriptorCount = 1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo      = &diiImg
        },{
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = pat->dsSource,
            .dstBinding      = 1,
            .descriptorCount = 1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo      = &diiSampler
        }
    };
    vkUpdateDescriptorSets(pat->surf->dev->vkDev, 2, writeDescriptorSet, 0, NULL);
}
void _surface_pattern_init (VkvgDevice dev, surface_pattern_t* pat) {
    const VkDescriptorPoolSize descriptorPoolSize[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1},
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1}
    };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {.sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                                           .maxSets = 1,
                                                           .flags   = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                                                           .poolSizeCount = 2,
                                                           .pPoolSizes    = descriptorPoolSize};
    VK_CHECK_RESULT(vkCreateDescriptorPool(dev->vkDev, &descriptorPoolCreateInfo, NULL, &pat->descPool));

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                                             .descriptorPool     = pat->descPool,
                                                             .descriptorSetCount = 1,
                                                             .pSetLayouts        = &dev->dslSrc};
    VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &pat->dsSource));

#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    vkh_device_set_object_name(dev->vkhDev, VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64_t)pat->descPool,
                               "Pattern Descriptor Pool");
    vkh_device_set_object_name(dev->vkhDev, VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)pat->dsSource, "Pattern DescSet");
#endif
}
void _create_sampler(surface_pattern_t* pat) {
    if (pat->sampler != VK_NULL_HANDLE)
        vkDestroySampler(pat->surf->dev->vkDev, pat->sampler, NULL);
    VkSamplerAddressMode addrMode = 0;
    VkFilter             filter   = VK_FILTER_NEAREST;
    switch (pat->base.extend) {
    case VKVG_EXTEND_NONE:
        addrMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        break;
    case VKVG_EXTEND_PAD:
        addrMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        break;
    case VKVG_EXTEND_REPEAT:
        addrMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        break;
    case VKVG_EXTEND_REFLECT:
        addrMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        break;
    }
    switch (pat->base.filter) {
    case VKVG_FILTER_BILINEAR:
    case VKVG_FILTER_BEST:
        filter = VK_FILTER_LINEAR;
        break;
    default:
        filter = VK_FILTER_NEAREST;
        break;
    }
    VkSamplerCreateInfo samplerCreateInfo = {.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                                             .maxAnisotropy = 1.0,
                                             .addressModeU  = addrMode,
                                             .addressModeV  = addrMode,
                                             .addressModeW  = addrMode,
                                             .magFilter     = filter,
                                             .minFilter     = filter,
                                             .mipmapMode    = VK_SAMPLER_MIPMAP_MODE_NEAREST};
    VK_CHECK_RESULT(vkCreateSampler(pat->surf->dev->vkDev, &samplerCreateInfo, NULL, &pat->sampler));
    pat->samplerIsDirty = false;
}
void _surface_pattern_update_sampler (surface_pattern_t* pat) {
    _create_sampler(pat);
    VkDescriptorImageInfo diiSampler = {pat->sampler};
    VkWriteDescriptorSet  writeDescriptorSet = {
             .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
             .dstSet          = pat->dsSource,
             .dstBinding      = 1,
             .descriptorCount = 1,
             .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
             .pImageInfo      = &diiSampler
    };
    vkUpdateDescriptorSets(pat->surf->dev->vkDev, 1, &writeDescriptorSet, 0, NULL);
}

VkvgPattern vkvg_pattern_create_solid(float r, float g, float b, float a) {
    VkvgPattern pat = (vkvg_pattern_t*)calloc(1, sizeof(vkvg_pattern_t));
    if (!pat) {
        return (VkvgPattern)&_vkvg_status_no_memory;
    }
    pat->type   = VKVG_PATTERN_TYPE_SOLID;
    pat->extend = VKVG_EXTEND_NONE;

    atomic_init(&pat->references, 1);

    return pat;
}
VkvgPattern vkvg_pattern_create_for_surface(VkvgSurface surf) {
    surface_pattern_t* pat = (surface_pattern_t*)calloc(1, sizeof(surface_pattern_t));
    if (!pat) {
        return (VkvgPattern)&_vkvg_status_no_memory;
    }

    pat->base.type      = VKVG_PATTERN_TYPE_SURFACE;
    pat->base.extend    = VKVG_EXTEND_NONE;
    pat->surf           = surf;

    vkvg_surface_reference(surf);
    if (vkvg_surface_status(surf)) {
        pat->base.status = VKVG_STATUS_INVALID_SURFACE;
    } else {
        _surface_pattern_init(surf->dev, pat);

        _create_sampler(pat);
        _update_descriptor(pat);
    }
    atomic_init(&pat->base.references, 1);

    return (VkvgPattern)pat;
}
VkvgPattern vkvg_pattern_create_linear(float x0, float y0, float x1, float y1) {
    vkvg_gradient_pattern_t* pat = (vkvg_gradient_pattern_t*)calloc(1, sizeof(vkvg_gradient_pattern_t));
    if (!pat) {
        return (VkvgPattern)&_vkvg_status_no_memory;
    }
    pat->base.type   = VKVG_PATTERN_TYPE_LINEAR;
    pat->base.extend = VKVG_EXTEND_NONE;

    pat->gradient.cp[0] = (vec4){{x0}, {y0}, {x1}, {y1}};

    atomic_init(&pat->base.references, 1);

    return (VkvgPattern)pat;
}
VkvgPattern vkvg_pattern_create_radial(float cx0, float cy0, float radius0, float cx1, float cy1, float radius1) {
    vkvg_gradient_pattern_t* pat = (vkvg_gradient_pattern_t*)calloc(1, sizeof(vkvg_gradient_pattern_t));
    if (!pat) {
        return (VkvgPattern)&_vkvg_status_no_memory;
    }
    pat->base.type   = VKVG_PATTERN_TYPE_RADIAL;
    pat->base.extend = VKVG_EXTEND_NONE;

    vkvg_pattern_edit_radial((VkvgPattern)pat, cx0, cy0, radius0, cx1, cy1, radius1);

    atomic_init(&pat->base.references, 1);

    return (VkvgPattern)pat;
}
VkvgPattern vkvg_pattern_reference(VkvgPattern pat) {
    if (!vkvg_pattern_status(pat))
        atomic_fetch_add_explicit(&pat->references, 1, memory_order_relaxed);
    return pat;
}
uint32_t vkvg_pattern_get_reference_count(VkvgPattern pat) {
    if (vkvg_pattern_status(pat))
        return 0;
    return atomic_load_explicit(&pat->references, memory_order_relaxed);
}

vkvg_status_t vkvg_pattern_get_linear_points(VkvgPattern pat, float* x0, float* y0, float* x1, float* y1) {
    if (vkvg_pattern_status(pat))
        return vkvg_pattern_status(pat);
    if (pat->type != VKVG_PATTERN_TYPE_LINEAR)
        return VKVG_STATUS_PATTERN_TYPE_MISMATCH;

    vkvg_gradient_t* grad = &((vkvg_gradient_pattern_t*)pat)->gradient;

    *x0 = grad->cp[0].x;
    *y0 = grad->cp[0].y;
    *x1 = grad->cp[0].z;
    *y1 = grad->cp[0].w;
    return VKVG_STATUS_SUCCESS;
}
vkvg_status_t vkvg_pattern_edit_linear(VkvgPattern pat, float x0, float y0, float x1, float y1) {
    if (vkvg_pattern_status(pat))
        return vkvg_pattern_status(pat);
    if (pat->type != VKVG_PATTERN_TYPE_LINEAR)
        return VKVG_STATUS_PATTERN_TYPE_MISMATCH;


    vkvg_gradient_t* grad = &((vkvg_gradient_pattern_t*)pat)->gradient;
    grad->cp[0] = (vec4){{x0}, {y0}, {x1}, {y1}};
    return VKVG_STATUS_SUCCESS;
}

vkvg_status_t vkvg_pattern_edit_radial(VkvgPattern pat, float cx0, float cy0, float radius0, float cx1, float cy1,
                                       float radius1) {
    if (vkvg_pattern_status(pat))
        return vkvg_pattern_status(pat);
    if (pat->type != VKVG_PATTERN_TYPE_RADIAL)
        return VKVG_STATUS_PATTERN_TYPE_MISMATCH;

    vkvg_gradient_t* grad = &((vkvg_gradient_pattern_t*)pat)->gradient;

    vec2 c0 = {cx0, cy0};
    vec2 c1 = {cx1, cy1};

    if (radius0 > radius1 - 1.0f)
        radius0 = radius1 - 1.0f;
    vec2  u = vec2_sub(c0, c1);
    float l = vec2_length(u);
    if (l + radius0 + 1.0f >= radius1) {
        vec2 v = vec2_div_s(u, l);
        c0     = vec2_add(c1, vec2_mult_s(v, radius1 - radius0 - 1.0f));
    }

    grad->cp[0] = (vec4){{c0.x}, {c0.y}, {radius0}, {0}};
    grad->cp[1] = (vec4){{c1.x}, {c1.y}, {radius1}, {0}};

    LOG(VKVG_LOG_INFO, "vkvg_pattern_edit_radial: %p status: %d\n", pat, pat->status);

    return VKVG_STATUS_SUCCESS;
}
vkvg_status_t vkvg_pattern_add_color_stop(VkvgPattern pat, float offset, float r, float g, float b, float a) {
    if (vkvg_pattern_status(pat))
        return vkvg_pattern_status(pat);
    if (pat->type == VKVG_PATTERN_TYPE_SURFACE || pat->type == VKVG_PATTERN_TYPE_SOLID)
        return VKVG_STATUS_PATTERN_TYPE_MISMATCH;

    vkvg_gradient_t* grad = &((vkvg_gradient_pattern_t*)pat)->gradient;
#ifdef VKVG_PREMULT_ALPHA
    vkvg_color_t c = {a * r, a * g, a * b, a};
#else
    vkvg_color_t c = {r, g, b, a};
#endif
    grad->colors[grad->count] = c;
#ifdef VKVG_ENABLE_VK_SCALAR_BLOCK_LAYOUT
    grad->stops[grad->count] = offset;
#else
    grad->stops[grad->count].r = offset;
#endif
    grad->count++;
    return VKVG_STATUS_SUCCESS;
}
void vkvg_pattern_set_extend(VkvgPattern pat, vkvg_extend_t extend) {
    if (vkvg_pattern_status(pat) || pat->extend == extend)
        return;
    pat->extend = extend;
    if (pat->type == VKVG_PATTERN_TYPE_SURFACE)
        ((surface_pattern_t*)pat)->samplerIsDirty = true;
}
void vkvg_pattern_set_filter(VkvgPattern pat, vkvg_filter_t filter) {
    if (vkvg_pattern_status(pat) || pat->filter == filter)
        return;
    pat->filter = filter;
    if (pat->type == VKVG_PATTERN_TYPE_SURFACE)
        ((surface_pattern_t*)pat)->samplerIsDirty = true;
}

vkvg_extend_t vkvg_pattern_get_extend(VkvgPattern pat) {
    if (vkvg_pattern_status(pat))
        return (vkvg_extend_t)0;
    return pat->extend;
}
vkvg_filter_t vkvg_pattern_get_filter(VkvgPattern pat) {
    if (vkvg_pattern_status(pat))
        return (vkvg_filter_t)0;
    return pat->filter;
}
vkvg_pattern_type_t vkvg_pattern_get_type(VkvgPattern pat) {
    if (vkvg_pattern_status(pat))
        return (vkvg_pattern_type_t)0;
    return pat->type;
}
vkvg_status_t vkvg_pattern_get_color_stop_count(VkvgPattern pat, uint32_t* count) {
    if (vkvg_pattern_status(pat))
        return vkvg_pattern_status(pat);
    if (pat->type == VKVG_PATTERN_TYPE_SURFACE || pat->type == VKVG_PATTERN_TYPE_SOLID)
        return VKVG_STATUS_PATTERN_TYPE_MISMATCH;
    vkvg_gradient_t* grad = &((vkvg_gradient_pattern_t*)pat)->gradient;
    *count                = grad->count;
    return VKVG_STATUS_SUCCESS;
}
vkvg_status_t vkvg_pattern_get_color_stop_rgba(VkvgPattern pat, uint32_t index, float* offset, float* r, float* g,
                                               float* b, float* a) {
    if (vkvg_pattern_status(pat))
        return vkvg_pattern_status(pat);
    if (pat->type == VKVG_PATTERN_TYPE_SURFACE || pat->type == VKVG_PATTERN_TYPE_SOLID)
        return VKVG_STATUS_PATTERN_TYPE_MISMATCH;
    vkvg_gradient_t* grad = &((vkvg_gradient_pattern_t*)pat)->gradient;
    if (index >= grad->count)
        return VKVG_STATUS_INVALID_INDEX;
#ifdef VKVG_ENABLE_VK_SCALAR_BLOCK_LAYOUT
    *offset = grad->stops[index];
#else
    *offset = grad->stops[index].r;
#endif
    vkvg_color_t c = grad->colors[index];
    *r             = c.r;
    *g             = c.g;
    *b             = c.b;
    *a             = c.a;
    return VKVG_STATUS_SUCCESS;
}
void vkvg_pattern_set_matrix(VkvgPattern pat, const vkvg_matrix_t* matrix) {
    if (vkvg_pattern_status(pat))
        return;
    pat->matrix    = *matrix;
    pat->hasMatrix = true;
}
void vkvg_pattern_get_matrix(VkvgPattern pat, vkvg_matrix_t* matrix) {
    if (vkvg_pattern_status(pat))
        return;
    if (pat->hasMatrix)
        *matrix = pat->matrix;
    else
        *matrix = VKVG_IDENTITY_MATRIX;
}
vkvg_status_t vkvg_pattern_status(VkvgPattern pat) {
    return !pat ? VKVG_STATUS_NULL_POINTER : pat->status;
}
void vkvg_pattern_destroy(VkvgPattern pat) {
    if (vkvg_pattern_status(pat))
        return;

    if (atomic_fetch_sub_explicit(&pat->references, 1, memory_order_acq_rel) != 1)
        return;

    if (pat->type == VKVG_PATTERN_TYPE_SURFACE) {
        surface_pattern_t* sp = (surface_pattern_t*)pat;
        vkDestroyDescriptorPool(sp->surf->dev->vkDev, sp->descPool, NULL);
        vkDestroySampler(sp->surf->dev->vkDev, sp->sampler, NULL);
        vkvg_surface_destroy(sp->surf);
    }

    free(pat);
    pat = NULL;
}

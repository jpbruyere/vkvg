#include "vkvg_surface_internal.h"
#include "vkvg_context_internal.h"
#include "vkvg_device_internal.h"
#include "vkvg_pattern.h"

VkvgPattern _init_pattern (VkvgDevice dev){
    VkvgPattern pat = (vkvg_pattern*)calloc(1,sizeof(vkvg_pattern));
    pat->dev = dev;
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                      .descriptorPool = dev->descriptorPool,
                                      .descriptorSetCount = 1,
                                      .pSetLayouts = &dev->dslFont };
    VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &pat->descriptorSet));
    return pat;
}

void _update_descSet (VkvgPattern pat){
    _font_cache_t* cache = pat->dev->fontCache;
    VkDescriptorImageInfo descFontTex   = vkh_image_get_descriptor (cache->cacheTex,VK_IMAGE_LAYOUT_GENERAL);
    VkDescriptorImageInfo descSrcTex    = vkh_image_get_descriptor (pat->img,       VK_IMAGE_LAYOUT_GENERAL);

    VkWriteDescriptorSet writeDescriptorSet[] = {
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = pat->descriptorSet,
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &descFontTex
        },{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = pat->descriptorSet,
            .dstBinding = 1,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &descSrcTex
        }};
    vkUpdateDescriptorSets(pat->dev->vkDev, 2, &writeDescriptorSet, 0, NULL);
}

VkvgPattern vkvg_pattern_create(VkvgDevice dev){
    VkvgPattern pat = _init_pattern (dev);
    return pat;
}

VkvgPattern vkvg_pattern_create_for_surface (VkvgSurface surf){
    VkvgPattern pat = _init_pattern (surf->dev);
    pat->img = surf->img;
    return pat;
}
VkvgPattern vkvg_pattern_create_linear (float x0, float y0, float x1, float y1){

}
VkvgPattern vkvg_pattern_create_radial (float cx0, float cy0, float radius0,
                                        float cx1, float cy1, float radius1){

}
void vkvg_pattern_set_extend (VkvgPattern pat, vkvg_extend_t extend){
    pat->extend = extend;
}

void vkvg_set_source (VkvgContext ctx, VkvgPattern pat){
    _update_descSet (pat);

    vkCmdBindDescriptorSets(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pat->dev->pipelineLayout,
                            0, 1, &pat->descriptorSet, 0, NULL);
}

void vkvg_pattern_destroy(VkvgPattern pat)
{
    vkFreeDescriptorSets(pat->dev, pat->dev->descriptorPool, 1, &pat->descriptorSet);

    free(pat);
}


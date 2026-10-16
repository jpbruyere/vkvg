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

#include "vkvg_device_internal.h"
#include "vkvg_surface_internal.h"
#include "vkvg_context_internal.h"

#include "vkh_image.h"
#include "vkh_queue.h"

#include "shaders.h"

void _explicit_ms_resolve(VkvgSurface surf) {
    LOCK_SURFACE(surf)

    VkCommandBuffer cmd = surf->cmd;

    vkh_cmd_begin(cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkh_image_set_layout(cmd, surf->imgMS, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT);
    vkh_image_set_layout(cmd, surf->img, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkImageResolve re = {.extent         = {surf->width, surf->height, 1},
                         .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                         .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}};

    vkCmdResolveImage(cmd, vkh_image_get_vkimage(surf->imgMS), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      vkh_image_get_vkimage(surf->img), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &re);
    vkh_image_set_layout(cmd, surf->imgMS, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    vkh_cmd_end(cmd);

    _surface_submit_cmd(surf);

    UNLOCK_SURFACE(surf)
}
void _transition_surf_images(VkvgSurface surf) {
    LOCK_SURFACE(surf)
    VkvgDevice dev = surf->dev;

    //_surface_wait_cmd (surf);

    vkh_cmd_begin(surf->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VkhImage imgMs = surf->imgMS;
    if (imgMs != NULL)
        vkh_image_set_layout(surf->cmd, imgMs, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    vkh_image_set_layout(surf->cmd, surf->img, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    vkh_image_set_layout(surf->cmd, surf->stencil, dev->stencilAspectFlag, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
    vkh_cmd_end(surf->cmd);

    _surface_submit_cmd(surf);

    UNLOCK_SURFACE(surf)
}
void _clear_surface(VkvgSurface surf, VkImageAspectFlags aspect) {
    LOCK_SURFACE(surf)

    VkCommandBuffer cmd = surf->cmd;

    vkh_cmd_begin(cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    if (aspect & VK_IMAGE_ASPECT_COLOR_BIT) {
        VkClearColorValue       cclr  = {{0, 0, 0, 0}};
        VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        VkhImage img = surf->imgMS;
        if (surf->samples == VK_SAMPLE_COUNT_1_BIT)
            img = surf->img;

        vkh_image_set_layout(cmd, img, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT);

        vkCmdClearColorImage(cmd, vkh_image_get_vkimage(img), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &cclr, 1, &range);

        vkh_image_set_layout(cmd, img, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }
    if (aspect & VK_IMAGE_ASPECT_STENCIL_BIT) {
        VkClearDepthStencilValue clr   = {0, 0};
        VkImageSubresourceRange  range = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1};

        vkh_image_set_layout(cmd, surf->stencil, VK_IMAGE_ASPECT_STENCIL_BIT,
                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        vkCmdClearDepthStencilImage(cmd, vkh_image_get_vkimage(surf->stencil), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    &clr, 1, &range);

        vkh_image_set_layout(cmd, surf->stencil, VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
    }
    vkh_cmd_end(cmd);

    _surface_submit_cmd(surf);

    UNLOCK_SURFACE(surf)
}
VkRenderPass _createRenderPassNoResolve(VkvgSurface surf, VkAttachmentLoadOp loadOp,
                                               VkAttachmentLoadOp stencilLoadOp) {
    VkAttachmentDescription attColor = {.format         = FB_COLOR_FORMAT,
                                        .samples        = surf->samples,
                                        .loadOp         = loadOp,
                                        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
                                        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                        .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                        .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentDescription attDS    = {.format         = surf->dev->stencilFormat,
                                        .samples        = surf->samples,
                                        .loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                        .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                        .stencilLoadOp  = stencilLoadOp,
                                        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
                                        .initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                        .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkAttachmentDescription attachments[] = {attColor, attDS};
    VkAttachmentReference   colorRef      = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference   dsRef         = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpassDescription = {.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
                                               .colorAttachmentCount    = 1,
                                               .pColorAttachments       = &colorRef,
                                               .pDepthStencilAttachment = &dsRef};

    VkSubpassDependency dependencies[] = {
                                          {VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                           VK_DEPENDENCY_BY_REGION_BIT},
                                          {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                           VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
                                           VK_DEPENDENCY_BY_REGION_BIT},
                                          };

    VkRenderPassCreateInfo renderPassInfo = {.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                             .attachmentCount = 2,
                                             .pAttachments    = attachments,
                                             .subpassCount    = 1,
                                             .pSubpasses      = &subpassDescription,
                                             .dependencyCount = 2,
                                             .pDependencies   = dependencies};
    VkRenderPass           rp;
    VK_CHECK_RESULT(vkCreateRenderPass(surf->dev->vkDev, &renderPassInfo, NULL, &rp));
    return rp;
}
VkRenderPass _createRenderPassMS(VkvgSurface surf, VkAttachmentLoadOp loadOp, VkAttachmentLoadOp stencilLoadOp) {
    VkAttachmentDescription attColor        = {.format         = FB_COLOR_FORMAT,
                                               .samples        = surf->samples,
                                               .loadOp         = loadOp,
                                               .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                               .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentDescription attColorResolve = {.format         = FB_COLOR_FORMAT,
                                               .samples        = VK_SAMPLE_COUNT_1_BIT,
                                               .loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
                                               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
                                               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentDescription attDS           = {.format         = surf->dev->stencilFormat,
                                               .samples        = surf->samples,
                                               .loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                               .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                               .stencilLoadOp  = stencilLoadOp,
                                               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
                                               .initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkAttachmentDescription attachments[] = {attColorResolve, attDS, attColor};
    VkAttachmentReference   resolveRef    = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference   dsRef         = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    VkAttachmentReference   colorRef      = {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpassDescription = {.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
                                               .colorAttachmentCount    = 1,
                                               .pColorAttachments       = &colorRef,
                                               .pResolveAttachments     = &resolveRef,
                                               .pDepthStencilAttachment = &dsRef};

    VkSubpassDependency dependencies[] = {
                                          {VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                           VK_DEPENDENCY_BY_REGION_BIT},
                                          {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                           VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
                                           VK_DEPENDENCY_BY_REGION_BIT},
                                          };

    VkRenderPassCreateInfo renderPassInfo = {.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                             .attachmentCount = 3,
                                             .pAttachments    = attachments,
                                             .subpassCount    = 1,
                                             .pSubpasses      = &subpassDescription,
                                             .dependencyCount = 2,
                                             .pDependencies   = dependencies};
    VkRenderPass           rp;
    VK_CHECK_RESULT(vkCreateRenderPass(surf->dev->vkDev, &renderPassInfo, NULL, &rp));
    return rp;
}
void _setupPipelines(VkvgSurface surf) {
    VkvgDevice dev = surf->dev;
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                                       .renderPass = surf->renderPass};

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
                                                                 .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                                                                 .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN};

    VkPipelineRasterizationStateCreateInfo rasterizationState = {
                                                                 .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                                                                 .polygonMode             = VK_POLYGON_MODE_FILL,
                                                                 .cullMode                = VK_CULL_MODE_NONE,
                                                                 .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                                                 .depthClampEnable        = VK_FALSE,
                                                                 .rasterizerDiscardEnable = VK_FALSE,
                                                                 .depthBiasEnable         = VK_FALSE,
                                                                 .lineWidth               = 1.0f};

    VkPipelineColorBlendAttachmentState blendAttachmentState = {
        .colorWriteMask = 0x0,
        .blendEnable    = VK_TRUE,
#ifdef VKVG_PREMULT_ALPHA
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
#else
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
#endif
    };

    VkPipelineColorBlendStateCreateInfo colorBlendState = {.sType =
                                                           VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                                           .attachmentCount = 1,
                                                           .pAttachments    = &blendAttachmentState};

    /*failOp,passOp,depthFailOp,compareOp, compareMask, writeMask, reference;*/
    VkStencilOpState polyFillOpState = {VK_STENCIL_OP_KEEP,
                                        VK_STENCIL_OP_INVERT,
                                        VK_STENCIL_OP_KEEP,
                                        VK_COMPARE_OP_EQUAL,
                                        STENCIL_CLIP_BIT,
                                        STENCIL_FILL_BIT,
                                        0};
    VkStencilOpState clipingOpState  = {VK_STENCIL_OP_ZERO,
                                        VK_STENCIL_OP_REPLACE,
                                        VK_STENCIL_OP_KEEP,
                                        VK_COMPARE_OP_EQUAL,
                                        STENCIL_FILL_BIT,
                                        STENCIL_ALL_BIT,
                                        0x2};
    VkStencilOpState stencilOpState  = {VK_STENCIL_OP_KEEP,
                                        VK_STENCIL_OP_ZERO,
                                        VK_STENCIL_OP_KEEP,
                                        VK_COMPARE_OP_EQUAL,
                                        STENCIL_FILL_BIT,
                                        STENCIL_FILL_BIT,
                                        0x1};

    VkPipelineDepthStencilStateCreateInfo dsStateCreateInfo = {
                                                               .sType             = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                                                               .depthTestEnable   = VK_FALSE,
                                                               .depthWriteEnable  = VK_FALSE,
                                                               .depthCompareOp    = VK_COMPARE_OP_ALWAYS,
                                                               .stencilTestEnable = VK_TRUE,
                                                               .front             = polyFillOpState,
                                                               .back              = polyFillOpState};

    VkDynamicState dynamicStateEnables[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
        VK_DYNAMIC_STATE_STENCIL_REFERENCE,
        VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                                                     .dynamicStateCount = 2,
                                                     .pDynamicStates    = dynamicStateEnables};

    VkPipelineViewportStateCreateInfo viewportState = {
                                                       .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1};

    VkPipelineMultisampleStateCreateInfo multisampleState = {
                                                             .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};
    /*if (dev->samples != VK_SAMPLE_COUNT_1_BIT){
        multisampleState.sampleShadingEnable = VK_TRUE;
        multisampleState.minSampleShading = 0.5f;
    }*/
    VkVertexInputBindingDescription vertexInputBinding = {
                                                          .binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription vertexInputAttributs[3] = {{0, 0, VK_FORMAT_R32G32_SFLOAT, 0},
                                                                 {1, 0, VK_FORMAT_R8G8B8A8_UNORM, 8},
                                                                 {2, 0, VK_FORMAT_R32G32B32_SFLOAT, 12}};

    VkPipelineVertexInputStateCreateInfo vertexInputState = {
                                                             .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                                                             .vertexBindingDescriptionCount   = 1,
                                                             .pVertexBindingDescriptions      = &vertexInputBinding,
                                                             .vertexAttributeDescriptionCount = 3,
                                                             .pVertexAttributeDescriptions    = vertexInputAttributs};
#ifdef VKVG_WIRED_DEBUG
    VkShaderModule modVert, modFrag, modFragWired;
#else
    VkShaderModule modVert, modFrag;
#endif
    VkShaderModuleCreateInfo createInfo = {.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                           .pCode    = (uint32_t*)vkvg_main_vert_spv,
                                           .codeSize = vkvg_main_vert_spv_len};
    VK_CHECK_RESULT(vkCreateShaderModule(dev->vkDev, &createInfo, NULL, &modVert));
#if defined(VKVG_LCD_FONT_FILTER) && defined(FT_CONFIG_OPTION_SUBPIXEL_RENDERING)
    createInfo.pCode    = (uint32_t*)vkvg_main_lcd_frag_spv;
    createInfo.codeSize = vkvg_main_lcd_frag_spv_len;
#else
    createInfo.pCode    = (uint32_t*)vkvg_main_frag_spv;
    createInfo.codeSize = vkvg_main_frag_spv_len;
#endif
    VK_CHECK_RESULT(vkCreateShaderModule(dev->vkDev, &createInfo, NULL, &modFrag));

    VkPipelineShaderStageCreateInfo vertStage = {
        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage  = VK_SHADER_STAGE_VERTEX_BIT,
        .module = modVert,
        .pName  = "main",
    };
    VkPipelineShaderStageCreateInfo fragStage = {
        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = modFrag,
        .pName  = "main",
    };

           // Use specialization constants to pass number of samples to the shader (used for MSAA resolve)
    /*VkSpecializationMapEntry specializationEntry = {
        .constantID = 0,
        .offset = 0,
        .size = sizeof(uint32_t)};
    uint32_t specializationData = VKVG_SAMPLES;
    VkSpecializationInfo specializationInfo = {
        .mapEntryCount = 1,
        .pMapEntries = &specializationEntry,
        .dataSize = sizeof(specializationData),
        .pData = &specializationData};*/

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertStage, fragStage};

    pipelineCreateInfo.stageCount          = 1;
    pipelineCreateInfo.pStages             = shaderStages;
    pipelineCreateInfo.pVertexInputState   = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pViewportState      = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pMultisampleState   = &multisampleState;
    pipelineCreateInfo.pColorBlendState    = &colorBlendState;
    pipelineCreateInfo.pDepthStencilState  = &dsStateCreateInfo;
    pipelineCreateInfo.pDynamicState       = &dynamicState;
    pipelineCreateInfo.layout              = dev->pipelineLayout;

#ifndef __APPLE__
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL,
                                              &surf->pipelinePolyFill));
#endif

    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    dsStateCreateInfo.back = dsStateCreateInfo.front = clipingOpState;
    dynamicState.dynamicStateCount                   = 5;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL,
                                              &surf->pipelineClipping));

    dsStateCreateInfo.back = dsStateCreateInfo.front = stencilOpState;
    blendAttachmentState.colorWriteMask              = 0xf;
    dynamicState.dynamicStateCount                   = 3;
    pipelineCreateInfo.stageCount                    = 2;
    VK_CHECK_RESULT(
        vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &surf->pipe_OVER));

    blendAttachmentState.alphaBlendOp = blendAttachmentState.colorBlendOp = VK_BLEND_OP_SUBTRACT;
    VK_CHECK_RESULT(
        vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &surf->pipe_SUB));

    colorBlendState.logicOpEnable    = VK_TRUE;
    blendAttachmentState.blendEnable = VK_FALSE;
    colorBlendState.logicOp          = VK_LOGIC_OP_CLEAR;
    VK_CHECK_RESULT(
        vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &surf->pipe_CLEAR));

#ifdef VKVG_WIRED_DEBUG
    colorBlendState.logicOpEnable    = VK_FALSE;
    blendAttachmentState.blendEnable = VK_TRUE;
    colorBlendState.logicOp          = VK_LOGIC_OP_CLEAR;

    createInfo.pCode = (uint32_t*)wired_frag_spv;

    createInfo.codeSize = wired_frag_spv_len;
    VK_CHECK_RESULT(vkCreateShaderModule(dev->vkDev, &createInfo, NULL, &modFragWired));

    shaderStages[1].module = modFragWired;

    rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL,
                                              &dev->pipelineLineList));

    inputAssemblyState.topology    = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    VK_CHECK_RESULT(
        vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipelineWired));

    vkDestroyShaderModule(dev->vkDev, modFragWired, NULL);
#endif

    vkDestroyShaderModule(dev->vkDev, modVert, NULL);
    vkDestroyShaderModule(dev->vkDev, modFrag, NULL);
}

void _create_surface_main_image(VkvgSurface surf) {
    VkhDevice vkhd = (VkhDevice)&surf->dev->vkDev;
    surf->img      = vkh_image_create(vkhd, surf->format, surf->width, surf->height, surf->dev->supportedTiling,
                                      VKH_MEMORY_USAGE_GPU_ONLY,
                                      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                          VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkh_image_create_descriptor(surf->img, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_NEAREST,
                                VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,
                                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    vkh_image_set_name(surf->img, "SURF main color");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)vkh_image_get_view(surf->img),
                               "SURF main color VIEW");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_SAMPLER, (uint64_t)vkh_image_get_sampler(surf->img),
                               "SURF main color SAMPLER");
#endif
}
// create multisample color img if sample count > 1 and the stencil buffer multisampled or not
void _create_surface_secondary_images(VkvgSurface surf) {
    VkhDevice vkhd = (VkhDevice)&surf->dev->vkDev;
    if (surf->samples > VK_SAMPLE_COUNT_1_BIT) {
        surf->imgMS = vkh_image_ms_create(
            vkhd, surf->format, surf->samples, surf->width, surf->height, VKH_MEMORY_USAGE_GPU_ONLY,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        vkh_image_create_descriptor(surf->imgMS, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_NEAREST,
                                    VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,
                                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
        vkh_image_set_name(surf->imgMS, "SURF MS color IMG");
        vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)vkh_image_get_view(surf->imgMS),
                                   "SURF MS color VIEW");
        vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_SAMPLER, (uint64_t)vkh_image_get_sampler(surf->imgMS),
                                   "SURF MS color SAMPLER");
#endif
    }
    surf->stencil = vkh_image_ms_create(vkhd, surf->dev->stencilFormat, surf->samples, surf->width, surf->height,
                                        VKH_MEMORY_USAGE_GPU_ONLY,
                                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkh_image_create_descriptor(surf->stencil, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_STENCIL_BIT, VK_FILTER_NEAREST,
                                VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,
                                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    vkh_image_set_name(surf->stencil, "SURF stencil");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)vkh_image_get_view(surf->stencil),
                               "SURF stencil VIEW");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_SAMPLER, (uint64_t)vkh_image_get_sampler(surf->stencil),
                               "SURF stencil SAMPLER");
#endif
}
void _create_framebuffer(VkvgSurface surf) {
    VkImageView attachments[] = {
        vkh_image_get_view(surf->img),
        vkh_image_get_view(surf->stencil),
        vkh_image_get_view(surf->imgMS),
    };
    VkFramebufferCreateInfo frameBufferCreateInfo = {.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                                     .renderPass      = surf->renderPass,
                                                     .attachmentCount = 3,
                                                     .pAttachments    = attachments,
                                                     .width           = surf->width,
                                                     .height          = surf->height,
                                                     .layers          = 1};
    if (surf->samples == VK_SAMPLE_COUNT_1_BIT)
        frameBufferCreateInfo.attachmentCount = 2;
    else if (surf->dev->deferredResolve) {
        attachments[0]                        = attachments[2];
        frameBufferCreateInfo.attachmentCount = 2;
    }
    VK_CHECK_RESULT(vkCreateFramebuffer(surf->dev->vkDev, &frameBufferCreateInfo, NULL, &surf->fb));
#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    vkh_device_set_object_name((VkhDevice)&surf->dev->vkDev, VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)surf->fb, "SURF FB");
#endif
}
void _create_surface_images(VkvgSurface surf) {

    _create_surface_main_image(surf);
    _create_surface_secondary_images(surf);
    _create_framebuffer(surf);

#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    vkh_image_set_name(surf->img, "surfImg");
    vkh_image_set_name(surf->imgMS, "surfImgMS");
    vkh_image_set_name(surf->stencil, "surfStencil");
#endif
}
VkvgSurface _create_surface(VkvgDevice dev, VkFormat format) {
    LOG(VKVG_LOG_INFO, "CREATE Surface\n");
    if (vkvg_device_status(dev)) {
        LOGE("CREATE Surface failed, invalid Device\n");
        return (VkvgSurface)&_vkvg_status_device_error;
    }

    VkvgSurface surf = (vkvg_surface*)calloc(1, sizeof(vkvg_surface));
    if (!surf) {
        LOGE("CREATE Surface failed, no memory\n");
        return (VkvgSurface)&_vkvg_status_no_memory;
    }

    atomic_init(&surf->references, 1);
    surf->dev     = dev;
    surf->format  = format;
    surf->hdpi    = 72;
    surf->vdpi    = 72;
    surf->samples = VK_SAMPLE_COUNT_1_BIT;

    if (dev->threadAware)
        mtx_init(&surf->mutex, mtx_plain);

    VkhDevice vkhd = (VkhDevice)&surf->dev->vkDev;

    if (dev->deferredResolve || surf->samples == VK_SAMPLE_COUNT_1_BIT) {
        surf->renderPass =
            _createRenderPassNoResolve(surf, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_LOAD);
        surf->renderPass_ClearStencil =
            _createRenderPassNoResolve(surf, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_CLEAR);
        surf->renderPass_ClearAll =
            _createRenderPassNoResolve(surf, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_CLEAR);
    } else {
        surf->renderPass =
            _createRenderPassMS(surf, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_LOAD);
        surf->renderPass_ClearStencil =
            _createRenderPassMS(surf, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_CLEAR);
        surf->renderPass_ClearAll =
            _createRenderPassMS(surf, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_CLEAR);
    }

    _setupPipelines(surf);

    surf->cmdPool =
        vkh_cmd_pool_create(vkhd, dev->gQueue->familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkh_cmd_buffs_create(vkhd, surf->cmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &surf->cmd);

#if VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
    surf->timeline = vkh_timeline_create(vkhd, 0);
#else
    surf->flushFence = vkh_fence_create(vkhd);
#endif

#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)surf->cmd, "vkvgSurfCmd");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)surf->renderPass, "RP load img/stencil");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)surf->renderPass_ClearStencil, "RP clear stencil");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)surf->renderPass_ClearAll, "RP clear all");
#ifndef __APPLE__
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_PIPELINE, (uint64_t)surf->pipelinePolyFill, "PL Poly fill");
#endif
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_PIPELINE, (uint64_t)surf->pipelineClipping, "PL Clipping");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_PIPELINE, (uint64_t)surf->pipe_OVER, "PL draw Over");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_PIPELINE, (uint64_t)surf->pipe_SUB, "PL draw Substract");
    vkh_device_set_object_name(vkhd, VK_OBJECT_TYPE_PIPELINE, (uint64_t)surf->pipe_CLEAR, "PL draw Clear");

#endif
    vkvg_device_reference(surf->dev);
    return surf;
}
// if fence sync, surf mutex must be locked.
/*bool _surface_wait_cmd (VkvgSurface surf) {
    LOG(VKVG_LOG_INFO, "SURF: _surface__wait_flush_fence\n");
#ifdef VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
    if (vkh_timeline_wait ((VkhDevice)surf->dev, surf->timeline, surf->timelineStep) == VK_SUCCESS)
        return true;
#else
    if (WaitForFences (surf->dev->vkDev, 1, &surf->flushFence, VK_TRUE, VKVG_FENCE_TIMEOUT) == VK_SUCCESS) {
        ResetFences (surf->dev->vkDev, 1, &surf->flushFence);
        return true;
    }
#endif
    LOG(VKVG_LOG_DEBUG, "CTX: _wait_flush_fence timeout\n");
    surf->status = VKVG_STATUS_TIMEOUT;
    return false;
}*/
// surface mutex must be locked to call this method, locking to guard also the surf->cmd local buffer usage.
void _surface_submit_cmd(VkvgSurface surf) {
    VkvgDevice dev = surf->dev;
#ifdef VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
    LOCK_DEVICE
    vkh_cmd_submit_timelined(dev->gQueue, &surf->cmd, surf->timeline, surf->timelineStep, surf->timelineStep + 1);
    surf->timelineStep++;
    UNLOCK_DEVICE
    vkh_timeline_wait((VkhDevice)&dev->vkDev, surf->timeline, surf->timelineStep);
#else
    LOCK_DEVICE
    vkh_cmd_submit(surf->dev->gQueue, &surf->cmd, surf->flushFence);
    UNLOCK_DEVICE
    WaitForFences(surf->dev->vkDev, 1, &surf->flushFence, VK_TRUE, VKVG_FENCE_TIMEOUT);
    ResetFences(surf->dev->vkDev, 1, &surf->flushFence);
#endif
}

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

#define GetInstProcAddress(inst, func)(PFN_##func)vkGetInstanceProcAddr(inst, #func);

#include "vkvg_device_internal.h"
#include "vkvg_context_internal.h"
#include "shaders.h"

void _flush_all_contexes (VkvgDevice dev){
    VkvgContext next = dev->lastCtx;
    while (next != NULL){
        _flush_cmd_buff(next);
        next = next->pPrev;
    }
}

void _create_pipeline_cache(VkvgDevice dev){

    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
    VK_CHECK_RESULT(vkCreatePipelineCache(dev->vkDev, &pipelineCacheCreateInfo, NULL, &dev->pipelineCache));
}

VkRenderPass _createRenderPassNoResolve(VkvgDevice dev, VkAttachmentLoadOp loadOp, VkAttachmentLoadOp stencilLoadOp)
{
    VkAttachmentDescription attColor = {
                    .format = FB_COLOR_FORMAT,
                    .samples = dev->samples,
                    .loadOp = loadOp,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentDescription attDS = {
                    .format = FB_STENCIL_FORMAT,
                    .samples = dev->samples,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .stencilLoadOp = stencilLoadOp,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkAttachmentDescription attachments[] = {attColor,attDS};
    VkAttachmentReference colorRef  = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference dsRef     = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpassDescription = { .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                        .colorAttachmentCount   = 1,
                        .pColorAttachments      = &colorRef,
                        .pDepthStencilAttachment= &dsRef};

    VkSubpassDependency dependencies[] =
    {
        { VK_SUBPASS_EXTERNAL, 0,
          VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
          VK_DEPENDENCY_BY_REGION_BIT},
        { 0, VK_SUBPASS_EXTERNAL,
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
          VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
          VK_DEPENDENCY_BY_REGION_BIT},
    };

    VkRenderPassCreateInfo renderPassInfo = { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = 2,
                .pAttachments = attachments,
                .subpassCount = 1,
                .pSubpasses = &subpassDescription,
                .dependencyCount = 2,
                .pDependencies = dependencies
    };
    VkRenderPass rp;
    VK_CHECK_RESULT(vkCreateRenderPass(dev->vkDev, &renderPassInfo, NULL, &rp));
    return rp;
}
VkRenderPass _createRenderPassMS(VkvgDevice dev, VkAttachmentLoadOp loadOp, VkAttachmentLoadOp stencilLoadOp)
{
    VkAttachmentDescription attColor = {
                    .format = FB_COLOR_FORMAT,
                    .samples = dev->samples,
                    .loadOp = loadOp,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentDescription attColorResolve = {
                    .format = FB_COLOR_FORMAT,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentDescription attDS = {
                    .format = FB_STENCIL_FORMAT,
                    .samples = dev->samples,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .stencilLoadOp = stencilLoadOp,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkAttachmentDescription attachments[] = {attColorResolve,attDS,attColor};
    VkAttachmentReference resolveRef= {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference dsRef     = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    VkAttachmentReference colorRef  = {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpassDescription = { .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                        .colorAttachmentCount   = 1,
                        .pColorAttachments      = &colorRef,
                        .pResolveAttachments    = &resolveRef,
                        .pDepthStencilAttachment= &dsRef};

    VkSubpassDependency dependencies[] =
    {
        { VK_SUBPASS_EXTERNAL, 0,
          VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
          VK_DEPENDENCY_BY_REGION_BIT},
        { 0, VK_SUBPASS_EXTERNAL,
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
          VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
          VK_DEPENDENCY_BY_REGION_BIT},
    };

    VkRenderPassCreateInfo renderPassInfo = { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = 3,
                .pAttachments = attachments,
                .subpassCount = 1,
                .pSubpasses = &subpassDescription,
                .dependencyCount = 2,
                .pDependencies = dependencies
    };
    VkRenderPass rp;
    VK_CHECK_RESULT(vkCreateRenderPass(dev->vkDev, &renderPassInfo, NULL, &rp));
    return rp;
}

void _setupPipelines(VkvgDevice dev)
{
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .renderPass = dev->renderPass };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN };

    VkPipelineRasterizationStateCreateInfo rasterizationState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = VK_CULL_MODE_NONE,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .depthBiasEnable = VK_FALSE,
                .lineWidth = 1.0f };

    VkPipelineColorBlendAttachmentState blendAttachmentState =
    { .colorWriteMask = 0x0, .blendEnable = VK_TRUE,
#ifdef VKVG_PREMULT_ALPHA
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .alphaBlendOp = VK_BLEND_OP_ADD,
#else
      .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
      .dstColorBlendFactor= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD,
#endif
    };

    VkPipelineColorBlendStateCreateInfo colorBlendState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .attachmentCount = 1,
                .pAttachments = &blendAttachmentState };

                                        /*failOp,passOp,depthFailOp,compareOp, compareMask, writeMask, reference;*/
    VkStencilOpState polyFillOpState ={VK_STENCIL_OP_KEEP,VK_STENCIL_OP_INVERT,VK_STENCIL_OP_KEEP,VK_COMPARE_OP_EQUAL,STENCIL_CLIP_BIT,STENCIL_FILL_BIT,0};
    VkStencilOpState clipingOpState = {VK_STENCIL_OP_ZERO,VK_STENCIL_OP_REPLACE,VK_STENCIL_OP_KEEP,VK_COMPARE_OP_EQUAL,STENCIL_FILL_BIT,STENCIL_ALL_BIT,0x2};
    VkStencilOpState stencilOpState = {VK_STENCIL_OP_KEEP,VK_STENCIL_OP_ZERO,VK_STENCIL_OP_KEEP,VK_COMPARE_OP_EQUAL,STENCIL_FILL_BIT,STENCIL_FILL_BIT,0x1};

    VkPipelineDepthStencilStateCreateInfo dsStateCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .depthTestEnable = VK_FALSE,
                .depthWriteEnable = VK_FALSE,
                .depthCompareOp = VK_COMPARE_OP_ALWAYS,
                .stencilTestEnable = VK_TRUE,
                .front = polyFillOpState,
                .back = polyFillOpState };

    VkDynamicState dynamicStateEnables[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
        VK_DYNAMIC_STATE_STENCIL_REFERENCE,
        VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
    };
    VkPipelineDynamicStateCreateInfo dynamicState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .dynamicStateCount = 2,
                .pDynamicStates = dynamicStateEnables };

    VkPipelineViewportStateCreateInfo viewportState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1, .scissorCount = 1 };

    VkPipelineMultisampleStateCreateInfo multisampleState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = dev->samples };
    /*if (VKVG_SAMPLES != VK_SAMPLE_COUNT_1_BIT){
        multisampleState.sampleShadingEnable = VK_TRUE;
        multisampleState.minSampleShading = 0.25f;
        //multisampleState.alphaToCoverageEnable = VK_FALSE;
        //multisampleState.alphaToOneEnable = VK_FALSE;
    }*/
    VkVertexInputBindingDescription vertexInputBinding = { .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };

    VkVertexInputAttributeDescription vertexInputAttributs[2] = {
        {0, 0, VK_FORMAT_R32G32_SFLOAT,         0},
        {1, 0, VK_FORMAT_R32G32B32_SFLOAT,      sizeof(vec2)}
    };

    VkPipelineVertexInputStateCreateInfo vertexInputState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertexInputBinding,
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = vertexInputAttributs };

    VkShaderModule modVert, modFrag, modFragWired;
    VkShaderModuleCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                            .pCode = (uint32_t*)vkvg_main_vert_spv,
                                            .codeSize = vkvg_main_vert_spv_len };
    VK_CHECK_RESULT(vkCreateShaderModule(dev->vkDev, &createInfo, NULL, &modVert));
#ifdef VKVG_LCD_FONT_FILTER
    createInfo.pCode = (uint32_t*)vkvg_main_lcd_frag_spv;
    createInfo.codeSize = vkvg_main_lcd_frag_spv_len;
#else
    createInfo.pCode = (uint32_t*)vkvg_main_frag_spv;
    createInfo.codeSize = vkvg_main_frag_spv_len;
#endif
    VK_CHECK_RESULT(vkCreateShaderModule(dev->vkDev, &createInfo, NULL, &modFrag));

    VkPipelineShaderStageCreateInfo vertStage = { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = modVert,
        .pName = "main",
    };
    VkPipelineShaderStageCreateInfo fragStage = { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = modFrag,
        .pName = "main",
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

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertStage,fragStage};

    pipelineCreateInfo.stageCount = 1;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pDepthStencilState = &dsStateCreateInfo;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.layout = dev->pipelineLayout;


    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipelinePolyFill));

    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    dsStateCreateInfo.back = dsStateCreateInfo.front = clipingOpState;
    dynamicState.dynamicStateCount = 5;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipelineClipping));

    dsStateCreateInfo.back = dsStateCreateInfo.front = stencilOpState;
    blendAttachmentState.colorWriteMask=0xf;
    dynamicState.dynamicStateCount = 3;
    pipelineCreateInfo.stageCount = 2;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipe_OVER));

    blendAttachmentState.alphaBlendOp = blendAttachmentState.colorBlendOp = VK_BLEND_OP_SUBTRACT;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipe_SUB));

    blendAttachmentState.blendEnable = VK_FALSE;
    //rasterizationState.polygonMode = VK_POLYGON_MODE_POINT;
    //shaderStages[1].pName = "op_CLEAR";
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipe_CLEAR));


#ifdef VKVG_WIRED_DEBUG
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipelineLineList));

    createInfo.pCode = (uint32_t*)wired_frag_spv;
    createInfo.codeSize = wired_frag_spv_len;
    VK_CHECK_RESULT(vkCreateShaderModule(dev->vkDev, &createInfo, NULL, &modFragWired));

    shaderStages[1].module = modFragWired;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipelineWired));
    vkDestroyShaderModule(dev->vkDev, modFragWired, NULL);
#endif

    vkDestroyShaderModule(dev->vkDev, modVert, NULL);
    vkDestroyShaderModule(dev->vkDev, modFrag, NULL);
}

void _createDescriptorSetLayout (VkvgDevice dev) {

    VkDescriptorSetLayoutBinding dsLayoutBinding =
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,VK_SHADER_STAGE_FRAGMENT_BIT, NULL};
    VkDescriptorSetLayoutCreateInfo dsLayoutCreateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                                                          .bindingCount = 1,
                                                          .pBindings = &dsLayoutBinding };
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(dev->vkDev, &dsLayoutCreateInfo, NULL, &dev->dslFont));
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(dev->vkDev, &dsLayoutCreateInfo, NULL, &dev->dslSrc));
    dsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(dev->vkDev, &dsLayoutCreateInfo, NULL, &dev->dslGrad));

    VkPushConstantRange pushConstantRange[] = {
        {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(push_constants)},
        //{VK_SHADER_STAGE_FRAGMENT_BIT,0,sizeof(push_constants)}
    };
    VkDescriptorSetLayout dsls[] = {dev->dslFont,dev->dslSrc,dev->dslGrad};

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                            .pushConstantRangeCount = 1,
                                                            .pPushConstantRanges = (VkPushConstantRange*)&pushConstantRange,
                                                            .setLayoutCount = 3,
                                                            .pSetLayouts = dsls };
    VK_CHECK_RESULT(vkCreatePipelineLayout(dev->vkDev, &pipelineLayoutCreateInfo, NULL, &dev->pipelineLayout));
}

void _wait_idle (VkvgDevice dev) {
    vkDeviceWaitIdle (dev->vkDev);
}
void _wait_and_reset_device_fence (VkvgDevice dev) {
    vkWaitForFences (dev->vkDev, 1, &dev->fence, VK_TRUE, UINT64_MAX);
    vkResetFences (dev->vkDev, 1, &dev->fence);
}

void _submit_cmd (VkvgDevice dev, VkCommandBuffer* cmd, VkFence fence) {
    MUTEX_LOCK (&dev->gQMutex);
    vkh_cmd_submit (dev->gQueue, cmd, fence);
    MUTEX_UNLOCK (&dev->gQMutex);
}

void _init_function_pointers (VkvgDevice dev) {
    CmdBindPipeline         = GetInstProcAddress(dev->instance, vkCmdBindPipeline);
    CmdBindDescriptorSets   = GetInstProcAddress(dev->instance, vkCmdBindDescriptorSets);
    CmdBindIndexBuffer      = GetInstProcAddress(dev->instance, vkCmdBindIndexBuffer);
    CmdBindVertexBuffers    = GetInstProcAddress(dev->instance, vkCmdBindVertexBuffers);
    CmdDrawIndexed          = GetInstProcAddress(dev->instance, vkCmdDrawIndexed);
    CmdDraw                 = GetInstProcAddress(dev->instance, vkCmdDraw);
    CmdSetStencilCompareMask= GetInstProcAddress(dev->instance, vkCmdSetStencilCompareMask);
    CmdSetStencilReference  = GetInstProcAddress(dev->instance, vkCmdSetStencilReference);
    CmdSetStencilWriteMask  = GetInstProcAddress(dev->instance, vkCmdSetStencilWriteMask);
    CmdBeginRenderPass      = GetInstProcAddress(dev->instance, vkCmdBeginRenderPass);
    CmdEndRenderPass        = GetInstProcAddress(dev->instance, vkCmdEndRenderPass);
    CmdSetViewport          = GetInstProcAddress(dev->instance, vkCmdSetViewport);
    CmdSetScissor           = GetInstProcAddress(dev->instance, vkCmdSetScissor);
    CmdPushConstants        = GetInstProcAddress(dev->instance, vkCmdPushConstants);
    CmdPushDescriptorSet    = (PFN_vkCmdPushDescriptorSetKHR)vkGetInstanceProcAddr(dev->instance, "vkCmdDescriptorSet");
}

void _create_empty_texture (VkvgDevice dev) {
    //create empty image to bind to context source descriptor when not in use
    dev->emptyImg = vkh_image_create((VkhDevice)dev,FB_COLOR_FORMAT,16,16,VKVG_TILING,VMA_MEMORY_USAGE_GPU_ONLY,
                                     VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkh_image_create_descriptor(dev->emptyImg, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

    _wait_and_reset_device_fence (dev);

    vkh_cmd_begin (dev->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkh_image_set_layout (dev->cmd, dev->emptyImg, VK_IMAGE_ASPECT_COLOR_BIT,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    vkh_cmd_end (dev->cmd);
    _submit_cmd (dev, &dev->cmd, dev->fence);
}

void _dump_image_format_properties (VkvgDevice dev) {
    VkImageFormatProperties imgProps;
    VK_CHECK_RESULT(vkGetPhysicalDeviceImageFormatProperties(dev->phy,
                                                             FB_COLOR_FORMAT, VK_IMAGE_TYPE_2D, VKVG_TILING,
                                                             VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                             0, &imgProps));
    printf ("tiling           = %d\n", VKVG_TILING);
    printf ("max extend       = (%d, %d, %d)\n", imgProps.maxExtent.width, imgProps.maxExtent.height, imgProps.maxExtent.depth);
    printf ("max mip levels   = %d\n", imgProps.maxMipLevels);
    printf ("max array layers = %d\n", imgProps.maxArrayLayers);
    printf ("sample counts    = ");
    if (imgProps.sampleCounts & VK_SAMPLE_COUNT_1_BIT)
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


}

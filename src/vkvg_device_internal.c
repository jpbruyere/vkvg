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
void _init_all_contexes (VkvgDevice dev){
    VkvgContext next = dev->lastCtx;
    while (next != NULL){
        _init_cmd_buff (next);
        next = next->pPrev;
    }
}

void _create_pipeline_cache(VkvgDevice dev){
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
    VK_CHECK_RESULT(vkCreatePipelineCache(dev->vkDev, &pipelineCacheCreateInfo, NULL, &dev->pipelineCache));
}
void _setupRenderPass(VkvgDevice dev)
{
    VkAttachmentDescription attColor = {
                    .format = FB_COLOR_FORMAT,
                    .samples = VKVG_SAMPLES,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentDescription attColorResolve = {
                    .format = FB_COLOR_FORMAT,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL };
    VkAttachmentDescription attDS = {
                    .format = VK_FORMAT_S8_UINT,
                    .samples = VKVG_SAMPLES,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
/*    VkAttachmentDescription attDSResolve = {
                    .format = VK_FORMAT_S8_UINT,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };*/
    VkAttachmentDescription attachments[] = {attColor,attColorResolve,attDS};
    VkAttachmentReference colorRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference colorResolveRef = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference dsRef = {
        .attachment = 2,
        .layout = VK_IMAGE_LAYOUT_GENERAL };
    /*VkAttachmentReference dsResolveRef = {
        .attachment = 3,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };*/
    VkSubpassDescription subpassDescription = { .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                        .colorAttachmentCount = 1,
                        .pColorAttachments = &colorRef,
                        .pResolveAttachments = &colorResolveRef,
                        .pDepthStencilAttachment = &dsRef};
    VkSubpassDependency dep0 = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT };
    VkSubpassDependency dep1 = {
        .srcSubpass = 0,
        .dstSubpass = VK_SUBPASS_EXTERNAL,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT };

    VkSubpassDependency dependencies[] = {dep0,dep1};
    VkRenderPassCreateInfo renderPassInfo = { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = 3,
                .pAttachments = attachments,
                .subpassCount = 1,
                .pSubpasses = &subpassDescription,
                .dependencyCount = 2,
                .pDependencies = dependencies };

    VK_CHECK_RESULT(vkCreateRenderPass(dev->vkDev, &renderPassInfo, NULL, &dev->renderPass));
}

void _setupPipelines(VkvgDevice dev)
{
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .renderPass = dev->renderPass };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };

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
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .alphaBlendOp = VK_BLEND_OP_ADD,
    };

    VkPipelineColorBlendStateCreateInfo colorBlendState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .attachmentCount = 1,
                .pAttachments = &blendAttachmentState };

                                        /*failOp,passOp,depthFailOp,compareOp, compareMask, writeMask, reference;*/
    VkStencilOpState clipingOpState = {VK_STENCIL_OP_KEEP,VK_STENCIL_OP_INCREMENT_AND_CLAMP,VK_STENCIL_OP_KEEP,VK_COMPARE_OP_EQUAL,0x0,0xf,0};
    VkStencilOpState stencilOpState = {VK_STENCIL_OP_KEEP,VK_STENCIL_OP_REPLACE,VK_STENCIL_OP_KEEP,VK_COMPARE_OP_EQUAL,0xf,0x0,0};

    VkPipelineDepthStencilStateCreateInfo dsStateCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .depthTestEnable = VK_FALSE,
                .depthWriteEnable = VK_FALSE,
                .depthCompareOp = VK_COMPARE_OP_ALWAYS,
                .stencilTestEnable = VK_TRUE,
                .front = clipingOpState,
                .back = clipingOpState };

    VkDynamicState dynamicStateEnables[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_STENCIL_REFERENCE,
    };
    VkPipelineDynamicStateCreateInfo dynamicState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .dynamicStateCount = 3,
                .pDynamicStates = dynamicStateEnables };

    VkPipelineViewportStateCreateInfo viewportState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1, .scissorCount = 1 };

    VkPipelineMultisampleStateCreateInfo multisampleState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = VKVG_SAMPLES };
    if (VKVG_SAMPLES != VK_SAMPLE_COUNT_1_BIT){
        multisampleState.sampleShadingEnable = VK_TRUE;
        multisampleState.minSampleShading = 0.25f;
        multisampleState.alphaToCoverageEnable = VK_FALSE;
        multisampleState.alphaToOneEnable = VK_FALSE;
    }
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

    VkShaderModule modVert, modFrag;
    VkShaderModuleCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                            .pCode = triangle_vert_spv,
                                            .codeSize = triangle_vert_spv_len };
    VK_CHECK_RESULT(vkCreateShaderModule(dev->vkDev, &createInfo, NULL, &modVert));
    createInfo.pCode = triangle_frag_spv;
    createInfo.codeSize = triangle_frag_spv_len;
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
    VkSpecializationMapEntry specializationEntry = {
        .constantID = 0,
        .offset = 0,
        .size = sizeof(uint32_t)};
    uint32_t specializationData = VKVG_SAMPLES;
    VkSpecializationInfo specializationInfo = {
        .mapEntryCount = 1,
        .pMapEntries = &specializationEntry,
        .dataSize = sizeof(specializationData),
        .pData = &specializationData};

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertStage,fragStage};

    pipelineCreateInfo.stageCount = 2;
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

    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipelineClipping));

    //dsStateCreateInfo.back.writeMask = dsStateCreateInfo.front.writeMask = 0;
    dsStateCreateInfo.back = dsStateCreateInfo.front = stencilOpState;
    blendAttachmentState.colorWriteMask=0xf;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipeline));

    blendAttachmentState.alphaBlendOp = blendAttachmentState.colorBlendOp = VK_BLEND_OP_SUBTRACT;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipeline_OP_SUB));

    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipelineWired));

    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipelineLineList));

    vkDestroyShaderModule(dev->vkDev, shaderStages[0].module, NULL);
    vkDestroyShaderModule(dev->vkDev, shaderStages[1].module, NULL);
}

void _createDescriptorSetLayout (VkvgDevice dev) {

    VkDescriptorSetLayoutBinding dsLayoutBinding =
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,VK_SHADER_STAGE_FRAGMENT_BIT};
    VkDescriptorSetLayoutCreateInfo dsLayoutCreateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                                                          .bindingCount = 1,
                                                          .pBindings = &dsLayoutBinding };
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(dev->vkDev, &dsLayoutCreateInfo, NULL, &dev->dslFont));
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(dev->vkDev, &dsLayoutCreateInfo, NULL, &dev->dslSrc));

    VkPushConstantRange pushConstantRange[] = {
        {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(push_constants)},
        //{VK_SHADER_STAGE_FRAGMENT_BIT,0,sizeof(push_constants)}
    };
    VkDescriptorSetLayout dsls[] = {dev->dslFont,dev->dslSrc};

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                            .pushConstantRangeCount = 1,
                                                            .pPushConstantRanges = &pushConstantRange,
                                                            .setLayoutCount = 2,
                                                            .pSetLayouts = dsls };
    VK_CHECK_RESULT(vkCreatePipelineLayout(dev->vkDev, &pipelineLayoutCreateInfo, NULL, &dev->pipelineLayout));
}

void _wait_device_fence (VkvgDevice dev) {
    vkWaitForFences (dev->vkDev, 1, &dev->fence, VK_TRUE, UINT64_MAX);
    vkResetFences   (dev->vkDev, 1, &dev->fence);
}

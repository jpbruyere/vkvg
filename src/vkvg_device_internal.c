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

#define GetInstProcAddress(inst, func)(PFN_##func)vkGetInstanceProcAddr(inst, #func);

#define GetVkProcAddress(dev, inst, func)(vkGetDeviceProcAddr(dev,#func)==NULL)?(PFN_##func)vkGetInstanceProcAddr(inst, #func):(PFN_##func)vkGetDeviceProcAddr(dev, #func)

#include "vkvg_device_internal.h"
#include "vkvg_context_internal.h"
#include "shaders.h"

uint32_t vkvg_log_level = VKVG_LOG_DEBUG;
#ifdef VKVG_WIRED_DEBUG
vkvg_wired_debug_mode vkvg_wired_debug = vkvg_wired_debug_mode_normal;
#endif

PFN_vkCmdBindPipeline			CmdBindPipeline;
PFN_vkCmdBindDescriptorSets		CmdBindDescriptorSets;
PFN_vkCmdBindIndexBuffer		CmdBindIndexBuffer;
PFN_vkCmdBindVertexBuffers		CmdBindVertexBuffers;

PFN_vkCmdDrawIndexed			CmdDrawIndexed;
PFN_vkCmdDraw					CmdDraw;

PFN_vkCmdSetStencilCompareMask	CmdSetStencilCompareMask;
PFN_vkCmdSetStencilReference	CmdSetStencilReference;
PFN_vkCmdSetStencilWriteMask	CmdSetStencilWriteMask;
PFN_vkCmdBeginRenderPass		CmdBeginRenderPass;
PFN_vkCmdEndRenderPass			CmdEndRenderPass;
PFN_vkCmdSetViewport			CmdSetViewport;
PFN_vkCmdSetScissor				CmdSetScissor;

PFN_vkCmdPushConstants			CmdPushConstants;

PFN_vkWaitForFences				WaitForFences;
PFN_vkResetFences				ResetFences;
PFN_vkResetCommandBuffer		ResetCommandBuffer;

bool _try_get_phyinfo (VkhPhyInfo* phys, uint32_t phyCount, VkPhysicalDeviceType gpuType, VkhPhyInfo* phy) {
	for (uint32_t i=0; i<phyCount; i++){
		if (vkh_phyinfo_get_properties(phys[i]).deviceType == gpuType) {
			 *phy = phys[i];
			 return true;
		}
	}
	return false;
}
void _device_flush_all_contexes (VkvgDevice dev){
	/*VkvgContext ctx = dev->lastCtx;
	while (ctx != NULL){
		if (ctx->cmdStarted)
			_flush_cmd_until_vx_base (ctx);

		ctx = ctx->pPrev;
	}*/
}
//TODO:save/reload cache in user temp directory
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
					.format = dev->stencilFormat,
					.samples = dev->samples,
					.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.stencilLoadOp = stencilLoadOp,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
					.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
					.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	VkAttachmentDescription attachments[] = {attColor,attDS};
	VkAttachmentReference colorRef	= {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
	VkAttachmentReference dsRef		= {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

	VkSubpassDescription subpassDescription = { .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
						.colorAttachmentCount	= 1,
						.pColorAttachments		= &colorRef,
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
					.format = dev->stencilFormat,
					.samples = dev->samples,
					.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.stencilLoadOp = stencilLoadOp,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
					.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
					.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	VkAttachmentDescription attachments[] = {attColorResolve,attDS,attColor};
	VkAttachmentReference resolveRef= {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
	VkAttachmentReference dsRef		= {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
	VkAttachmentReference colorRef	= {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

	VkSubpassDescription subpassDescription = { .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
						.colorAttachmentCount	= 1,
						.pColorAttachments		= &colorRef,
						.pResolveAttachments	= &resolveRef,
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

	VkVertexInputAttributeDescription vertexInputAttributs[3] = {
		{0, 0, VK_FORMAT_R32G32_SFLOAT, 0},
		{1, 0, VK_FORMAT_R8G8B8A8_UNORM, 8},
		{2, 0, VK_FORMAT_R32G32B32_SFLOAT, 12}
	};

	VkPipelineVertexInputStateCreateInfo vertexInputState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &vertexInputBinding,
		.vertexAttributeDescriptionCount = 3,
		.pVertexAttributeDescriptions = vertexInputAttributs };
#ifdef VKVG_WIRED_DEBUG
	VkShaderModule modVert, modFrag, modFragWired;
#else
	VkShaderModule modVert, modFrag;
#endif
	VkShaderModuleCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
											.pCode = (uint32_t*)vkvg_main_vert_spv,
											.codeSize = vkvg_main_vert_spv_len };
	VK_CHECK_RESULT(vkCreateShaderModule(dev->vkDev, &createInfo, NULL, &modVert));
#if defined(VKVG_LCD_FONT_FILTER) && defined(FT_CONFIG_OPTION_SUBPIXEL_RENDERING)
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

#ifndef __APPLE__
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipelinePolyFill));
#endif

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

	//shaderStages[1].pName = "op_CLEAR";
	blendAttachmentState.colorBlendOp = blendAttachmentState.alphaBlendOp = VK_BLEND_OP_SUBTRACT;
	blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipe_CLEAR));


#ifdef VKVG_WIRED_DEBUG
	createInfo.pCode = (uint32_t*)wired_frag_spv;
	createInfo.codeSize = wired_frag_spv_len;
	VK_CHECK_RESULT(vkCreateShaderModule(dev->vkDev, &createInfo, NULL, &modFragWired));

	shaderStages[1].module = modFragWired;

	rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(dev->vkDev, dev->pipelineCache, 1, &pipelineCreateInfo, NULL, &dev->pipelineLineList));

	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
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

void _device_wait_idle (VkvgDevice dev) {
	vkDeviceWaitIdle (dev->vkDev);
}
void _device_wait_and_reset_device_fence (VkvgDevice dev) {
	vkWaitForFences (dev->vkDev, 1, &dev->fence, VK_TRUE, UINT64_MAX);
	_device_reset_fence(dev, dev->fence);
}

void _device_destroy_fence (VkvgDevice dev, VkFence fence) {
	LOCK_DEVICE

	if (dev->gQLastFence == fence)
		dev->gQLastFence = VK_NULL_HANDLE;

	vkDestroyFence (dev->vkDev, fence, NULL);

	UNLOCK_DEVICE
}
void _device_reset_fence (VkvgDevice dev, VkFence fence){
	LOCK_DEVICE

	if (dev->gQLastFence == fence)
		dev->gQLastFence = VK_NULL_HANDLE;

	ResetFences (dev->vkDev, 1, &fence);

	UNLOCK_DEVICE
}
void _device_wait_fence (VkvgDevice dev, VkFence fence){

}
void _device_wait_and_reset_fence	(VkvgDevice dev, VkFence fence){

}
bool _device_try_get_cached_context (VkvgDevice dev, VkvgContext* pCtx) {
	LOCK_DEVICE

	if (dev->cachedContextCount)
		*pCtx = dev->cachedContext[--dev->cachedContextCount];
	else
		*pCtx = NULL;

	UNLOCK_DEVICE

	return *pCtx != NULL;
}
void _device_store_context (VkvgContext ctx) {
	VkvgDevice dev = ctx->dev;

	LOCK_DEVICE

	if (dev->gQLastFence == ctx->flushFence)
		dev->gQLastFence = VK_NULL_HANDLE;
	dev->cachedContext[dev->cachedContextCount++] = ctx;
	ctx->references++;

	UNLOCK_DEVICE
}
void _device_submit_cmd (VkvgDevice dev, VkCommandBuffer* cmd, VkFence fence) {
	LOCK_DEVICE
	if (dev->gQLastFence != VK_NULL_HANDLE)
		WaitForFences (dev->vkDev, 1, &dev->gQLastFence, VK_TRUE, VKVG_FENCE_TIMEOUT);
	vkh_cmd_submit (dev->gQueue, cmd, fence);
	dev->gQLastFence = fence;
	UNLOCK_DEVICE
}

bool _init_function_pointers (VkvgDevice dev) {
#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	if (vkGetInstanceProcAddr(dev->instance, "vkSetDebugUtilsObjectNameEXT")==VK_NULL_HANDLE){
		LOG(VKVG_LOG_ERR, "vkvg create device failed: 'VK_EXT_debug_utils' has to be loaded for Debug build\n");
		return false;
	}
	vkh_device_init_debug_utils ((VkhDevice)dev);
#endif
	CmdBindPipeline			= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdBindPipeline);
	CmdBindDescriptorSets	= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdBindDescriptorSets);
	CmdBindIndexBuffer		= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdBindIndexBuffer);
	CmdBindVertexBuffers	= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdBindVertexBuffers);
	CmdDrawIndexed			= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdDrawIndexed);
	CmdDraw					= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdDraw);
	CmdSetStencilCompareMask= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdSetStencilCompareMask);
	CmdSetStencilReference	= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdSetStencilReference);
	CmdSetStencilWriteMask	= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdSetStencilWriteMask);
	CmdBeginRenderPass		= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdBeginRenderPass);
	CmdEndRenderPass		= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdEndRenderPass);
	CmdSetViewport			= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdSetViewport);
	CmdSetScissor			= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdSetScissor);
	CmdPushConstants		= GetVkProcAddress(dev->vkDev, dev->instance, vkCmdPushConstants);
	WaitForFences			= GetVkProcAddress(dev->vkDev, dev->instance, vkWaitForFences);
	ResetFences				= GetVkProcAddress(dev->vkDev, dev->instance, vkResetFences);
	ResetCommandBuffer		= GetVkProcAddress(dev->vkDev, dev->instance, vkResetCommandBuffer);
	return true;
}

void _create_empty_texture (VkvgDevice dev, VkFormat format, VkImageTiling tiling) {
	//create empty image to bind to context source descriptor when not in use
	dev->emptyImg = vkh_image_create((VkhDevice)dev,format,16,16,tiling,VMA_MEMORY_USAGE_GPU_ONLY,
									 VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	vkh_image_create_descriptor(dev->emptyImg, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

	_device_wait_and_reset_device_fence (dev);

	vkh_cmd_begin (dev->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	vkh_image_set_layout (dev->cmd, dev->emptyImg, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						  VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	vkh_cmd_end (dev->cmd);
	_device_submit_cmd (dev, &dev->cmd, dev->fence);
}

void _check_best_image_tiling (VkvgDevice dev, VkFormat format) {
	VkFlags stencilFormats[] = { VK_FORMAT_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT };
	VkFormatProperties phyStencilProps = { 0 }, phyImgProps = { 0 };

	//check png blit format
	VkFlags pngBlitFormats[] = { VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_UNORM};
	dev->pngStagFormat = VK_FORMAT_UNDEFINED;
	for (int i = 0; i < 2; i++)
	{
		vkGetPhysicalDeviceFormatProperties(dev->phy, pngBlitFormats[i], &phyImgProps);
		if ((phyImgProps.linearTilingFeatures & VKVG_PNG_WRITE_IMG_REQUIREMENTS) == VKVG_PNG_WRITE_IMG_REQUIREMENTS) {
			dev->pngStagFormat = pngBlitFormats[i];
			dev->pngStagTiling = VK_IMAGE_TILING_LINEAR;
			break;
		} else if ((phyImgProps.optimalTilingFeatures & VKVG_PNG_WRITE_IMG_REQUIREMENTS) == VKVG_PNG_WRITE_IMG_REQUIREMENTS) {
			dev->pngStagFormat = pngBlitFormats[i];
			dev->pngStagTiling = VK_IMAGE_TILING_OPTIMAL;
			break;
		}
	}

	if (dev->pngStagFormat == VK_FORMAT_UNDEFINED)
		LOG(VKVG_LOG_DEBUG, "vkvg create device failed: no suitable image format for png write\n");

	dev->stencilFormat = VK_FORMAT_UNDEFINED;
	dev->supportedTiling = 0xff;
	
	vkGetPhysicalDeviceFormatProperties(dev->phy, format, &phyImgProps);
	
	if ((phyImgProps.optimalTilingFeatures & VKVG_SURFACE_IMGS_REQUIREMENTS) == VKVG_SURFACE_IMGS_REQUIREMENTS) {
		for (int i = 0; i < 4; i++)
		{
			vkGetPhysicalDeviceFormatProperties(dev->phy, stencilFormats[i], &phyStencilProps);
			if (phyStencilProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				dev->stencilFormat = stencilFormats[i];
				dev->supportedTiling = VK_IMAGE_TILING_OPTIMAL;
				return;
			}
		}
	}
	if ((phyImgProps.linearTilingFeatures & VKVG_SURFACE_IMGS_REQUIREMENTS) == VKVG_SURFACE_IMGS_REQUIREMENTS) {
		for (int i = 0; i < 4; i++)
		{
			vkGetPhysicalDeviceFormatProperties(dev->phy, stencilFormats[i], &phyStencilProps);
			if (phyStencilProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				dev->stencilFormat = stencilFormats[i];
				dev->supportedTiling = VK_IMAGE_TILING_LINEAR;
				return;
			}
		}
	}
	dev->status = VKVG_STATUS_INVALID_FORMAT;
	LOG(VKVG_LOG_ERR, "vkvg create device failed: image format not supported: %d\n", format);
}

void _dump_image_format_properties (VkvgDevice dev, VkFormat format) {
	/*VkImageFormatProperties imgProps;
	VK_CHECK_RESULT(vkGetPhysicalDeviceImageFormatProperties(dev->phy,
															 format, VK_IMAGE_TYPE_2D, VKVG_TILING,
															 VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT,
															 0, &imgProps));
	printf ("tiling			  = %d\n", VKVG_TILING);
	printf ("max extend		  = (%d, %d, %d)\n", imgProps.maxExtent.width, imgProps.maxExtent.height, imgProps.maxExtent.depth);
	printf ("max mip levels	  = %d\n", imgProps.maxMipLevels);
	printf ("max array layers = %d\n", imgProps.maxArrayLayers);
	printf ("sample counts	  = ");
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
*/

}

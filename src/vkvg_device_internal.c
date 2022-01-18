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
#include "vkvg_pattern.h"

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

void _create_vertices_buff (vkvg_device_thread_items_t* ctx){
	vkvg_buffer_create (ctx->dev,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		ctx->sizeVBO * sizeof(Vertex), &ctx->vertices);
	vkvg_buffer_create (ctx->dev,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		ctx->sizeIBO * sizeof(VKVG_IBO_INDEX_TYPE), &ctx->indices);
}
void _resize_vbo (vkvg_device_thread_items_t* ctx, uint32_t new_size) {
	ctx->sizeVBO = new_size;
	uint32_t mod = ctx->sizeVBO % VKVG_VBO_SIZE;
	if (mod > 0)
		ctx->sizeVBO += VKVG_VBO_SIZE - mod;
	LOG(VKVG_LOG_DBG_ARRAYS, "resize VBO: new size: %d\n", ctx->sizeVBO);
	vkvg_buffer_destroy (&ctx->vertices);
	vkvg_buffer_create (ctx->dev,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		ctx->sizeVBO * sizeof(Vertex), &ctx->vertices);
}
void _resize_ibo (vkvg_device_thread_items_t* ctx, size_t new_size) {
	ctx->sizeIBO = new_size;
	uint32_t mod = ctx->sizeIBO % VKVG_IBO_SIZE;
	if (mod > 0)
		ctx->sizeIBO += VKVG_IBO_SIZE - mod;
	LOG(VKVG_LOG_DBG_ARRAYS, "resize IBO: new size: %d\n", ctx->sizeIBO);
	vkvg_buffer_destroy (&ctx->indices);
	vkvg_buffer_create (ctx->dev,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		ctx->sizeIBO * sizeof(VKVG_IBO_INDEX_TYPE), &ctx->indices);
}

void _delete_threaded_object (VkvgDevice dev, vkvg_device_thread_items_t* throbjs) {
	vkvg_buffer_destroy (&throbjs->uboGrad);
	vkvg_buffer_destroy (&throbjs->vertices);
	vkvg_buffer_destroy (&throbjs->indices);

	/*VkDescriptorSet dss[] = {throbjs->dsFont, throbjs->dsSrc};
	vkFreeDescriptorSets	(dev->vkDev, throbjs->descriptorPool, 2, dss);*/

	vkDestroyDescriptorPool (dev->vkDev, throbjs->descriptorPool,NULL);

	free(throbjs->vertexCache);
	free(throbjs->indexCache);
	free(throbjs->pathes);
	free(throbjs->points);

	free (throbjs);
}
void _add_threaded_objects (VkvgDevice dev, vkvg_device_thread_items_t* throbjs) {
	vkvg_device_thread_items_t* tmp = dev->threaded_objects;
	if (!tmp) {
		dev->threaded_objects = throbjs;
		return;
	}
	while (tmp->next)
		tmp = tmp->next;
	tmp->next = throbjs;
}

vkvg_device_thread_items_t* _get_or_create_threaded_objects (VkvgDevice dev, thrd_t id) {
	vkvg_device_thread_items_t* tmp = dev->threaded_objects;
	while (tmp) {
		if (thrd_equal(tmp->id, id) && !tmp->inUse) {
			tmp->inUse = true;
			return tmp;
		}
		tmp = tmp->next;
	}
	tmp = (vkvg_device_thread_items_t*)calloc(1, sizeof(vkvg_device_thread_items_t));
	tmp->id = thrd_current();
	tmp->dev = dev;
	tmp->sizeVertices = tmp->sizeVBO = VKVG_VBO_SIZE;
	tmp->sizeIndices = tmp->sizeIBO = VKVG_IBO_SIZE;
	tmp->sizePoints		= VKVG_PTS_SIZE;
	tmp->sizeVertices	= VKVG_VBO_SIZE;
	tmp->sizeIndices	= VKVG_IBO_SIZE;
	tmp->sizePathes		= VKVG_PATHES_SIZE;

	tmp->points	= (vec2*)malloc (VKVG_VBO_SIZE*sizeof(vec2));
	tmp->pathes	= (uint32_t*)malloc (VKVG_PATHES_SIZE*sizeof(uint32_t));
	tmp->vertexCache = (Vertex*)malloc(tmp->sizeVertices * sizeof(Vertex));
	tmp->indexCache = (VKVG_IBO_INDEX_TYPE*)malloc(tmp->sizeIndices * sizeof(VKVG_IBO_INDEX_TYPE));


	const VkDescriptorPoolSize descriptorPoolSize[] = {
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 },
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 }
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
															.maxSets = 3,
															.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
															.poolSizeCount = 2,
															.pPoolSizes = descriptorPoolSize };
	VK_CHECK_RESULT(vkCreateDescriptorPool (dev->vkDev, &descriptorPoolCreateInfo, NULL, &tmp->descriptorPool));

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
															  .descriptorPool = tmp->descriptorPool,
															  .descriptorSetCount = 1,
															  .pSetLayouts = &dev->dslGrad };
	VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &tmp->dsGrad));
	descriptorSetAllocateInfo.pSetLayouts = &dev->dslFont;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &tmp->dsFont));
	descriptorSetAllocateInfo.pSetLayouts = &dev->dslSrc;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &tmp->dsSrc));

	vkvg_buffer_create (dev,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		sizeof(vkvg_gradient_t), &tmp->uboGrad);

	VkDescriptorBufferInfo dbi = {tmp->uboGrad.buffer, 0, VK_WHOLE_SIZE};
	VkWriteDescriptorSet writeDescriptorSet = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = tmp->dsGrad,
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &dbi
	};
	vkUpdateDescriptorSets(dev->vkDev, 1, &writeDescriptorSet, 0, NULL);

	_update_descriptor_set	(tmp, dev->fontCache->texture, tmp->dsFont);
	_update_descriptor_set	(tmp, dev->emptyImg, tmp->dsSrc);

	_create_vertices_buff	(tmp);

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64_t)tmp->descriptorPool, "CTX Descriptor Pool");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)tmp->dsSrc, "CTX DescSet SOURCE");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)tmp->dsFont, "CTX DescSet FONT");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)tmp->dsGrad, "CTX DescSet GRADIENT");

	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_BUFFER, (uint64_t)tmp->indices.buffer, "CTX Index Buff");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_BUFFER, (uint64_t)tmp->vertices.buffer, "CTX Vertex Buff");
#endif


	_add_threaded_objects(dev, tmp);

	tmp->inUse = true;

	return tmp;
}
void _update_descriptor_set (vkvg_device_thread_items_t* ctx, VkhImage img, VkDescriptorSet ds){
	VkDescriptorImageInfo descSrcTex = vkh_image_get_descriptor (img, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkWriteDescriptorSet writeDescriptorSet = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = ds,
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &descSrcTex
	};
	vkUpdateDescriptorSets(ctx->dev->vkDev, 1, &writeDescriptorSet, 0, NULL);
}
bool _try_get_phyinfo (VkhPhyInfo* phys, uint32_t phyCount, VkPhysicalDeviceType gpuType, VkhPhyInfo* phy) {
	for (uint32_t i=0; i<phyCount; i++){
		if (vkh_phyinfo_get_properties(phys[i]).deviceType == gpuType) {
			 *phy = phys[i];
			 return true;
		}
	}
	return false;
}
void _flush_all_contexes (VkvgDevice dev){
	VkvgContext ctx = dev->lastCtx;
	while (ctx != NULL){
		if (ctx->cmdStarted)
			_flush_cmd_until_vx_base (ctx);

		ctx = ctx->pPrev;
	}
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

	_wait_and_reset_device_fence (dev);

	vkh_cmd_begin (dev->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	vkh_image_set_layout (dev->cmd, dev->emptyImg, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						  VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	vkh_cmd_end (dev->cmd);
	_submit_cmd (dev, &dev->cmd, dev->fence);
}

void _check_best_image_tiling (VkvgDevice dev, VkFormat format) {
	VkFlags stencilFormats[] = { VK_FORMAT_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT };
	VkFormatProperties phyStencilProps = { 0 }, phyImgProps = { 0 };

	dev->stencilFormat = VK_FORMAT_UNDEFINED;
	dev->supportedTiling = 0xff;
	
	vkGetPhysicalDeviceFormatProperties(dev->phy, format, &phyImgProps);
	
	if (phyImgProps.optimalTilingFeatures & (VKVG_SURFACE_IMGS_REQUIREMENTS)) {
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
	if (phyImgProps.linearTilingFeatures & (VKVG_SURFACE_IMGS_REQUIREMENTS)) {
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

static VkExtensionProperties* instExtProps;
static uint32_t instExtCount;
bool _instance_extension_supported (const char* instanceName) {
	for (uint32_t i=0; i<instExtCount; i++) {
		if (!strcmp(instExtProps[i].extensionName, instanceName))
			return true;
	}
	return false;
}
void _instance_extensions_check_init () {
	VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &instExtCount, NULL));
	instExtProps =(VkExtensionProperties*)malloc(instExtCount * sizeof(VkExtensionProperties));
	VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &instExtCount, instExtProps));
}
void _instance_extensions_check_release () {
	free (instExtProps);
}

static VkLayerProperties* instLayerProps;
static uint32_t instance_layer_count;
bool _layer_is_present (const char* layerName) {
	for (uint32_t i=0; i<instance_layer_count; i++) {
		if (!strcmp(instLayerProps[i].layerName, layerName))
			return true;
	}
	return false;
}
void _layers_check_init () {
	VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL));
	instLayerProps =(VkLayerProperties*)malloc(instance_layer_count * sizeof(VkLayerProperties));
	VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&instance_layer_count, instLayerProps));
}
void _layers_check_release () {
	free (instLayerProps);
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

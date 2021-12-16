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

#include "vkh.h"
#include "vkengine.h"
#include "vkh_app.h"
#include "vkh_phyinfo.h"
#include "vkh_presenter.h"
#include "vkh_image.h"
#include "vkh_device.h"

VkSampleCountFlagBits getMaxUsableSampleCount(VkSampleCountFlags counts)
{
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
	return VK_SAMPLE_COUNT_1_BIT;
}

void vkengine_dump_Infos (VkEngine e){
	printf("max samples = %d\n", getMaxUsableSampleCount(e->gpu_props.limits.framebufferColorSampleCounts));
	printf("max tex2d size = %d\n", e->gpu_props.limits.maxImageDimension2D);
	printf("max tex array layers = %d\n", e->gpu_props.limits.maxImageArrayLayers);
	printf("max mem alloc count = %d\n", e->gpu_props.limits.maxMemoryAllocationCount);

	for (uint32_t i = 0; i < e->memory_properties.memoryHeapCount; i++) {
		printf("Mem Heap %d\n", i);
		printf("\tflags= %d\n", e->memory_properties.memoryHeaps[i].flags);
		printf("\tsize = %lu Mo\n", (unsigned long)e->memory_properties.memoryHeaps[i].size/ (uint32_t)(1024*1024));
	}
	for (uint32_t i = 0; i < e->memory_properties.memoryTypeCount; i++) {
		printf("Mem type %d\n", i);
		printf("\theap %d: ", e->memory_properties.memoryTypes[i].heapIndex);
		if (e->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			printf("VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|");
		if (e->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			printf("VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|");
		if (e->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
			printf("VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|");
		if (e->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
			printf("VK_MEMORY_PROPERTY_HOST_CACHED_BIT|");
		if (e->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
			printf("VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT|");
		printf("\n");
	}
}



void vkengine_dump_available_layers () {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);

	VkLayerProperties* availableLayers = (VkLayerProperties*)malloc(layerCount*sizeof(VkLayerProperties));
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

	printf("Available Layers:\n");
	printf("-----------------\n");
	for (uint32_t i=0; i<layerCount; i++) {
		 printf ("\t - %s\n", availableLayers[i].layerName);
	}
	printf("-----------------\n\n");
	free (availableLayers);
}
bool vkengine_try_get_phyinfo (VkhPhyInfo* phys, uint32_t phyCount, VkPhysicalDeviceType gpuType, VkhPhyInfo* phy) {
	for (uint32_t i=0; i<phyCount; i++){
		if (phys[i]->properties.deviceType == gpuType) {
			 *phy = phys[i];
			 return true;
		}
	}
	return false;
}
bool instance_extension_supported (VkExtensionProperties* instanceExtProps, uint32_t extCount, const char* instanceName) {
	for (uint32_t i=0; i<extCount; i++) {
		if (!strcmp(instanceExtProps[i].extensionName, instanceName))
			return true;
	}
	return false;
}
vk_engine_t* vkengine_create (VkPhysicalDeviceType preferedGPU, VkPresentModeKHR presentMode, uint32_t width, uint32_t height) {
	vk_engine_t* e = (vk_engine_t*)calloc(1,sizeof(vk_engine_t));

	glfwInit();
	assert (glfwVulkanSupported()==GLFW_TRUE);

	uint32_t enabledExtsCount = 0, phyCount = 0;
	const char** gflwExts = glfwGetRequiredInstanceExtensions (&enabledExtsCount);

	const char* enabledExts [10];

	for (uint32_t i=0;i<enabledExtsCount;i++)
		enabledExts[i] = gflwExts[i];
#ifdef VKVG_USE_RENDERDOC
	const uint32_t enabledLayersCount = 2;
	const char* enabledLayers[] = {"VK_LAYER_KHRONOS_validation", "VK_LAYER_RENDERDOC_Capture"};
#elif defined (VKVG_USE_VALIDATION)
	const uint32_t enabledLayersCount = 1;
	const char* enabledLayers[] = {"VK_LAYER_KHRONOS_validation"};
#else
	const uint32_t enabledLayersCount = 0;
	const char** enabledLayers = NULL;
#endif
#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	enabledExts[enabledExtsCount++] = "VK_EXT_debug_utils";
#endif
	uint32_t instanceExtCount;
	VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &instanceExtCount, NULL));
	VkExtensionProperties* instanceExtProps =(VkExtensionProperties*)malloc(instanceExtCount * sizeof(VkExtensionProperties));
	VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &instanceExtCount, instanceExtProps));

	if (instance_extension_supported(instanceExtProps, instanceExtCount, "VK_KHR_get_physical_device_properties2"))
		enabledExts[enabledExtsCount++] = "VK_KHR_get_physical_device_properties2";

	free(instanceExtProps);

	e->app = vkh_app_create("vkvgTest", enabledLayersCount, enabledLayers, enabledExtsCount, enabledExts);
#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_app_enable_debug_messenger(e->app
								   , VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
								   //| VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
								   //| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
								   , VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
								   //| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
								   //| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
								   //| VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
								   , NULL);
#endif

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE,  GLFW_TRUE);
	glfwWindowHint(GLFW_FLOATING,   GLFW_FALSE);
	glfwWindowHint(GLFW_DECORATED,  GLFW_TRUE);

	e->window = glfwCreateWindow ((int)width, (int)height, "Window Title", NULL, NULL);

	VkSurfaceKHR surf;
	VK_CHECK_RESULT (glfwCreateWindowSurface(e->app->inst, e->window, NULL, &surf))

	VkhPhyInfo* phys = vkh_app_get_phyinfos (e->app, &phyCount, surf);

	VkhPhyInfo pi = 0;
	if (!vkengine_try_get_phyinfo(phys, phyCount, preferedGPU, &pi))
		if (!vkengine_try_get_phyinfo(phys, phyCount, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, &pi))
			if (!vkengine_try_get_phyinfo(phys, phyCount, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, &pi))
				pi = phys[0];

	if (pi) {
		e->memory_properties = pi->memProps;
		e->gpu_props = pi->properties;
	}

	uint32_t qCount = 0;
	float qPriorities[] = {0.0};

	VkDeviceQueueCreateInfo pQueueInfos[] = { {0},{0},{0} };
	if (vkh_phyinfo_create_presentable_queues	(pi, 1, qPriorities, &pQueueInfos[qCount]))
		qCount++;
	/*if (vkh_phyinfo_create_compute_queues		(pi, 1, qPriorities, &pQueueInfos[qCount]))
		qCount++;
	if (vkh_phyinfo_create_transfer_queues		(pi, 1, qPriorities, &pQueueInfos[qCount]))
		qCount++;*/

	enabledExtsCount=0;
	enabledExts[enabledExtsCount] = "VK_KHR_swapchain";
	enabledExtsCount++;

	if (vkh_phyinfo_try_get_extension_properties(pi, "VK_EXT_blend_operation_advanced", NULL))
		enabledExts[enabledExtsCount++] = "VK_EXT_blend_operation_advanced";

	if (vkh_phyinfo_try_get_extension_properties(pi, "VK_KHR_portability_subset", NULL))
		enabledExts[enabledExtsCount++] = "VK_KHR_portability_subset";

	VkPhysicalDeviceFeatures enabledFeatures = {
		.fillModeNonSolid = true,
		//.sampleRateShading = true
	};

	VkDeviceCreateInfo device_info = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
									   .queueCreateInfoCount = qCount,
									   .pQueueCreateInfos = (VkDeviceQueueCreateInfo*)&pQueueInfos,
									   .enabledExtensionCount = enabledExtsCount,
									   .ppEnabledExtensionNames = enabledExts,
									   .pEnabledFeatures = &enabledFeatures};

	e->dev = vkh_device_create(e->app, pi, &device_info);

	e->renderer = vkh_presenter_create
			(e->dev, (uint32_t) pi->pQueue, surf, width, height, VK_FORMAT_B8G8R8A8_SRGB, presentMode);

	vkh_app_free_phyinfos (phyCount, phys);

	return e;
}

void vkengine_destroy (VkEngine e) {
	//vkDeviceWaitIdle(e->dev->dev);

	VkSurfaceKHR surf = e->renderer->surface;

	vkh_presenter_destroy (e->renderer);
	vkDestroySurfaceKHR (e->app->inst, surf, NULL);

	vkh_device_destroy (e->dev);

	glfwDestroyWindow (e->window);
	vkh_app_destroy (e->app);

	glfwTerminate ();

	free(e);
}
void vkengine_close (VkEngine e) {
	glfwSetWindowShouldClose(e->window, GLFW_TRUE);
}
void vkengine_blitter_run (VkEngine e, VkImage img, uint32_t width, uint32_t height) {
	VkhPresenter p = e->renderer;
	vkh_presenter_build_blit_cmd (p, img, width, height);

	while (!vkengine_should_close (e)) {
		glfwPollEvents();
		if (!vkh_presenter_draw (p))
			vkh_presenter_build_blit_cmd (p, img, width, height);
	}
}
bool vkengine_should_close (VkEngine e) {
	return glfwWindowShouldClose (e->window);
}
void vkengine_set_title (VkEngine e, const char* title) {
	glfwSetWindowTitle(e->window, title);
}
VkDevice vkengine_get_device (VkEngine e){
	return e->dev->dev;
}
VkPhysicalDevice vkengine_get_physical_device (VkEngine e){
	return e->dev->phy;
}
VkQueue vkengine_get_queue (VkEngine e){
	return e->renderer->queue;
}
uint32_t vkengine_get_queue_fam_idx (VkEngine e){
	return e->renderer->qFam;
}
void vkengine_wait_idle (VkEngine e) {
	vkDeviceWaitIdle(e->dev->dev);
}

void vkengine_set_key_callback (VkEngine e, GLFWkeyfun key_callback){
	glfwSetKeyCallback (e->window, key_callback);
}
void vkengine_set_mouse_but_callback (VkEngine e, GLFWmousebuttonfun onMouseBut){
	glfwSetMouseButtonCallback(e->window, onMouseBut);
}
void vkengine_set_cursor_pos_callback (VkEngine e, GLFWcursorposfun onMouseMove){
	glfwSetCursorPosCallback(e->window, onMouseMove);
}
void vkengine_set_scroll_callback (VkEngine e, GLFWscrollfun onScroll){
	glfwSetScrollCallback(e->window, onScroll);
}
void vkengine_set_char_callback (VkEngine e, GLFWcharfun onChar){
	glfwSetCharCallback(e->window, onChar);
}


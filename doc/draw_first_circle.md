# How to draw the first circle

This tutorial shows how one develops an application based on vulkan which interacts with the vkvg library. Throughout this document you will be refered to online documentation whenever necessary. The full sample is located [here](sample_1.cpp), follow the code as you read this document.

Necessary steps to follow the tutorial:
1. Complete the steps necessary to build the library.
2. Have a C++ compiler compatible with C++17.
3. Set up the project according to your development environment.
4. To understand the nomenclature use throught the document take a look through [Alexander Overvoorde](https://vulkan-tutorial.com/) tutorial

This document is simillar to the tutorial developed by [Alexander Overvoorde](https://vulkan-tutorial.com/). The functions which are unchanged from this reference are used without further explanation. On the contrary, any change is adressed and explained.

To start we create a vulkan instance throught the function `init_instance()` which initializes the Vulkan library. To receive feedback from the Vulkan debug layers the function `setupDebugMessenger()`establishes the necessary callbacks. To print to the screen create the surface associated with your operating system through the GFLW library by calling `init_surface()`.

Until now all function calls are simillar to the ones written by Alexander. The first divergence comes while selecting the queue families which will be used to present and to process the graphics processing necessary by this application. To simplify the tutorial the queue family of the physical device must support both graphics and presentation operations as it is request in the following function: 

```c++
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {

	VkBool32 presentSupport = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

	if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport ) {
		indices.graphicsAndPresentationFamily = i;
		break;
	}
	i++;
	}
	return indices;
}
```

With the physical device selected one can create the logical device through the function call `createLogicalDevice()`.

The second divergence from Alexander comes with the swapchain creation. To interact with GFLW surface, vkvg requires support for copy operations, which translates into the following logic in `querySwapChainSupport()`:
```c++
SwapChainSupportDetails details;

vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
if (!(details.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
	throw std::exception("glfw surface does not support copy operations");
}
```
To support copy operations between vkvg and the tutorial, the internal image representations must be equal. This means that while calling the function chooseSwapSurfaceFormat() one should request a VkSurfaceFormatKHR equal to VK_FORMAT_B8G8R8A8_UNORM or terminate the application otherwise.

To establish the link between the internal memory block used by the vkvg surface and the swapchain the struct VkSwapchainCreateInfoKHR must be have the imageUsage field changed to both color attachment and as a destination of memory copies from the vkvg surface as seen in the following code:

```c++
VkSwapchainCreateInfoKHR createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
createInfo.pNext = NULL;
createInfo.flags = 0;
createInfo.surface = surface;
createInfo.minImageCount = imageCount;
createInfo.imageFormat = surfaceFormat.format;
createInfo.imageColorSpace = surfaceFormat.colorSpace;
createInfo.imageExtent = extent;
createInfo.imageArrayLayers = 1;
createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
createInfo.queueFamilyIndexCount = 0; 
createInfo.pQueueFamilyIndices = NULL;
createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
createInfo.presentMode = presentMode;
createInfo.clipped = VK_TRUE;
createInfo.oldSwapchain = NULL;
```
After creating the swapchain one must check that the format properties of the physical device support the tilling features required by vkvg. Assuming that the library was built with the VKVG_TILING_OPTIMAL option enabled then one can check the supported tilling features by writing: 

```c++
if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)){
	throw std::exception("Format of swapchain does not support vkvg required VKVG_TILING_OPTIMAL optimal");
}
```
If everything went well one can now create the image views associated with the swapchain by calling `createImageViews()`, then creating the command pool `createCommandPool()` and finally creating the `createCommandBuffers()`. The next change comes from the syncronization objects. 

Instead of creating one graphics and presentation semaphores for each swapchain image we only have one semaphore associated with the operation of presenting the image to the screen, which is done with `createSyncObjects()` function call. 

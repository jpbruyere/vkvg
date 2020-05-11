This tutorial shows how a sample of the vkvg library is coded from the ground up and how one creates an application based on vulkan which interacts with this library. Throught this document you will be refered to online documentation whenever necessary. 

Necessary steps to follow the tutorial
1. Complete the steps necessary to build the library.
2. Have a C++ compiler compatible with C++17.
3. Set up the project according to your development environment.

This document is simillar to the tutorial developed by [Alexander Overvoorde](https://vulkan-tutorial.com/) but it uses a nomenclature simillar to the one introduced in the Vulkan samples. 

To start we create a vulkan instance throught the function
```batch
init_instance();
```

This initializes the Vulkan library.

The following call provides a callback function to receive feedback from the Vulkan layers.
```batch
setupDebugMessenger();
```

To print to the screen create the surface associated with your operating system through the GFLW library by calling
```batch
init_surface();
```
Until now all function calls are simillar to the ones written by Alexander. The first divergence comes while selecting the queue families which will be used to present and to process the graphics processing necessary by this application. To simplify the tutorial the queue family of the physical device must support both graphics and presentation information as it is request in the following function: 

```batch
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

With the physical device select one can create the logical device through the function call:

```batch
createLogicalDevice();
```

The second divergence from Alexander comes with the swapchain creation. To establish a link between the vkvg surface used to draw 

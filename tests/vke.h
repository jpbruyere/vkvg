#ifndef VKE_H
#define VKE_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "vkh.h"

#define APP_SHORT_NAME "vkcrow_test"
/* Number of samples needs to be the same at image creation,      */
/* renderpass creation and pipeline creation.                     */
#define FENCE_TIMEOUT 100000000

//#include "vkh_presenter.h"

typedef struct ImageBuffer_t {
    VkImage     image;
    VkImageView view;
}ImageBuffer;

typedef struct vkh_presenter_t {
    VkQueue         queue;
    VkCommandPool   cmdPool;
    uint32_t        qFam;
    VkDevice        dev;

    GLFWwindow*     window;
    VkSurfaceKHR    surface;

    VkSemaphore     semaPresentEnd;
    VkSemaphore     semaDrawEnd;

    VkFormat        format;
    VkColorSpaceKHR colorSpace;
    VkPresentModeKHR presentMode;
    uint32_t        width;
    uint32_t        height;

    uint32_t        imgCount;
    uint32_t        currentScBufferIndex;

    VkRenderPass    renderPass;
    VkSwapchainKHR  swapChain;
    ImageBuffer*    ScBuffers;
    VkCommandBuffer* cmdBuffs;
    VkFramebuffer*  frameBuffs;
}vkh_presenter;

typedef struct VkLoader_t {
    VkQueue queue;
    VkCommandPool cmdPool;
}VkLoader;

typedef struct VkComputer_t {
    VkQueue queue;
    VkCommandPool cmdPool;
}VkComputer;

typedef struct VkEngine_t {
    VkhApp              app;
    VkPhysicalDevice    phy;
    VkPhysicalDeviceMemoryProperties    memory_properties;
    VkPhysicalDeviceProperties          gpu_props;
    VkDevice            dev;

    vkh_presenter       renderer;
    VkComputer          computer;
    VkLoader            loader;
}VkEngine;
#endif

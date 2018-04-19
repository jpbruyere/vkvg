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

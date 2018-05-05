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
#ifndef VKENGINE_H
#define VKENGINE_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "vkh.h"

/* Number of samples needs to be the same at image creation,      */
/* renderpass creation and pipeline creation.                     */
#define FENCE_TIMEOUT 100000000


typedef struct _vk_engine_t {
    VkhApp              app;
    VkPhysicalDeviceMemoryProperties    memory_properties;
    VkPhysicalDeviceProperties          gpu_props;
    VkhDevice           dev;
    GLFWwindow*         window;

    VkhPresenter        renderer;
}vk_engine_t;

vk_engine_t*   vke_create  (VkPhysicalDeviceType preferedGPU, uint32_t width, uint32_t height);
void        vke_destroy (vk_engine_t* e);

void initPhySurface(VkhPresenter r, VkFormat preferedFormat, VkPresentModeKHR presentMode);

VkSampleCountFlagBits getMaxUsableSampleCount(VkSampleCountFlags counts);

void vkengine_dump_Infos (vk_engine_t* e);
void vkengine_get_queues_properties (vk_engine_t* e, VkQueueFamilyProperties** qFamProps, uint32_t* count);


//void vke_init_blit_renderer (VkhPresenter r, VkImage blitSource);
bool vke_should_close       (vk_engine_t* e);

//void submitCommandBuffer(VkQueue queue, VkCommandBuffer *pCmdBuff, VkSemaphore* pWaitSemaphore, VkSemaphore* pSignalSemaphore);
//void draw(vk_engine_t* e, VkImage blitSource);
#endif

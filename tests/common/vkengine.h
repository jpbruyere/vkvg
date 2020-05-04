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

#define FENCE_TIMEOUT 100000000

typedef struct _vk_engine_t* VkEngine;

typedef struct _vk_engine_t {
    VkhApp              app;
    VkPhysicalDeviceMemoryProperties    memory_properties;
    VkPhysicalDeviceProperties          gpu_props;
    VkhDevice           dev;
    GLFWwindow*         window;
    VkhPresenter        renderer;
}vk_engine_t;

vk_engine_t*   vkengine_create  (VkPhysicalDeviceType preferedGPU, VkPresentModeKHR presentMode, uint32_t width, uint32_t height);
void vkengine_dump_available_layers   ();
void vkengine_destroy       (VkEngine e);
bool vkengine_should_close  (VkEngine e);
void vkengine_close         (VkEngine e);
void vkengine_dump_Infos    (VkEngine e);
void vkengine_set_title     (VkEngine e, const char* title);
VkDevice            vkengine_get_device         (VkEngine e);
VkPhysicalDevice    vkengine_get_physical_device(VkEngine e);
VkQueue             vkengine_get_queue          (VkEngine e);
uint32_t            vkengine_get_queue_fam_idx  (VkEngine e);

void vkengine_get_queues_properties (vk_engine_t* e, VkQueueFamilyProperties** qFamProps, uint32_t* count);

void vkengine_set_key_callback          (VkEngine e, GLFWkeyfun key_callback);
void vkengine_set_mouse_but_callback    (VkEngine e, GLFWmousebuttonfun onMouseBut);
void vkengine_set_cursor_pos_callback   (VkEngine e, GLFWcursorposfun onMouseMove);
void vkengine_set_scroll_callback       (VkEngine e, GLFWscrollfun onScroll);
void vkengine_set_char_callback         (VkEngine e, GLFWcharfun onChar);
#endif

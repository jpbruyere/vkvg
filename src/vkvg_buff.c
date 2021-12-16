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

#include "vkvg_buff.h"
#include "vkvg_device_internal.h"

void vkvg_buffer_create(VkvgDevice pDev, VkBufferUsageFlags usage, VmaMemoryUsage memoryPropertyFlags, VkDeviceSize size, vkvg_buff *buff){
	buff->pDev = pDev;
	VkBufferCreateInfo bufCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.usage = usage, .size = size, .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
	VmaAllocationCreateInfo allocInfo = { .usage = memoryPropertyFlags, .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT };

	VK_CHECK_RESULT(vmaCreateBuffer (pDev->allocator, &bufCreateInfo, &allocInfo, &buff->buffer, &buff->alloc, &buff->allocInfo));
}

void vkvg_buffer_destroy(vkvg_buff *buff){
	vmaDestroyBuffer (buff->pDev->allocator, buff->buffer, buff->alloc);
}
void vkvg_buffer_flush (vkvg_buff* buff){
	vmaFlushAllocation (buff->pDev->allocator, buff->alloc, buff->allocInfo.offset, buff->allocInfo.size);
}

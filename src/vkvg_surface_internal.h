/*
 * Copyright (c) 2018-2020 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
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
#ifndef SURFACE_INTERNAL_H
#define SURFACE_INTERNAL_H

#include "vkvg_internal.h"
#include "vkvg.h"
#include "vkh.h"

typedef struct _vkvg_surface_t {
	vkvg_status_t	status;					/**< Current status of surface, affected by last operation */
	uint32_t		references;
	VkvgDevice		dev;
	uint32_t		width;
	uint32_t		height;
	VkFormat		format;
	VkFramebuffer	fb;
	VkhImage		img;
	VkhImage		imgMS;
	VkhImage		stencil;
	bool			newSurf;
	mtx_t			mutex;
#ifdef VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
	VkSemaphore		timeline;				/**< Timeline semaphore */
	uint64_t		timelineStep;
#endif
}vkvg_surface;

#define LOCK_SURFACE(surf) \
	if (surf->dev->threadAware)\
		mtx_lock (&surf->mutex);
#define UNLOCK_SURFACE(surf) \
	if (surf->dev->threadAware)\
		mtx_unlock (&surf->mutex);

void _explicit_ms_resolve (VkvgSurface surf);
void _clear_surface (VkvgSurface surf, VkImageAspectFlags aspect);
void _create_surface_main_image (VkvgSurface surf);
void _create_surface_secondary_images (VkvgSurface surf);
void _create_framebuffer (VkvgSurface surf);
void _create_surface_images (VkvgSurface surf);
VkvgSurface _create_surface (VkvgDevice dev, VkFormat format);
#endif

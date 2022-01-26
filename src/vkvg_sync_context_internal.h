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
#ifndef VKVG_SYNC_CONTEXT_INTERNAL_H
#define VKVG_SYNC_CONTEXT_INTERNAL_H

#include "vkvg_internal.h"
#include "vkvg.h"
#include "vkh.h"

typedef struct _vkvg_sync_context* vkvgSync;

typedef struct _vkvg_sync_context {
	VkvgDevice	dev;
	VkFence		fence;			// this fence is kept signaled when idle, wait and reset are called before each recording.
	VkSemaphore signals[2];		// dual signal semaphore couple with the fence guard ensure sync in multithreaded.
	bool		nextSubmIdx;	// switch between signal[0] and [1], index of the next sema to use as signal

	vkvgSync	awaitedBy;
	vkvgSync	awaiting;
}vkvg_sync_context;

void	_sync_context_init				(VkvgDevice dev, vkvg_sync_context* ctx);
void	_sync_context_clear				(vkvgSync ctx);
VkResult _sync_context_wait				(vkvgSync ctx, uint64_t timeout);
VkResult _sync_context_wait_and_reset	(vkvgSync ctx, uint64_t timeout);

void _sync_context_dbg_print (const char *label, vkvg_sync_context* ctx);
#endif

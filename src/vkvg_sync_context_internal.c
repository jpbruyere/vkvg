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

#include "vkvg_sync_context_internal.h"
#include "vkvg_device_internal.h"

void _sync_context_init (VkvgDevice dev, vkvg_sync_context* ctx) {
	VkhDevice d = (VkhDevice)dev;
	ctx->dev		= dev;
	ctx->fence		= vkh_fence_create_signaled	(d);
	ctx->signals[0] = vkh_semaphore_create		(d);
	ctx->signals[1] = vkh_semaphore_create		(d);
}
void _sync_context_clear (vkvgSync ctx) {
	if (ctx->dev->gQLockGuard)
		ctx->dev->gQLockGuard (ctx->dev->gQGuardUserData);

	_sync_context_dbg_print("cleared:", ctx);
	//_sync_context_dbg_print("signaled:", ctx->dev->gQLastSignaledSync);
	//_sync_context_dbg_print("awaited :", ctx->dev->gQLastAwaitedSync);

	/*if (ctx->dev->gQLastAwaitedSync == ctx) {
		vkWaitForFences(ctx->dev->vkDev, 1,	&ctx->dev->gQLastAwaitedSync->fence, VK_TRUE, UINT64_MAX);
		if (ctx->dev->gQLastSignaledSync)
			vkWaitForFences(ctx->dev->vkDev, 1,	&ctx->dev->gQLastSignaledSync->fence, VK_TRUE, UINT64_MAX);
		ctx->dev->gQLastAwaitedSync = NULL;
		ctx->dev->gQLastSignaledSync = NULL;
	}*/
	if (ctx->dev->gQLastSignaledSync == ctx)
		ctx->dev->gQLastSignaledSync = NULL;

	VkDevice dev = ctx->dev->vkDev;

	if (ctx->awaiting) {
		ctx->awaiting->awaitedBy = NULL;
		ctx->awaiting = NULL;
	}

	if (ctx->awaitedBy) {
		vkWaitForFences(dev, 1, &ctx->awaitedBy->fence, VK_TRUE, UINT64_MAX);
		ctx->awaitedBy = NULL;
	}
	vkWaitForFences(dev, 1, &ctx->fence, VK_TRUE, UINT64_MAX);

	vkDestroyFence		(dev, ctx->fence,		NULL);
	vkDestroySemaphore	(dev, ctx->signals[0],	NULL);
	vkDestroySemaphore	(dev, ctx->signals[1],	NULL);

	if (ctx->dev->gQUnlockGuard)
		ctx->dev->gQUnlockGuard (ctx->dev->gQGuardUserData);
}
VkResult _sync_context_wait (vkvgSync ctx, uint64_t timeout) {
	return vkWaitForFences (ctx->dev->vkDev, 1, &ctx->fence, VK_TRUE, timeout);
}
VkResult _sync_context_wait_and_reset (vkvgSync ctx, uint64_t timeout) {
	VkResult res = vkWaitForFences (ctx->dev->vkDev, 1, &ctx->fence, VK_TRUE, timeout);
	if (ctx->dev->gQLockGuard)
		ctx->dev->gQLockGuard (ctx->dev->gQGuardUserData);
	_sync_context_dbg_print("reset:", ctx);
	vkResetFences (ctx->dev->vkDev, 1, &ctx->fence);
	if (ctx->dev->gQUnlockGuard)
		ctx->dev->gQUnlockGuard (ctx->dev->gQGuardUserData);
	return res;
}
void _sync_context_dbg_print (const char* label, vkvg_sync_context* ctx) {
	if (ctx)
		printf("\t\t%20s ctx: %16p sigA: %16p sigB: %16p\n", label, ctx, ctx->signals[0], ctx->signals[1]);
	else
		printf("\t\t%20s ctx: %16p\n", label, ctx);
}


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

//credits for bezier algorithms to:
//		Anti-Grain Geometry (AGG) - Version 2.5
//		A high quality rendering engine for C++
//		Copyright (C) 2002-2006 Maxim Shemanarev
//		Contact: mcseem@antigrain.com
//				 mcseemagg@yahoo.com
//				 http://antigrain.com


#include "vkvg_surface_internal.h"
#include "vkvg_context_internal.h"
#include "vkvg_device_internal.h"
#include "vkvg_pattern.h"
#include "vkh_queue.h"
#include "vkh_image.h"

#ifdef VKVG_FILL_NZ_GLUTESS
#include "glutess.h"
#endif

void _resize_vertex_cache (VkvgContext ctx, uint32_t newSize) {
	Vertex* tmp = (Vertex*) realloc (ctx->vertexCache, (size_t)newSize * sizeof(Vertex));
	LOG(VKVG_LOG_DBG_ARRAYS, "resize vertex cache (vx count=%u): old size: %u -> new size: %u size(byte): %zu Ptr: %p -> %p\n",
		ctx->vertCount, ctx->sizeVertices, newSize, (size_t)newSize * sizeof(Vertex), ctx->vertexCache, tmp);
	if (tmp == NULL){
		ctx->status = VKVG_STATUS_NO_MEMORY;
		LOG(VKVG_LOG_ERR, "resize vertex cache failed: vert count: %u byte size: %zu\n", newSize, newSize * sizeof(Vertex));
		return;
	}
	ctx->vertexCache = tmp;
	ctx->sizeVertices = newSize;
}
void _resize_index_cache (VkvgContext ctx, uint32_t newSize) {
	VKVG_IBO_INDEX_TYPE* tmp = (VKVG_IBO_INDEX_TYPE*) realloc (ctx->indexCache, (size_t)newSize * sizeof(VKVG_IBO_INDEX_TYPE));
	LOG(VKVG_LOG_DBG_ARRAYS, "resize IBO: new size: %lu Ptr: %p -> %p\n", (size_t)newSize * sizeof(VKVG_IBO_INDEX_TYPE), ctx->indexCache, tmp);
	if (tmp == NULL){
		ctx->status = VKVG_STATUS_NO_MEMORY;
		LOG(VKVG_LOG_ERR, "resize IBO failed: idx count: %u size(byte): %zu\n", newSize, (size_t)newSize * sizeof(VKVG_IBO_INDEX_TYPE));
		return;
	}
	ctx->indexCache = tmp;
	ctx->sizeIndices = newSize;
}
void _ensure_vertex_cache_size (VkvgContext ctx, uint32_t addedVerticesCount) {
	if (ctx->sizeVertices - ctx->vertCount > VKVG_ARRAY_THRESHOLD + addedVerticesCount)
		return;
	uint32_t newSize = ctx->sizeVertices + addedVerticesCount;
	uint32_t modulo = addedVerticesCount % VKVG_VBO_SIZE;
	if (modulo > 0)
		newSize += VKVG_VBO_SIZE - modulo;
	_resize_vertex_cache (ctx, newSize);
}
void _check_vertex_cache_size (VkvgContext ctx) {
	assert(ctx->sizeVertices > ctx->vertCount);
	if (ctx->sizeVertices - VKVG_ARRAY_THRESHOLD > ctx->vertCount)
		return;
	_resize_vertex_cache (ctx, ctx->sizeVertices + VKVG_VBO_SIZE);
}
void _ensure_index_cache_size (VkvgContext ctx, uint32_t addedIndicesCount) {
	assert(ctx->sizeIndices > ctx->indCount);
	if (ctx->sizeIndices - VKVG_ARRAY_THRESHOLD > ctx->indCount + addedIndicesCount)
		return;
	uint32_t newSize = ctx->sizeIndices + addedIndicesCount;
	uint32_t modulo = addedIndicesCount % VKVG_IBO_SIZE;
	if (modulo > 0)
		newSize += VKVG_IBO_SIZE - modulo;
	_resize_index_cache (ctx, newSize);
}
void _check_index_cache_size (VkvgContext ctx) {
	if (ctx->sizeIndices - VKVG_ARRAY_THRESHOLD > ctx->indCount)
		return;
	_resize_index_cache (ctx, ctx->sizeIndices + VKVG_IBO_SIZE);
}
//check host path array size, return true if error. pathPtr is already incremented
bool _check_pathes_array (VkvgContext ctx){
	if (ctx->sizePathes - ctx->pathPtr - ctx->segmentPtr > VKVG_ARRAY_THRESHOLD)
		return false;
	ctx->sizePathes += VKVG_PATHES_SIZE;
	uint32_t* tmp = (uint32_t*) realloc (ctx->pathes, (size_t)ctx->sizePathes * sizeof(uint32_t));
	LOG(VKVG_LOG_DBG_ARRAYS, "resize PATH: new size: %u Ptr: %p -> %p\n", ctx->sizePathes, ctx->pathes, tmp);
	if (tmp == NULL){
		ctx->status = VKVG_STATUS_NO_MEMORY;
		LOG(VKVG_LOG_ERR, "resize PATH failed: new size(byte): %zu\n", ctx->sizePathes * sizeof(uint32_t));
		_clear_path(ctx);
		return true;
	}
	ctx->pathes = tmp;
	return false;
}
//check host point array size, return true if error
bool _check_point_array (VkvgContext ctx){
	if (ctx->sizePoints - VKVG_ARRAY_THRESHOLD > ctx->pointCount)
		return false;
	ctx->sizePoints += VKVG_PTS_SIZE;
	vec2* tmp = (vec2*) realloc (ctx->points, (size_t)ctx->sizePoints * sizeof(vec2));
	LOG(VKVG_LOG_DBG_ARRAYS, "resize Points: new size(point): %u Ptr: %p -> %p\n", ctx->sizePoints, ctx->points, tmp);
	if (tmp == NULL){
		ctx->status = VKVG_STATUS_NO_MEMORY;
		LOG(VKVG_LOG_ERR, "resize PATH failed: new size(byte): %zu\n", ctx->sizePoints * sizeof(vec2));
		_clear_path (ctx);
		return true;
	}
	ctx->points = tmp;
	return false;
}
bool _current_path_is_empty (VkvgContext ctx) {
	return ctx->pathes [ctx->pathPtr] == 0;
}
//this function expect that current point exists
vec2 _get_current_position (VkvgContext ctx) {
	return ctx->points[ctx->pointCount-1];
}
//set curve start point and set path has curve bit
void _set_curve_start (VkvgContext ctx) {
	if (ctx->segmentPtr > 0) {
		//check if current segment has points (straight)
		if ((ctx->pathes [ctx->pathPtr + ctx->segmentPtr]&PATH_ELT_MASK) > 0)
			ctx->segmentPtr++;
	}else{
		//not yet segmented path, first segment length is copied
		if (ctx->pathes [ctx->pathPtr] > 0){//create first straight segment first
			ctx->pathes [ctx->pathPtr + 1] = ctx->pathes [ctx->pathPtr];
			ctx->segmentPtr = 2;
		}else
			ctx->segmentPtr = 1;
	}
	_check_pathes_array(ctx);
	ctx->pathes [ctx->pathPtr + ctx->segmentPtr] = 0;
}
//compute segment length and set is curved bit
void _set_curve_end (VkvgContext ctx) {
	//ctx->pathes [ctx->pathPtr + ctx->segmentPtr] = ctx->pathes [ctx->pathPtr] - ctx->pathes [ctx->pathPtr + ctx->segmentPtr];
	ctx->pathes [ctx->pathPtr + ctx->segmentPtr] |= PATH_HAS_CURVES_BIT;
	ctx->segmentPtr++;
	_check_pathes_array(ctx);
	ctx->pathes [ctx->pathPtr + ctx->segmentPtr] = 0;
}
//path start pointed at ptrPath has curve bit
bool _path_has_curves (VkvgContext ctx, uint32_t ptrPath) {
	return ctx->pathes[ptrPath] & PATH_HAS_CURVES_BIT;
}
void _finish_path (VkvgContext ctx){
	if (ctx->pathes [ctx->pathPtr] == 0)//empty
		return;
	if ((ctx->pathes [ctx->pathPtr]&PATH_ELT_MASK) < 2){
		//only current pos is in path
		ctx->pointCount -= ctx->pathes[ctx->pathPtr];//what about the bounds?
		ctx->pathes[ctx->pathPtr] = 0;
		ctx->segmentPtr = 0;
		return;
	}

	LOG(VKVG_LOG_INFO_PATH, "PATH: points count=%10d\n", ctx->pathes[ctx->pathPtr]&PATH_ELT_MASK);

	if (ctx->pathPtr == 0 && ctx->simpleConvex)
		ctx->pathes[0] |= PATH_IS_CONVEX_BIT;

	if (ctx->segmentPtr > 0) {
		ctx->pathes[ctx->pathPtr] |= PATH_HAS_CURVES_BIT;
		//if last segment is not a curve and point count > 0
		if ((ctx->pathes[ctx->pathPtr+ctx->segmentPtr]&PATH_HAS_CURVES_BIT)==0 &&
				(ctx->pathes[ctx->pathPtr+ctx->segmentPtr]&PATH_ELT_MASK) > 0)
			ctx->segmentPtr++;//current segment has to be included
		ctx->pathPtr += ctx->segmentPtr;
	}else
		ctx->pathPtr ++;

	if (_check_pathes_array(ctx))
		return;

	ctx->pathes[ctx->pathPtr] = 0;
	ctx->segmentPtr = 0;
	ctx->subpathCount++;
	ctx->simpleConvex = false;
}
//clear path datas in context
void _clear_path (VkvgContext ctx){
	ctx->pathPtr = 0;
	ctx->pathes [ctx->pathPtr] = 0;
	ctx->pointCount = 0;
	ctx->segmentPtr = 0;
	ctx->subpathCount = 0;
	ctx->simpleConvex = false;
}
void _remove_last_point (VkvgContext ctx){
	ctx->pathes[ctx->pathPtr]--;
	ctx->pointCount--;
	if (ctx->segmentPtr > 0){//if path is segmented
		if (!ctx->pathes [ctx->pathPtr + ctx->segmentPtr])//if current segment is empty
			ctx->segmentPtr--;
		ctx->pathes [ctx->pathPtr + ctx->segmentPtr]--;//decrement last segment point count
		if ((ctx->pathes [ctx->pathPtr + ctx->segmentPtr]&PATH_ELT_MASK) == 0)//if no point left (was only one)
			ctx->pathes [ctx->pathPtr + ctx->segmentPtr] = 0;//reset current segment
		else if (ctx->pathes [ctx->pathPtr + ctx->segmentPtr]&PATH_HAS_CURVES_BIT)//if segment is a curve
			ctx->segmentPtr++;//then segPtr has to be forwarded to new segment
	}
}
bool _path_is_closed (VkvgContext ctx, uint32_t ptrPath){
	return ctx->pathes[ptrPath] & PATH_CLOSED_BIT;
}
void _add_point (VkvgContext ctx, float x, float y){
	if (_check_point_array(ctx))
		return;
	if (isnan(x) || isnan(y)) {
		LOG(VKVG_LOG_DEBUG, "_add_point: (%f, %f)\n", x, y);
		return;
	}
	vec2 v = {x,y};
	/*if (!_current_path_is_empty(ctx) && vec2_length(vec2_sub(ctx->points[ctx->pointCount-1], v))<1.f)
		return;*/
	LOG(VKVG_LOG_INFO_PTS, "_add_point: (%f, %f)\n", x, y);

	ctx->points[ctx->pointCount] = v;
	ctx->pointCount++;//total point count of pathes, (for array bounds check)
	ctx->pathes[ctx->pathPtr]++;//total point count in path
	if (ctx->segmentPtr > 0)
		ctx->pathes[ctx->pathPtr + ctx->segmentPtr]++;//total point count in path's segment
}
float _normalizeAngle(float a)
{
	float res = ROUND_DOWN(fmodf(a, 2.0f * M_PIF), 100);
	if (res < 0.0f)
		res += 2.0f * M_PIF;
	return res;
}
float _get_arc_step (VkvgContext ctx, float radius) {
	float sx, sy;
	vkvg_matrix_get_scale (&ctx->pushConsts.mat, &sx, &sy);
	float r = radius * fabsf(fmaxf(sx,sy));
	if (r < 30.0f)
		return fminf(M_PI / 3.f, M_PI / r);
	return fminf(M_PI / 3.f,M_PI / (r * 0.4f));
}
void _create_gradient_buff (VkvgContext ctx){
	vkvg_buffer_create (ctx->dev,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		sizeof(vkvg_gradient_t), &ctx->uboGrad);
}
void _create_vertices_buff (VkvgContext ctx){
	vkvg_buffer_create (ctx->dev,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		ctx->sizeVBO * sizeof(Vertex), &ctx->vertices);
	vkvg_buffer_create (ctx->dev,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		ctx->sizeIBO * sizeof(VKVG_IBO_INDEX_TYPE), &ctx->indices);
}
void _resize_vbo (VkvgContext ctx, uint32_t new_size) {
	if (!_wait_ctx_flush_end (ctx))//wait previous cmd if not completed
		return;
	LOG(VKVG_LOG_DBG_ARRAYS, "resize VBO: %d -> ", ctx->sizeVBO);
	ctx->sizeVBO = new_size;
	uint32_t mod = ctx->sizeVBO % VKVG_VBO_SIZE;
	if (mod > 0)
		ctx->sizeVBO += VKVG_VBO_SIZE - mod;
	LOG(VKVG_LOG_DBG_ARRAYS, "%d\n", ctx->sizeVBO);
	vkvg_buffer_destroy (&ctx->vertices);
	vkvg_buffer_create (ctx->dev,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		ctx->sizeVBO * sizeof(Vertex), &ctx->vertices);
}
void _resize_ibo (VkvgContext ctx, size_t new_size) {
	if (!_wait_ctx_flush_end (ctx))//wait previous cmd if not completed
		return;
	ctx->sizeIBO = new_size;
	uint32_t mod = ctx->sizeIBO % VKVG_IBO_SIZE;
	if (mod > 0)
		ctx->sizeIBO += VKVG_IBO_SIZE - mod;
	LOG(VKVG_LOG_DBG_ARRAYS, "resize IBO: new size: %d\n", ctx->sizeIBO);
	vkvg_buffer_destroy (&ctx->indices);
	vkvg_buffer_create (ctx->dev,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		ctx->sizeIBO * sizeof(VKVG_IBO_INDEX_TYPE), &ctx->indices);
}
void _add_vertexf (VkvgContext ctx, float x, float y){
	Vertex* pVert = &ctx->vertexCache[ctx->vertCount];
	pVert->pos.x = x;
	pVert->pos.y = y;
	pVert->color = ctx->curColor;
	pVert->uv.z = -1;
	LOG(VKVG_LOG_INFO_VBO, "Add Vertexf %10d: pos:(%10.4f, %10.4f) uv:(%10.4f,%10.4f,%10.4f) color:0x%.8x \n", ctx->vertCount, pVert->pos.x, pVert->pos.y, pVert->uv.x, pVert->uv.y, pVert->uv.z, pVert->color);
	ctx->vertCount++;
	_check_vertex_cache_size(ctx);
}
void _add_vertexf_unchecked (VkvgContext ctx, float x, float y){
	Vertex* pVert = &ctx->vertexCache[ctx->vertCount];
	pVert->pos.x = x;
	pVert->pos.y = y;
	pVert->color = ctx->curColor;
	pVert->uv.z = -1;
	LOG(VKVG_LOG_INFO_VBO, "Add Vertexf %10d: pos:(%10.4f, %10.4f) uv:(%10.4f,%10.4f,%10.4f) color:0x%.8x \n", ctx->vertCount, pVert->pos.x, pVert->pos.y, pVert->uv.x, pVert->uv.y, pVert->uv.z, pVert->color);
	ctx->vertCount++;
}
void _add_vertex(VkvgContext ctx, Vertex v){
	ctx->vertexCache[ctx->vertCount] = v;
	LOG(VKVG_LOG_INFO_VBO, "Add Vertex  %10d: pos:(%10.4f, %10.4f) uv:(%10.4f,%10.4f,%10.4f) color:0x%.8x \n", ctx->vertCount, v.pos.x, v.pos.y, v.uv.x, v.uv.y, v.uv.z, v.color);
	ctx->vertCount++;
	_check_vertex_cache_size(ctx);
}
void _set_vertex(VkvgContext ctx, uint32_t idx, Vertex v){
	ctx->vertexCache[idx] = v;
}
#ifdef VKVG_FILL_NZ_GLUTESS
void _add_indice (VkvgContext ctx, VKVG_IBO_INDEX_TYPE i) {
	ctx->indexCache[ctx->indCount++] = i;
	_check_index_cache_size(ctx);
}
void _add_indice_for_fan (VkvgContext ctx, VKVG_IBO_INDEX_TYPE i) {
	VKVG_IBO_INDEX_TYPE* inds = &ctx->indexCache[ctx->indCount];
	inds[0] = ctx->tesselator_fan_start;
	inds[1] = ctx->indexCache[ctx->indCount-1];
	inds[2] = i;
	ctx->indCount+=3;
	_check_index_cache_size(ctx);
}
void _add_indice_for_strip (VkvgContext ctx, VKVG_IBO_INDEX_TYPE i, bool odd) {
	VKVG_IBO_INDEX_TYPE* inds = &ctx->indexCache[ctx->indCount];
	if (odd) {
		inds[0] = ctx->indexCache[ctx->indCount-2];
		inds[1] = i;
		inds[2] = ctx->indexCache[ctx->indCount-1];
	} else {
		inds[0] = ctx->indexCache[ctx->indCount-1];
		inds[1] = ctx->indexCache[ctx->indCount-2];
		inds[2] = i;
	}
	ctx->indCount+=3;
	_check_index_cache_size(ctx);
}
#endif
void _add_tri_indices_for_rect (VkvgContext ctx, VKVG_IBO_INDEX_TYPE i){
	VKVG_IBO_INDEX_TYPE* inds = &ctx->indexCache[ctx->indCount];
	inds[0] = i;
	inds[1] = i+2;
	inds[2] = i+1;
	inds[3] = i+1;
	inds[4] = i+2;
	inds[5] = i+3;
	ctx->indCount+=6;

	_check_index_cache_size(ctx);
	LOG(VKVG_LOG_INFO_IBO, "Rectangle IDX: %d %d %d | %d %d %d (count=%d)\n", inds[0], inds[1], inds[2], inds[3], inds[4], inds[5], ctx->indCount);
}
void _add_triangle_indices(VkvgContext ctx, VKVG_IBO_INDEX_TYPE i0, VKVG_IBO_INDEX_TYPE i1, VKVG_IBO_INDEX_TYPE i2){
	VKVG_IBO_INDEX_TYPE* inds = &ctx->indexCache[ctx->indCount];
	inds[0] = i0;
	inds[1] = i1;
	inds[2] = i2;
	ctx->indCount+=3;

	_check_index_cache_size(ctx);
	LOG(VKVG_LOG_INFO_IBO, "Triangle IDX: %d %d %d (indCount=%d)\n", i0,i1,i2,ctx->indCount);
}
void _add_triangle_indices_unchecked (VkvgContext ctx, VKVG_IBO_INDEX_TYPE i0, VKVG_IBO_INDEX_TYPE i1, VKVG_IBO_INDEX_TYPE i2){
	VKVG_IBO_INDEX_TYPE* inds = &ctx->indexCache[ctx->indCount];
	inds[0] = i0;
	inds[1] = i1;
	inds[2] = i2;
	ctx->indCount+=3;

	LOG(VKVG_LOG_INFO_IBO, "Triangle IDX: %d %d %d (indCount=%d)\n", i0,i1,i2,ctx->indCount);
}
void _vao_add_rectangle (VkvgContext ctx, float x, float y, float width, float height){
	Vertex v[4] =
	{
		{{x,y},				ctx->curColor, {0,0,-1}},
		{{x,y+height},		ctx->curColor, {0,0,-1}},
		{{x+width,y},		ctx->curColor, {0,0,-1}},
		{{x+width,y+height},ctx->curColor, {0,0,-1}}
	};
	VKVG_IBO_INDEX_TYPE firstIdx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);
	Vertex* pVert = &ctx->vertexCache[ctx->vertCount];
	memcpy (pVert,v,4*sizeof(Vertex));
	ctx->vertCount+=4;

	_check_vertex_cache_size(ctx);

	_add_tri_indices_for_rect(ctx, firstIdx);
}
//start render pass if not yet started or update push const if requested
void _ensure_renderpass_is_started (VkvgContext ctx) {
	LOG(VKVG_LOG_INFO, "_ensure_renderpass_is_started\n");
	if (!ctx->cmdStarted)
		_start_cmd_for_render_pass(ctx);
	else if (ctx->pushCstDirty)
		_update_push_constants(ctx);
}
void _create_cmd_buff (VkvgContext ctx){
	vkh_cmd_buffs_create((VkhDevice)ctx->dev, ctx->cmdPool,VK_COMMAND_BUFFER_LEVEL_PRIMARY, 2, ctx->cmdBuffers);
#if defined(DEBUG) && defined(ENABLE_VALIDATION)
	vkh_device_set_object_name((VkhDevice)ctx->pSurf->dev, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, (uint64_t)ctx->cmd, "vkvgCtxCmd");
#endif
}
void _clear_attachment (VkvgContext ctx) {

}

bool _wait_ctx_flush_end (VkvgContext ctx) {
	LOG(VKVG_LOG_INFO, "CTX: _wait_flush_fence\n");
#ifdef VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
	if (vkh_timeline_wait ((VkhDevice)ctx->dev, ctx->pSurf->timeline, ctx->timelineStep) == VK_SUCCESS)
		return true;
#else
	if (WaitForFences (ctx->dev->vkDev, 1, &ctx->flushFence, VK_TRUE, VKVG_FENCE_TIMEOUT) == VK_SUCCESS)
		return true;
#endif
	LOG(VKVG_LOG_DEBUG, "CTX: _wait_flush_fence timeout\n");
	ctx->status = VKVG_STATUS_TIMEOUT;
	return false;
}


bool _wait_and_submit_cmd (VkvgContext ctx){
	if (!ctx->cmdStarted)//current cmd buff is empty, be aware that wait is also canceled!!
		return true;

	LOG(VKVG_LOG_INFO, "CTX: _wait_and_submit_cmd\n");

#ifdef VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
	VkvgSurface surf = ctx->pSurf;
	VkvgDevice dev = surf->dev;
	//vkh_timeline_wait ((VkhDevice)dev, surf->timeline, ct->timelineStep);
	if (ctx->pattern && ctx->pattern->type == VKVG_PATTERN_TYPE_SURFACE) {
		//add source surface timeline sync.
		VkvgSurface source = (VkvgSurface)ctx->pattern->data;
		LOCK_SURFACE(surf)
		LOCK_SURFACE(source)
		LOCK_DEVICE
		vkh_cmd_submit_timelined2 (dev->gQueue, &ctx->cmd,
								  (VkSemaphore[2]){surf->timeline,source->timeline},
								  (uint64_t[2]){surf->timelineStep,source->timelineStep},
								  (uint64_t[2]){surf->timelineStep+1,source->timelineStep+1});
		surf->timelineStep++;
		source->timelineStep++;
		ctx->timelineStep = surf->timelineStep;
		UNLOCK_DEVICE
		UNLOCK_SURFACE(source)
		UNLOCK_SURFACE(surf)
	} else {
		LOCK_SURFACE(surf)
		LOCK_DEVICE
		vkh_cmd_submit_timelined (dev->gQueue, &ctx->cmd, surf->timeline, surf->timelineStep, surf->timelineStep+1);
		surf->timelineStep++;
		ctx->timelineStep = surf->timelineStep;
		UNLOCK_DEVICE
		UNLOCK_SURFACE(surf)
	}
#else

	if (!_wait_ctx_flush_end (ctx))
		return false;
	ResetFences (ctx->dev->vkDev, 1, &ctx->flushFence);
	_device_submit_cmd (ctx->dev, &ctx->cmd, ctx->flushFence);
#endif

	if (ctx->cmd == ctx->cmdBuffers[0])
		ctx->cmd = ctx->cmdBuffers[1];
	else
		ctx->cmd = ctx->cmdBuffers[0];

	ResetCommandBuffer (ctx->cmd, 0);
	ctx->cmdStarted = false;
	return true;
}
/*void _explicit_ms_resolve (VkvgContext ctx){//should init cmd before calling this (unused, using automatic resolve by renderpass)
	vkh_image_set_layout (ctx->cmd, ctx->pSurf->imgMS, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	vkh_image_set_layout (ctx->cmd, ctx->pSurf->img, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkImageResolve re = {
		.extent = {ctx->pSurf->width, ctx->pSurf->height,1},
		.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1},
		.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1}
	};

	vkCmdResolveImage(ctx->cmd,
					  vkh_image_get_vkimage (ctx->pSurf->imgMS), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					  vkh_image_get_vkimage (ctx->pSurf->img) ,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					  1,&re);
	vkh_image_set_layout (ctx->cmd, ctx->pSurf->imgMS, VK_IMAGE_ASPECT_COLOR_BIT,
						  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ,
						  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}*/

//pre flush vertices because of vbo or ibo too small, all vertices except last draw call are flushed
//this function expects a vertex offset > 0
void _flush_vertices_caches_until_vertex_base (VkvgContext ctx) {
	_wait_ctx_flush_end (ctx);

	memcpy(ctx->vertices.allocInfo.pMappedData, ctx->vertexCache, ctx->curVertOffset * sizeof (Vertex));
	memcpy(ctx->indices.allocInfo.pMappedData, ctx->indexCache, ctx->curIndStart * sizeof (VKVG_IBO_INDEX_TYPE));

	//copy remaining vertices and indices to caches starts
	ctx->vertCount -= ctx->curVertOffset;
	ctx->indCount -= ctx->curIndStart;
	memcpy(ctx->vertexCache, &ctx->vertexCache[ctx->curVertOffset], ctx->vertCount * sizeof (Vertex));
	memcpy(ctx->indexCache, &ctx->indexCache[ctx->curIndStart], ctx->indCount * sizeof (VKVG_IBO_INDEX_TYPE));

	ctx->curVertOffset = 0;
	ctx->curIndStart = 0;
}
//copy vertex and index caches to the vbo and ibo vkbuffers used by gpu for drawing
//current running cmd has to be completed to free usage of those
void _flush_vertices_caches (VkvgContext ctx) {
	if (!_wait_ctx_flush_end (ctx))
		return;

	memcpy(ctx->vertices.allocInfo.pMappedData, ctx->vertexCache, ctx->vertCount * sizeof (Vertex));
	memcpy(ctx->indices.allocInfo.pMappedData, ctx->indexCache, ctx->indCount * sizeof (VKVG_IBO_INDEX_TYPE));

	ctx->vertCount = ctx->indCount = ctx->curIndStart = ctx->curVertOffset = 0;
}
//this func expect cmdStarted to be true
void _end_render_pass (VkvgContext ctx) {
	LOG(VKVG_LOG_INFO, "END RENDER PASS: ctx = %p;\n", ctx);
	CmdEndRenderPass	  (ctx->cmd);
#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_cmd_label_end (ctx->cmd);
#endif
	ctx->renderPassBeginInfo.renderPass = ctx->dev->renderPass;
}

void _check_vao_size (VkvgContext ctx) {
	if (ctx->vertCount > ctx->sizeVBO || ctx->indCount > ctx->sizeIBO){
		//vbo or ibo buffers too small
		if (ctx->cmdStarted)
			//if cmd is started buffers, are already bound, so no resize is possible
			//instead we flush, and clear vbo and ibo caches
			_flush_cmd_until_vx_base (ctx);
		if (ctx->vertCount > ctx->sizeVBO)		
			_resize_vbo(ctx, ctx->sizeVertices);
		if (ctx->indCount > ctx->sizeIBO)
			_resize_ibo(ctx, ctx->sizeIndices);
	}
}

//stroke and non-zero draw call for solid color flush
void _emit_draw_cmd_undrawn_vertices (VkvgContext ctx){
	if (ctx->indCount == ctx->curIndStart)
		return;

	_check_vao_size (ctx);

	_ensure_renderpass_is_started (ctx);

#ifdef VKVG_WIRED_DEBUG
	if (vkvg_wired_debug&vkvg_wired_debug_mode_normal)
		CmdDrawIndexed(ctx->cmd, ctx->indCount - ctx->curIndStart, 1, ctx->curIndStart, (int32_t)ctx->curVertOffset, 0);
	if (vkvg_wired_debug&vkvg_wired_debug_mode_lines) {
		CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineLineList);
		CmdDrawIndexed(ctx->cmd, ctx->indCount - ctx->curIndStart, 1, ctx->curIndStart, (int32_t)ctx->curVertOffset, 0);
	}
	if (vkvg_wired_debug&vkvg_wired_debug_mode_points) {
		CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineWired);
		CmdDrawIndexed(ctx->cmd, ctx->indCount - ctx->curIndStart, 1, ctx->curIndStart, (int32_t)ctx->curVertOffset, 0);
	}
	if (vkvg_wired_debug&vkvg_wired_debug_mode_both)
		CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipe_OVER);
#else
	CmdDrawIndexed(ctx->cmd, ctx->indCount - ctx->curIndStart, 1, ctx->curIndStart, (int32_t)ctx->curVertOffset, 0);
#endif
	LOG(VKVG_LOG_INFO, "RECORD DRAW CMD: ctx = %p; vertices = %d; indices = %d (vxOff = %d idxStart = %d idxTot = %d )\n",
		ctx, ctx->vertCount - ctx->curVertOffset,
		ctx->indCount - ctx->curIndStart, ctx->curVertOffset, ctx->curIndStart, ctx->indCount);

	ctx->curIndStart = ctx->indCount;
	ctx->curVertOffset = ctx->vertCount;
}
//preflush vertices with drawcommand already emited
void _flush_cmd_until_vx_base (VkvgContext ctx){
	_end_render_pass (ctx);
	if (ctx->curVertOffset > 0){
		LOG(VKVG_LOG_INFO, "FLUSH UNTIL VX BASE CTX: ctx = %p; vertices = %d; indices = %d\n", ctx, ctx->vertCount, ctx->indCount);
		_flush_vertices_caches_until_vertex_base (ctx);
	}
	vkh_cmd_end (ctx->cmd);
	_wait_and_submit_cmd (ctx);
}
void _flush_cmd_buff (VkvgContext ctx){
	_emit_draw_cmd_undrawn_vertices (ctx);
	if (!ctx->cmdStarted)
		return;
	_end_render_pass		(ctx);
	LOG(VKVG_LOG_INFO, "FLUSH CTX: ctx = %p; vertices = %d; indices = %d\n", ctx, ctx->vertCount, ctx->indCount);
	_flush_vertices_caches	(ctx);
	vkh_cmd_end				(ctx->cmd);

	_wait_and_submit_cmd	(ctx);
}

//bind correct draw pipeline depending on current OPERATOR
void _bind_draw_pipeline (VkvgContext ctx) {
	switch (ctx->curOperator) {
	case VKVG_OPERATOR_OVER:
		CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->dev->pipe_OVER);
		break;
	case VKVG_OPERATOR_CLEAR:
		CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->dev->pipe_CLEAR);
		break;
	case VKVG_OPERATOR_DIFFERENCE:
		CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->dev->pipe_SUB);
		break;
	default:
		CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->dev->pipe_OVER);
		break;
	}
}
#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
const float DBG_LAB_COLOR_RP[4]		= {0,0,1,1};
const float DBG_LAB_COLOR_FSQ[4]	= {1,0,0,1};
#endif

void _start_cmd_for_render_pass (VkvgContext ctx) {
	LOG(VKVG_LOG_INFO, "START RENDER PASS: ctx = %p\n", ctx);
	vkh_cmd_begin (ctx->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	if (ctx->pSurf->img->layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || ctx->dev->threadAware){
		VkhImage imgMs = ctx->pSurf->imgMS;
		if (imgMs != NULL)
			vkh_image_set_layout(ctx->cmd, imgMs, VK_IMAGE_ASPECT_COLOR_BIT,
								 VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
								 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		vkh_image_set_layout(ctx->cmd, ctx->pSurf->img, VK_IMAGE_ASPECT_COLOR_BIT,
						 VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencil, VK_IMAGE_ASPECT_STENCIL_BIT,
							  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
							  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
	}

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_cmd_label_start(ctx->cmd, "ctx render pass", DBG_LAB_COLOR_RP);
#endif

	CmdBeginRenderPass (ctx->cmd, &ctx->renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	VkViewport viewport = {0,0,(float)ctx->pSurf->width,(float)ctx->pSurf->height,0,1.f};
	CmdSetViewport(ctx->cmd, 0, 1, &viewport);

	CmdSetScissor(ctx->cmd, 0, 1, &ctx->bounds);

	VkDescriptorSet dss[] = {ctx->dsFont, ctx->dsSrc,ctx->dsGrad};
	CmdBindDescriptorSets(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->dev->pipelineLayout,
							0, 3, dss, 0, NULL);

	VkDeviceSize offsets[1] = { 0 };
	CmdBindVertexBuffers(ctx->cmd, 0, 1, &ctx->vertices.buffer, offsets);
	CmdBindIndexBuffer(ctx->cmd, ctx->indices.buffer, 0, VKVG_VK_INDEX_TYPE);

	_update_push_constants	(ctx);

	_bind_draw_pipeline (ctx);
	CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
	ctx->cmdStarted = true;
}
//compute inverse mat used in shader when context matrix has changed
//then trigger push constants command
void _set_mat_inv_and_vkCmdPush (VkvgContext ctx) {
	ctx->pushConsts.matInv = ctx->pushConsts.mat;
	vkvg_matrix_invert (&ctx->pushConsts.matInv);
	ctx->pushCstDirty = true;
}
void _update_push_constants (VkvgContext ctx) {
	CmdPushConstants(ctx->cmd, ctx->dev->pipelineLayout,
					   VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constants),&ctx->pushConsts);
	ctx->pushCstDirty = false;
}
void _update_cur_pattern (VkvgContext ctx, VkvgPattern pat) {
	VkvgPattern lastPat = ctx->pattern;
	ctx->pattern = pat;

	uint32_t newPatternType = VKVG_PATTERN_TYPE_SOLID;

	LOG(VKVG_LOG_INFO, "CTX: _update_cur_pattern: %p -> %p\n", lastPat, pat);

	if (pat == NULL) {//solid color
		if (lastPat == NULL)//solid
			return;//solid to solid transition, no extra action requested
	}else
		newPatternType = pat->type;

	switch (newPatternType)	 {
	case VKVG_PATTERN_TYPE_SOLID:
		_flush_cmd_buff				(ctx);
		if (!_wait_ctx_flush_end (ctx))
			return;
		if (lastPat->type == VKVG_PATTERN_TYPE_SURFACE)//unbind current source surface by replacing it with empty texture
			_update_descriptor_set		(ctx, ctx->dev->emptyImg, ctx->dsSrc);
		break;
	case VKVG_PATTERN_TYPE_SURFACE:
	{
		_emit_draw_cmd_undrawn_vertices(ctx);

		VkvgSurface surf = (VkvgSurface)pat->data;

		//flush ctx in two steps to add the src transitioning in the cmd buff
		if (ctx->cmdStarted){//transition of img without appropriate dependencies in subpass must be done outside renderpass.
			_end_render_pass (ctx);
			_flush_vertices_caches (ctx);
		}else {
			vkh_cmd_begin (ctx->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			ctx->cmdStarted = true;
		}

		//transition source surface for sampling
		vkh_image_set_layout (ctx->cmd, surf->img, VK_IMAGE_ASPECT_COLOR_BIT,
							  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
							  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		vkh_cmd_end				(ctx->cmd);
		_wait_and_submit_cmd	(ctx);
		if (!_wait_ctx_flush_end (ctx))
			return;

		ctx->source = surf->img;

		VkSamplerAddressMode addrMode = 0;
		VkFilter filter = VK_FILTER_NEAREST;
		switch (pat->extend) {
		case VKVG_EXTEND_NONE:
			addrMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			break;
		case VKVG_EXTEND_PAD:
			addrMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		case VKVG_EXTEND_REPEAT:
			addrMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		case VKVG_EXTEND_REFLECT:
			addrMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			break;
		}
		switch (pat->filter) {
		case VKVG_FILTER_BILINEAR:
		case VKVG_FILTER_BEST:
			filter = VK_FILTER_LINEAR;
			break;
		default:
			filter = VK_FILTER_NEAREST;
			break;
		}
		vkh_image_create_sampler (ctx->source, filter, filter,
									VK_SAMPLER_MIPMAP_MODE_NEAREST, addrMode);

		_update_descriptor_set (ctx, ctx->source, ctx->dsSrc);

		if (pat->hasMatrix) {

		}

		ctx->pushConsts.source.width	= (float)surf->width;
		ctx->pushConsts.source.height	= (float)surf->height;
		break;
	}
	case VKVG_PATTERN_TYPE_LINEAR:
	case VKVG_PATTERN_TYPE_RADIAL:
		_flush_cmd_buff (ctx);
		if (!_wait_ctx_flush_end (ctx))
			return;

		if (lastPat && lastPat->type == VKVG_PATTERN_TYPE_SURFACE)
			_update_descriptor_set (ctx, ctx->dev->emptyImg, ctx->dsSrc);

		vec4 bounds = {{(float)ctx->pSurf->width}, {(float)ctx->pSurf->height}, {0}, {0}};//store img bounds in unused source field
		ctx->pushConsts.source = bounds;

		//transform control point with current ctx matrix
		vkvg_gradient_t grad = *(vkvg_gradient_t*)pat->data;

		if (grad.count < 2) {
			ctx->status = VKVG_STATUS_PATTERN_INVALID_GRADIENT;
			return;
		}
		vkvg_matrix_t mat;
		if (pat->hasMatrix) {
			vkvg_pattern_get_matrix (pat, &mat);
			if (vkvg_matrix_invert (&mat) != VKVG_STATUS_SUCCESS)
				mat = VKVG_IDENTITY_MATRIX;
		}

		if (pat->hasMatrix)
			vkvg_matrix_transform_point (&mat, &grad.cp[0].x, &grad.cp[0].y);
		vkvg_matrix_transform_point (&ctx->pushConsts.mat, &grad.cp[0].x, &grad.cp[0].y);
		if (pat->type == VKVG_PATTERN_TYPE_LINEAR) {
			if (pat->hasMatrix)
				vkvg_matrix_transform_point (&mat, &grad.cp[0].z, &grad.cp[0].w);
			vkvg_matrix_transform_point (&ctx->pushConsts.mat, &grad.cp[0].z, &grad.cp[0].w);
		} else {
			if (pat->hasMatrix)
				vkvg_matrix_transform_point (&mat, &grad.cp[1].x, &grad.cp[1].y);
			vkvg_matrix_transform_point (&ctx->pushConsts.mat, &grad.cp[1].x, &grad.cp[1].y);

			//radii
			if (pat->hasMatrix) {
				vkvg_matrix_transform_distance (&mat, &grad.cp[0].z, &grad.cp[0].w);
				vkvg_matrix_transform_distance (&mat, &grad.cp[1].z, &grad.cp[0].w);
			}
			vkvg_matrix_transform_distance (&ctx->pushConsts.mat, &grad.cp[0].z, &grad.cp[0].w);
			vkvg_matrix_transform_distance (&ctx->pushConsts.mat, &grad.cp[1].z, &grad.cp[0].w);
		}

		memcpy (ctx->uboGrad.allocInfo.pMappedData , &grad, sizeof(vkvg_gradient_t));
		vkvg_buffer_flush(&ctx->uboGrad);
		break;
	}
	ctx->pushConsts.fsq_patternType = (ctx->pushConsts.fsq_patternType & FULLSCREEN_BIT) + newPatternType;
	ctx->pushCstDirty = true;
	if (lastPat)
		vkvg_pattern_destroy (lastPat);
}
void _update_descriptor_set (VkvgContext ctx, VkhImage img, VkDescriptorSet ds){
	VkDescriptorImageInfo descSrcTex = vkh_image_get_descriptor (img, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkWriteDescriptorSet writeDescriptorSet = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = ds,
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &descSrcTex
	};
	vkUpdateDescriptorSets(ctx->dev->vkDev, 1, &writeDescriptorSet, 0, NULL);
}

void _update_gradient_desc_set (VkvgContext ctx){
	VkDescriptorBufferInfo dbi = {ctx->uboGrad.buffer, 0, VK_WHOLE_SIZE};
	VkWriteDescriptorSet writeDescriptorSet = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = ctx->dsGrad,
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &dbi
	};
	vkUpdateDescriptorSets(ctx->dev->vkDev, 1, &writeDescriptorSet, 0, NULL);
}
/*
 * Reset currently bound descriptor which image could be destroyed
 */
/*void _reset_src_descriptor_set (VkvgContext ctx){
	VkvgDevice dev = ctx->pSurf->dev;
	//VkDescriptorSet dss[] = {ctx->dsSrc};
	vkFreeDescriptorSets	(dev->vkDev, ctx->descriptorPool, 1, &ctx->dsSrc);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
															  .descriptorPool = ctx->descriptorPool,
															  .descriptorSetCount = 1,
															  .pSetLayouts = &dev->dslSrc };
	VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &ctx->dsSrc));
}*/

void _createDescriptorPool (VkvgContext ctx) {
	VkvgDevice dev = ctx->dev;
	const VkDescriptorPoolSize descriptorPoolSize[] = {
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 },
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 }
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
															.maxSets = 3,
															.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
															.poolSizeCount = 2,
															.pPoolSizes = descriptorPoolSize };
	VK_CHECK_RESULT(vkCreateDescriptorPool (dev->vkDev, &descriptorPoolCreateInfo, NULL, &ctx->descriptorPool));
}
void _init_descriptor_sets (VkvgContext ctx){
	VkvgDevice dev = ctx->dev;
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
															  .descriptorPool = ctx->descriptorPool,
															  .descriptorSetCount = 1,
															  .pSetLayouts = &dev->dslFont
															};
	VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &ctx->dsFont));
	descriptorSetAllocateInfo.pSetLayouts = &dev->dslSrc;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &ctx->dsSrc));
	descriptorSetAllocateInfo.pSetLayouts = &dev->dslGrad;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &ctx->dsGrad));
}
void _release_context_ressources (VkvgContext ctx) {
	VkDevice dev = ctx->dev->vkDev;
	
#ifndef VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
	vkDestroyFence (dev, ctx->flushFence, NULL);
#endif

	vkFreeCommandBuffers(dev, ctx->cmdPool, 2, ctx->cmdBuffers);
	vkDestroyCommandPool(dev, ctx->cmdPool, NULL);

	VkDescriptorSet dss[] = {ctx->dsFont, ctx->dsSrc, ctx->dsGrad};
	vkFreeDescriptorSets	(dev, ctx->descriptorPool, 3, dss);

	vkDestroyDescriptorPool (dev, ctx->descriptorPool,NULL);

	vkvg_buffer_destroy (&ctx->uboGrad);
	vkvg_buffer_destroy (&ctx->indices);
	vkvg_buffer_destroy (&ctx->vertices);

	free(ctx->vertexCache);
	free(ctx->indexCache);

	vkh_image_destroy	  (ctx->fontCacheImg);
	//TODO:check this for source counter
	//vkh_image_destroy	  (ctx->source);

	free(ctx->pathes);
	free(ctx->points);

	free(ctx);
}
//populate vertice buff for stroke
bool _build_vb_step(VkvgContext ctx, stroke_context_t* str, bool isCurve){
	Vertex v = {{0},ctx->curColor, {0,0,-1}};
	vec2 pL = ctx->points[str->iL];
	vec2 p0 = ctx->points[str->cp];
	vec2 pR = ctx->points[str->iR];

	vec2 v0 = vec2_sub(p0, pL);
	vec2 v1 = vec2_sub(pR, p0);
	float length_v0 = vec2_length(v0);
	float length_v1 = vec2_length(v1);
	if (length_v0 < FLT_EPSILON || length_v1 < FLT_EPSILON) {
		LOG(VKVG_LOG_STROKE, "vb_step discard, length<epsilon: l0:%f l1:%f\n", length_v0, length_v1);
		return false;
	}
	vec2 v0n = vec2_div_s (v0, length_v0);
	vec2 v1n = vec2_div_s (v1, length_v1);
	float dot = vec2_dot (v0n, v1n);
	float det = v0n.x * v1n.y - v0n.y * v1n.x;
	if (EQUF(dot,1.0f)) {//colinear
		LOG(VKVG_LOG_STROKE, "vb_step discard, dot==1\n");
		return false;
	}

	if (EQUF(dot,-1.0f)) {//cusp (could draw line butt?)
		vec2 vPerp = vec2_mult_s(vec2_perp (v0n), str->hw);

		VKVG_IBO_INDEX_TYPE idx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);

		v.pos = vec2_add(p0, vPerp);
		_add_vertex(ctx, v);
		v.pos = vec2_sub(p0, vPerp);
		_add_vertex(ctx, v);

		_add_triangle_indices(ctx, idx, idx+1, idx+2);
		_add_triangle_indices(ctx, idx, idx+2, idx+3);
		LOG(VKVG_LOG_STROKE, "vb_step cusp, dot==-1\n");
		return true;
	}


	vec2 bisec_n = vec2_norm(vec2_add(v0n,v1n));//bisec/bisec_perp are inverted names


	float alpha = acosf(dot);


	if (det<0)
		alpha = -alpha;

	float halfAlpha = alpha / 2.f;

	float lh = str->hw / cosf(halfAlpha);
	vec2 bisec_n_perp = vec2_perp(bisec_n);

	//limit bisectrice length
	float rlh = lh;//rlh is for inside pos tweeks
	if (dot < 0.f)
		rlh = fminf (rlh, fminf (length_v0, length_v1));
	//---

	vec2 bisec = vec2_mult_s (bisec_n_perp, rlh);

	VKVG_IBO_INDEX_TYPE idx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);

	vec2 rlh_inside_pos, rlh_outside_pos;
	if (rlh < lh) {
		vec2 vnPerp;
		if (length_v0 < length_v1)
			vnPerp = vec2_perp (v1n);
		else
			vnPerp = vec2_perp (v0n);
		vec2 vHwPerp = vec2_mult_s (vnPerp, str->hw);

		double lbc = cosf(halfAlpha) * rlh;
		if (det < 0.f) {
			rlh_inside_pos	= vec2_add (vec2_add (vec2_mult_s(vnPerp, -lbc), vec2_add(p0, bisec)), vHwPerp);
			rlh_outside_pos	= vec2_sub (p0, vec2_mult_s (bisec_n_perp, lh));
		} else {
			rlh_inside_pos	= vec2_sub (vec2_add (vec2_mult_s(vnPerp, lbc), vec2_sub(p0, bisec)), vHwPerp);
			rlh_outside_pos	= vec2_add (p0, vec2_mult_s (bisec_n_perp, lh));
		}
	} else {
		if (det < 0.0) {
			rlh_inside_pos	= vec2_add (p0, bisec);
			rlh_outside_pos	= vec2_sub (p0, bisec);
		} else {
			rlh_inside_pos	= vec2_sub (p0, bisec);
			rlh_outside_pos	= vec2_add (p0, bisec);
		}
	}

	vkvg_line_join_t join = ctx->lineJoin;

	if (isCurve) {
		if (dot < 0.8f)
			join = VKVG_LINE_JOIN_ROUND;
		else
			join = VKVG_LINE_JOIN_MITER;
	}

	if (join == VKVG_LINE_JOIN_MITER){
		if (lh > str->lhMax) {//miter limit
			double x = (lh - str->lhMax) * cosf (halfAlpha);
			vec2 bisecPerp = vec2_mult_s (bisec_n, x);
			bisec = vec2_mult_s (bisec_n_perp, str->lhMax);
			if (det < 0) {
				v.pos = rlh_inside_pos;
				_add_vertex(ctx, v);

				vec2 p = vec2_sub(p0, bisec);

				v.pos = vec2_sub(p, bisecPerp);
				_add_vertex(ctx, v);
				v.pos = vec2_add(p, bisecPerp);
				_add_vertex(ctx, v);

				_add_triangle_indices(ctx, idx, idx+2, idx+1);
				_add_triangle_indices(ctx, idx+2, idx+4, idx);
				_add_triangle_indices(ctx, idx, idx+3, idx+4);
				return true;
			} else {
				vec2 p = vec2_add(p0, bisec);
				v.pos = vec2_sub(p, bisecPerp);
				_add_vertex(ctx, v);

				v.pos = rlh_inside_pos;
				_add_vertex(ctx, v);

				v.pos = vec2_add(p, bisecPerp);
				_add_vertex(ctx, v);

				_add_triangle_indices(ctx, idx, idx+2, idx+1);
				_add_triangle_indices(ctx, idx+2, idx+3, idx+1);
				_add_triangle_indices(ctx, idx+1, idx+3, idx+4);
				return false;
			}

		} else {//normal miter
			if (det < 0) {
				v.pos = rlh_inside_pos;
				_add_vertex(ctx, v);
				v.pos = rlh_outside_pos;
				_add_vertex(ctx, v);
			} else {
				v.pos = rlh_outside_pos;
				_add_vertex(ctx, v);
				v.pos = rlh_inside_pos;
				_add_vertex(ctx, v);
			}

			_add_tri_indices_for_rect(ctx, idx);
			return false;
		}
	}else{
		vec2 vp = vec2_perp(v0n);

		if (det<0){
			if (dot < 0 && rlh < lh)
				v.pos = rlh_inside_pos;
			else
				v.pos = vec2_add (p0, bisec);
			_add_vertex(ctx, v);
			v.pos = vec2_sub (p0, vec2_mult_s (vp, str->hw));
		}else{
			v.pos = vec2_add (p0, vec2_mult_s (vp, str->hw));
			_add_vertex(ctx, v);
			if (dot < 0 && rlh < lh)
				v.pos = rlh_inside_pos;
			else
				v.pos = vec2_sub (p0, bisec);
		}
		_add_vertex(ctx, v);

		if (join == VKVG_LINE_JOIN_BEVEL){
			if (det<0){
				_add_triangle_indices(ctx, idx, idx+2, idx+1);
				_add_triangle_indices(ctx, idx+2, idx+4, idx+0);
				_add_triangle_indices(ctx, idx, idx+3, idx+4);
			}else{
				_add_triangle_indices(ctx, idx, idx+2, idx+1);
				_add_triangle_indices(ctx, idx+2, idx+3, idx+1);
				_add_triangle_indices(ctx, idx+1, idx+3, idx+4);
			}
		}else if (join == VKVG_LINE_JOIN_ROUND){
			if (!str->arcStep)
				str->arcStep = _get_arc_step (ctx, str->hw);
			float a = acosf(vp.x);
			if (vp.y < 0)
				a = -a;

			if (det<0){
				a+=M_PIF;
				float a1 = a + alpha;
				a-=str->arcStep;
				while (a > a1){
					_add_vertexf(ctx, cosf(a) * str->hw + p0.x, sinf(a) * str->hw + p0.y);
					a-=str->arcStep;
				}
			}else{
				float a1 = a + alpha;
				a+=str->arcStep;
				while (a < a1){
					_add_vertexf(ctx, cosf(a) * str->hw + p0.x, sinf(a) * str->hw + p0.y);
					a+=str->arcStep;
				}
			}
			VKVG_IBO_INDEX_TYPE p0Idx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);
			_add_triangle_indices(ctx, idx, idx+2, idx+1);
			if (det < 0){
				for (VKVG_IBO_INDEX_TYPE p = idx+2; p < p0Idx; p++)
					_add_triangle_indices(ctx, p, p+1, idx);
				_add_triangle_indices(ctx, p0Idx, p0Idx+2, idx);
				_add_triangle_indices(ctx, idx, p0Idx+1, p0Idx+2);
			}else{
				for (VKVG_IBO_INDEX_TYPE p = idx+2; p < p0Idx; p++)
					_add_triangle_indices(ctx, p, p+1, idx+1);
				_add_triangle_indices(ctx, p0Idx, p0Idx+1, idx+1);
				_add_triangle_indices(ctx, idx+1, p0Idx+1, p0Idx+2);
			}

		}

		vp = vec2_mult_s (vec2_perp(v1n), str->hw);
		if (det < 0)
			v.pos = vec2_sub (p0, vp);
		else
			v.pos = vec2_add (p0, vp);
		_add_vertex(ctx, v);
	}

/*
#ifdef DEBUG

	debugLinePoints[dlpCount] = p0;
	debugLinePoints[dlpCount+1] = _v2add(p0, _vec2dToVec2(_v2Multd(v0n,10)));
	dlpCount+=2;
	debugLinePoints[dlpCount] = p0;
	debugLinePoints[dlpCount+1] = _v2add(p0, _vec2dToVec2(_v2Multd(v1n,10)));
	dlpCount+=2;
	debugLinePoints[dlpCount] = p0;
	debugLinePoints[dlpCount+1] = pR;
	dlpCount+=2;
#endif*/
	/*if (reducedLH)
		return -det;
	else*/
	return (det < 0);
}

void _draw_stoke_cap (VkvgContext ctx, stroke_context_t *str, vec2 p0, vec2 n, bool isStart) {
	Vertex v = {{0},ctx->curColor,{0,0,-1}};

	VKVG_IBO_INDEX_TYPE firstIdx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);

	if (isStart){
		vec2 vhw = vec2_mult_s (n, str->hw);

		if (ctx->lineCap == VKVG_LINE_CAP_SQUARE)
			p0 = vec2_sub (p0, vhw);

		vhw = vec2_perp (vhw);

		if (ctx->lineCap == VKVG_LINE_CAP_ROUND){
			if (!str->arcStep)
				str->arcStep = _get_arc_step (ctx, str->hw);

			float a = acosf(n.x) + M_PIF_2;
			if (n.y < 0)
				a = M_PIF-a;
			float a1 = a + M_PIF;

			a += str->arcStep;
			while (a < a1){
				_add_vertexf (ctx, cosf(a) * str->hw + p0.x, sinf(a) * str->hw + p0.y);
				a += str->arcStep;
			}
			VKVG_IBO_INDEX_TYPE p0Idx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);
			for (VKVG_IBO_INDEX_TYPE p = firstIdx; p < p0Idx; p++)
				_add_triangle_indices (ctx, p0Idx+1, p, p+1);
			firstIdx = p0Idx;
		}

		v.pos = vec2_add (p0, vhw);
		_add_vertex (ctx, v);
		v.pos = vec2_sub (p0, vhw);
		_add_vertex (ctx, v);

		_add_tri_indices_for_rect (ctx, firstIdx);
	}else{
		vec2 vhw = vec2_mult_s (n, str->hw);

		if (ctx->lineCap == VKVG_LINE_CAP_SQUARE)
			p0 = vec2_add (p0, vhw);

		vhw = vec2_perp (vhw);

		v.pos = vec2_add (p0, vhw);
		_add_vertex (ctx, v);
		v.pos = vec2_sub (p0, vhw);
		_add_vertex (ctx, v);

		firstIdx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);

		if (ctx->lineCap == VKVG_LINE_CAP_ROUND){
			if (!str->arcStep)
				str->arcStep = _get_arc_step (ctx, str->hw);

			float a = acosf(n.x)+ M_PIF_2;
			if (n.y < 0)
				a = M_PIF-a;
			float a1 = a - M_PIF;

			a -= str->arcStep;
			while ( a > a1){
				_add_vertexf (ctx, cosf(a) * str->hw + p0.x, sinf(a) * str->hw + p0.y);
				a -= str->arcStep;
			}

			VKVG_IBO_INDEX_TYPE p0Idx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset - 1);
			for (VKVG_IBO_INDEX_TYPE p = firstIdx-1 ; p < p0Idx; p++)
				_add_triangle_indices (ctx, p+1, p, firstIdx-2);
		}
	}
}
float _draw_dashed_segment (VkvgContext ctx, stroke_context_t* str, dash_context_t* dc, bool isCurve) {
	//vec2 pL = ctx->points[str->iL];
	vec2 p = ctx->points[str->cp];
	vec2 pR = ctx->points[str->iR];

	if (!dc->dashOn)//we test in fact the next dash start, if dashOn = true => next segment is a void.
		_build_vb_step (ctx, str, isCurve);

	vec2 d = vec2_sub (pR, p);
	dc->normal = vec2_norm (d);
	float segmentLength = vec2_length(d);

	while (dc->curDashOffset < segmentLength){
		vec2 p0 = vec2_add (p, vec2_mult_s(dc->normal, dc->curDashOffset));

		_draw_stoke_cap (ctx, str, p0, dc->normal, dc->dashOn);
		dc->dashOn ^= true;
		dc->curDashOffset += ctx->dashes[dc->curDash];
		if (++dc->curDash == ctx->dashCount)
			dc->curDash = 0;
	}
	dc->curDashOffset -= segmentLength;
	dc->curDashOffset = fmodf(dc->curDashOffset, dc->totDashLength);
	return segmentLength;
}
void _draw_segment (VkvgContext ctx, stroke_context_t* str, dash_context_t* dc, bool isCurve) {
	str->iR = str->cp + 1;
	if (ctx->dashCount > 0)
		_draw_dashed_segment(ctx, str, dc, isCurve);
	else
		_build_vb_step (ctx, str, isCurve);
	str->iL = str->cp++;
	if (ctx->vertCount - ctx->curVertOffset > VKVG_IBO_MAX / 3) {
		Vertex v0 = ctx->vertexCache[ctx->curVertOffset + str->firstIdx];
		Vertex v1 = ctx->vertexCache[ctx->curVertOffset + str->firstIdx + 1];
		_emit_draw_cmd_undrawn_vertices(ctx);
		//repeat first 2 vertices for closed pathes
		str->firstIdx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);
		_add_vertex(ctx, v0);
		_add_vertex(ctx, v1);
		ctx->curVertOffset = ctx->vertCount;//prevent redrawing them at the start of the batch
	}
}

bool ptInTriangle(vec2 p, vec2 p0, vec2 p1, vec2 p2) {
	float dX = p.x-p2.x;
	float dY = p.y-p2.y;
	float dX21 = p2.x-p1.x;
	float dY12 = p1.y-p2.y;
	float D = dY12*(p0.x-p2.x) + dX21*(p0.y-p2.y);
	float s = dY12*dX + dX21*dY;
	float t = (p2.y-p0.y)*dX + (p0.x-p2.x)*dY;
	if (D<0)
		return (s<=0) && (t<=0) && (s+t>=D);
	return (s>=0) && (t>=0) && (s+t<=D);
}

void _free_ctx_save (vkvg_context_save_t* sav){
	if (sav->dashCount > 0)
		free (sav->dashes);
	free (sav);
}


#define M_APPROXIMATION_SCALE	1.0
#define M_ANGLE_TOLERANCE		0.01
#define M_CUSP_LIMIT			0.01
#define CURVE_RECURSION_LIMIT	100
#define CURVE_COLLINEARITY_EPSILON 1.7
#define CURVE_ANGLE_TOLERANCE_EPSILON 0.001
//no floating point arithmetic operation allowed in macro.
#pragma warning(disable:4127)
void _recursive_bezier (VkvgContext ctx, float distanceTolerance,
						float x1, float y1, float x2, float y2,
						float x3, float y3, float x4, float y4,
						unsigned level) {
	if(level > CURVE_RECURSION_LIMIT)
	{
		return;
	}

	// Calculate all the mid-points of the line segments
	//----------------------
	float x12	= (x1 + x2) / 2;
	float y12	= (y1 + y2) / 2;
	float x23	= (x2 + x3) / 2;
	float y23	= (y2 + y3) / 2;
	float x34	= (x3 + x4) / 2;
	float y34	= (y3 + y4) / 2;
	float x123	= (x12 + x23) / 2;
	float y123	= (y12 + y23) / 2;
	float x234	= (x23 + x34) / 2;
	float y234	= (y23 + y34) / 2;
	float x1234 = (x123 + x234) / 2;
	float y1234 = (y123 + y234) / 2;

	if(level > 0) // Enforce subdivision first time
	{
		// Try to approximate the full cubic curve by a single straight line
		//------------------
		float dx = x4-x1;
		float dy = y4-y1;

		float d2 = fabsf(((x2 - x4) * dy - (y2 - y4) * dx));
		float d3 = fabsf(((x3 - x4) * dy - (y3 - y4) * dx));

		float da1, da2;

		if(d2 > CURVE_COLLINEARITY_EPSILON && d3 > CURVE_COLLINEARITY_EPSILON)
		{
			// Regular care
			//-----------------
			if((d2 + d3)*(d2 + d3) <= (dx*dx + dy*dy) * distanceTolerance)
			{
				// If the curvature doesn't exceed the distance_tolerance value
				// we tend to finish subdivisions.
				//----------------------
				if (M_ANGLE_TOLERANCE < CURVE_ANGLE_TOLERANCE_EPSILON) {
					_add_point(ctx, x1234, y1234);
					return;
				}

				// Angle & Cusp Condition
				//----------------------
				float a23 = atan2f(y3 - y2, x3 - x2);
				da1 = fabsf(a23 - atan2f(y2 - y1, x2 - x1));
				da2 = fabsf(atan2f(y4 - y3, x4 - x3) - a23);
				if(da1 >= M_PIF) da1 = M_2_PIF - da1;
				if(da2 >= M_PIF) da2 = M_2_PIF - da2;

				if(da1 + da2 < (float)M_ANGLE_TOLERANCE)
				{
					// Finally we can stop the recursion
					//----------------------
					_add_point (ctx, x1234, y1234);
					return;
				}

				if(M_CUSP_LIMIT != 0.0)
				{
					if(da1 > M_CUSP_LIMIT)
					{
						_add_point (ctx, x2, y2);
						return;
					}

					if(da2 > M_CUSP_LIMIT)
					{
						_add_point (ctx, x3, y3);
						return;
					}
				}
			}
		} else {
			if(d2 > CURVE_COLLINEARITY_EPSILON)
			{
				// p1,p3,p4 are collinear, p2 is considerable
				//----------------------
				if(d2 * d2 <= distanceTolerance * (dx*dx + dy*dy))
				{
					if(M_ANGLE_TOLERANCE < CURVE_ANGLE_TOLERANCE_EPSILON)
					{
						_add_point (ctx, x1234, y1234);
						return;
					}

					// Angle Condition
					//----------------------
					da1 = fabsf(atan2f(y3 - y2, x3 - x2) - atan2f(y2 - y1, x2 - x1));
					if(da1 >= M_PIF) da1 = M_2_PIF - da1;

					if(da1 < M_ANGLE_TOLERANCE)
					{
						_add_point (ctx, x2, y2);
						_add_point (ctx, x3, y3);
						return;
					}

					if(M_CUSP_LIMIT != 0.0)
					{
						if(da1 > M_CUSP_LIMIT)
						{
							_add_point (ctx, x2, y2);
							return;
						}
					}
				}
			} else if(d3 > CURVE_COLLINEARITY_EPSILON) {
				// p1,p2,p4 are collinear, p3 is considerable
				//----------------------
				if(d3 * d3 <= distanceTolerance* (dx*dx + dy*dy))
				{
					if(M_ANGLE_TOLERANCE < CURVE_ANGLE_TOLERANCE_EPSILON)
					{
						_add_point (ctx, x1234, y1234);
						return;
					}

					// Angle Condition
					//----------------------
					da1 = fabsf(atan2f(y4 - y3, x4 - x3) - atan2f(y3 - y2, x3 - x2));
					if(da1 >= M_PIF) da1 = M_2_PIF - da1;

					if(da1 < M_ANGLE_TOLERANCE)
					{
						_add_point (ctx, x2, y2);
						_add_point (ctx, x3, y3);
						return;
					}

					if(M_CUSP_LIMIT != 0.0)
					{
						if(da1 > M_CUSP_LIMIT)
						{
							_add_point (ctx, x3, y3);
							return;
						}
					}
				}
			}
			else
			{
				// Collinear case
				//-----------------
				dx = x1234 - (x1 + x4) / 2;
				dy = y1234 - (y1 + y4) / 2;
				if(dx*dx + dy*dy <= distanceTolerance)
				{
					_add_point (ctx, x1234, y1234);
					return;
				}
			}
		}
	}

	// Continue subdivision
	//----------------------
	_recursive_bezier (ctx, distanceTolerance, x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
	_recursive_bezier (ctx, distanceTolerance,x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);
}
#pragma warning(default:4127)
void _line_to (VkvgContext ctx, float x, float y) {
	vec2 p = {x,y};
	if (!_current_path_is_empty (ctx)){
		//prevent adding the same point
		if (vec2_equ (_get_current_position (ctx), p))
			return;
	}
	_add_point (ctx, x, y);
	ctx->simpleConvex = false;
}
void _elliptic_arc (VkvgContext ctx, float x1, float y1, float x2, float y2, bool largeArc, bool counterClockWise, float _rx, float _ry, float phi) {
	if (ctx->status)
		return;

	if (_rx==0||_ry==0) {
		if (_current_path_is_empty(ctx))
			vkvg_move_to(ctx, x1, y1);
		vkvg_line_to(ctx, x2, y2);
		return;
	}
	float rx = fabsf(_rx);
	float ry = fabsf(_ry);

	mat2 m = {
		{ cosf (phi), sinf (phi)},
		{-sinf (phi), cosf (phi)}
	};
	vec2 p = {(x1 - x2)/2, (y1 - y2)/2};
	vec2 p1 = mat2_mult_vec2 (m, p);

	//radii corrections
	double lambda = powf (p1.x, 2) / powf (rx, 2) + powf (p1.y, 2) / powf (ry, 2);
	if (lambda > 1) {
		lambda = sqrtf (lambda);
		rx *= lambda;
		ry *= lambda;
	}

	p = (vec2){rx * p1.y / ry, -ry * p1.x / rx};

	vec2 cp = vec2_mult_s (p, sqrtf (fabsf (
		(powf (rx,2) * powf (ry,2) - powf (rx,2) * powf (p1.y, 2) - powf (ry,2) * powf (p1.x, 2)) /
		(powf (rx,2) * powf (p1.y, 2) + powf (ry,2) * powf (p1.x, 2))
	)));

	if (largeArc == counterClockWise)
		vec2_inv(&cp);

	m = (mat2) {
		{cosf (phi),-sinf (phi)},
		{sinf (phi), cosf (phi)}
	};
	p = (vec2){(x1 + x2)/2, (y1 + y2)/2};
	vec2 c = vec2_add (mat2_mult_vec2(m, cp) , p);

	vec2 u = vec2_unit_x;
	vec2 v = {(p1.x-cp.x)/rx, (p1.y-cp.y)/ry};
	double sa = acosf (vec2_dot (u, v) / (fabsf(vec2_length(v)) * fabsf(vec2_length(u))));
	if (isnan(sa))
		sa=M_PIF;
	if (u.x*v.y-u.y*v.x < 0)
		sa = -sa;

	u = v;
	v = (vec2) {(-p1.x-cp.x)/rx, (-p1.y-cp.y)/ry};
	double delta_theta = acosf (vec2_dot (u, v) / (fabsf(vec2_length (v)) * fabsf(vec2_length (u))));
	if (isnan(delta_theta))
		delta_theta=M_PIF;
	if (u.x*v.y-u.y*v.x < 0)
		delta_theta = -delta_theta;

	if (counterClockWise) {
		if (delta_theta < 0)
			delta_theta += M_PIF * 2.0;
	} else if (delta_theta > 0)
		delta_theta -= M_PIF * 2.0;

	m = (mat2) {
		{cosf (phi),-sinf (phi)},
		{sinf (phi), cosf (phi)}
	};

	double theta = sa;
	double ea = sa + delta_theta;

	float step = fmaxf(0.001f, fminf(M_PI, _get_arc_step(ctx, fminf (rx, ry))*0.1f));

	p = (vec2) {
		rx * cosf (theta),
		ry * sinf (theta)
	};
	vec2 xy = vec2_add (mat2_mult_vec2 (m, p), c);

	if (_current_path_is_empty(ctx)){
		_set_curve_start (ctx);
		_add_point (ctx, xy.x, xy.y);
		if (!ctx->pathPtr)
			ctx->simpleConvex = true;
		else
			ctx->simpleConvex = false;
	}else{
		_line_to(ctx, xy.x, xy.y);
		_set_curve_start (ctx);
		ctx->simpleConvex = false;
	}

	_set_curve_start (ctx);

	if (sa < ea) {
		theta += step;
		while (theta < ea) {
			p = (vec2) {
				rx * cosf (theta),
				ry * sinf (theta)
			};
			xy = vec2_add (mat2_mult_vec2 (m, p), c);
			_add_point (ctx, xy.x, xy.y);
			theta += step;
		}
	} else {
		theta -= step;
		while (theta > ea) {
			p = (vec2) {
				rx * cosf (theta),
				ry * sinf (theta)
			};
			xy = vec2_add (mat2_mult_vec2 (m, p), c);
			_add_point (ctx, xy.x, xy.y);
			theta -= step;
		}
	}
	p = (vec2) {
		rx * cosf (ea),
		ry * sinf (ea)
	};
	xy = vec2_add (mat2_mult_vec2 (m, p), c);
	_add_point (ctx, xy.x, xy.y);
	_set_curve_end(ctx);
}


//Even-Odd inside test with stencil buffer implementation.
void _poly_fill (VkvgContext ctx, vec4* bounds){
	//we anticipate the check for vbo buffer size, ibo is not used in poly_fill
	//the polyfill emit a single vertex for each point in the path.
	if (ctx->sizeVBO - VKVG_ARRAY_THRESHOLD <  ctx->vertCount + ctx->pointCount) {
		if (ctx->cmdStarted) {
			_end_render_pass (ctx);
			if (ctx->vertCount > 0)
				_flush_vertices_caches (ctx);
			vkh_cmd_end (ctx->cmd);
			_wait_and_submit_cmd (ctx);
			_wait_ctx_flush_end (ctx);
			if (ctx->sizeVBO - VKVG_ARRAY_THRESHOLD < ctx->pointCount){
				_resize_vbo (ctx, ctx->pointCount + VKVG_ARRAY_THRESHOLD);
				_resize_vertex_cache (ctx, ctx->sizeVBO);
			}
		}else{
			_resize_vbo (ctx, ctx->vertCount + ctx->pointCount + VKVG_ARRAY_THRESHOLD);
			_resize_vertex_cache (ctx, ctx->sizeVBO);
		}

		_start_cmd_for_render_pass(ctx);
	} else {
		_ensure_vertex_cache_size (ctx, ctx->pointCount);
		_ensure_renderpass_is_started (ctx);
	}

	CmdBindPipeline (ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->dev->pipelinePolyFill);

	Vertex v = {{0}, ctx->curColor, {0,0,-1}};
	uint32_t ptrPath = 0;
	uint32_t firstPtIdx = 0;

	while (ptrPath < ctx->pathPtr){
		uint32_t pathPointCount = ctx->pathes[ptrPath] & PATH_ELT_MASK;
		if (pathPointCount > 2) {
			VKVG_IBO_INDEX_TYPE firstVertIdx = (VKVG_IBO_INDEX_TYPE)ctx->vertCount;

			for (uint32_t i = 0; i < pathPointCount; i++) {
				v.pos = ctx->points [i+firstPtIdx];
				ctx->vertexCache[ctx->vertCount++] = v;
				if (!bounds)
					continue;
				//bounds are computed here to scissor the painting operation
				//that speed up fill drastically.
				vkvg_matrix_transform_point (&ctx->pushConsts.mat, &v.pos.x, &v.pos.y);

				if (v.pos.x < bounds->xMin)
					bounds->xMin = v.pos.x;
				if (v.pos.x > bounds->xMax)
					bounds->xMax = v.pos.x;
				if (v.pos.y < bounds->yMin)
					bounds->yMin = v.pos.y;
				if (v.pos.y > bounds->yMax)
					bounds->yMax = v.pos.y;
			}

			LOG(VKVG_LOG_INFO_PATH, "\tpoly fill: point count = %d; 1st vert = %d; vert count = %d\n", pathPointCount, firstVertIdx, ctx->vertCount - firstVertIdx);
			CmdDraw (ctx->cmd, pathPointCount, 1, firstVertIdx , 0);
		}
		firstPtIdx += pathPointCount;

		if (_path_has_curves (ctx, ptrPath)) {
			//skip segments lengths used in stroke
			ptrPath++;
			uint32_t totPts = 0;
			while (totPts < pathPointCount)
				totPts += (ctx->pathes[ptrPath++] & PATH_ELT_MASK);
		}else
			ptrPath++;
	}
	ctx->curVertOffset = ctx->vertCount;
}
#ifdef VKVG_FILL_NZ_GLUTESS
void fan_vertex2(VKVG_IBO_INDEX_TYPE v, VkvgContext ctx) {
	VKVG_IBO_INDEX_TYPE i = (VKVG_IBO_INDEX_TYPE)v;
	switch (ctx->tesselator_idx_counter) {
	case 0:
		_add_indice(ctx, i);
		ctx->tesselator_fan_start = i;
		ctx->tesselator_idx_counter ++;
		break;
	case 1:
	case 2:
		_add_indice(ctx, i);
		ctx->tesselator_idx_counter ++;
		break;
	default:
		_add_indice_for_fan (ctx, i);
		break;
	}
}

void strip_vertex2(VKVG_IBO_INDEX_TYPE v, VkvgContext ctx) {
	VKVG_IBO_INDEX_TYPE i = (VKVG_IBO_INDEX_TYPE)v;
	if (ctx->tesselator_idx_counter < 3) {
		_add_indice(ctx, i);
	} else
		_add_indice_for_strip(ctx, i, ctx->tesselator_idx_counter % 2);
	ctx->tesselator_idx_counter ++;
}

void triangle_vertex2 (VKVG_IBO_INDEX_TYPE v, VkvgContext ctx) {
	VKVG_IBO_INDEX_TYPE i = (VKVG_IBO_INDEX_TYPE)v;
	_add_indice(ctx, i);
}
void skip_vertex2 (VKVG_IBO_INDEX_TYPE v, VkvgContext ctx) {}
void begin2(GLenum which, void *poly_data)
{
	VkvgContext ctx = (VkvgContext)poly_data;
	switch (which) {
	case GL_TRIANGLES:
		ctx->vertex_cb = &triangle_vertex2;
		break;
	case GL_TRIANGLE_STRIP:
		ctx->tesselator_idx_counter = 0;
		ctx->vertex_cb = &strip_vertex2;
		break;
	case GL_TRIANGLE_FAN:
		ctx->tesselator_idx_counter = ctx->tesselator_fan_start = 0;
		ctx->vertex_cb = &fan_vertex2;
		break;
	default:
		fprintf(stderr, "ERROR, can't handle %d\n", (int)which);
		ctx->vertex_cb = &skip_vertex2;
	}
}

void combine2(const GLdouble newVertex[3],
			 const void *neighborVertex_s[4],
			 const GLfloat neighborWeight[4], void **outData, void *poly_data)
{
	VkvgContext ctx = (VkvgContext)poly_data;
	Vertex v = {{newVertex[0],newVertex[1]},ctx->curColor, {0,0,-1}};
	*outData = (void*)((unsigned long)(ctx->vertCount - ctx->curVertOffset));
	_add_vertex(ctx, v);
}
void vertex2(void *vertex_data, void *poly_data)
{
	VKVG_IBO_INDEX_TYPE i = (VKVG_IBO_INDEX_TYPE)vertex_data;
	VkvgContext ctx = (VkvgContext)poly_data;
	ctx->vertex_cb(i, ctx);
}
void _fill_non_zero (VkvgContext ctx){
	Vertex v = {{0},ctx->curColor, {0,0,-1}};

	uint32_t ptrPath = 0;
	uint32_t firstPtIdx = 0;

	if (ctx->pathPtr == 1 && ctx->pathes[0] & PATH_IS_CONVEX_BIT) {
		//simple concave rectangle or circle
		VKVG_IBO_INDEX_TYPE firstVertIdx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);
		uint32_t pathPointCount = ctx->pathes[ptrPath] & PATH_ELT_MASK;

		_ensure_vertex_cache_size(ctx, pathPointCount);
		_ensure_index_cache_size(ctx, (pathPointCount-2)*3);

		VKVG_IBO_INDEX_TYPE i = 0;
		while (i < 2){
			v.pos = ctx->points [i++];
			_set_vertex (ctx, ctx->vertCount++, v);
		}
		while (i < pathPointCount){
			v.pos = ctx->points [i];
			_set_vertex (ctx, ctx->vertCount++, v);
			_add_triangle_indices_unchecked(ctx, firstVertIdx, firstVertIdx + i - 1, firstVertIdx + i);
			i++;
		}
		return;
	}


	GLUtesselator *tess = gluNewTess();
	gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
	gluTessCallback(tess, GLU_TESS_VERTEX_DATA,  (GLvoid (*) ()) &vertex2);
	gluTessCallback(tess, GLU_TESS_BEGIN_DATA,   (GLvoid (*) ()) &begin2);
	gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (GLvoid (*) ()) &combine2);

	gluTessBeginPolygon(tess, ctx);

	while (ptrPath < ctx->pathPtr){
		uint32_t pathPointCount = ctx->pathes[ptrPath] & PATH_ELT_MASK;

		if (pathPointCount > 2) {
			VKVG_IBO_INDEX_TYPE firstVertIdx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);
			gluTessBeginContour(tess);

			VKVG_IBO_INDEX_TYPE i = 0;

			while (i < pathPointCount){
				v.pos = ctx->points[i+firstPtIdx];
				double dp[] = {v.pos.x,v.pos.y,0};
				_add_vertex(ctx, v);
				gluTessVertex(tess, dp, (void*)((unsigned long)firstVertIdx + i));
				i++;
			}
			gluTessEndContour(tess);

			//limit batch size here to 1/3 of the ibo index type ability
			//if (ctx->vertCount - ctx->curVertOffset > VKVG_IBO_MAX / 3)
			//	_emit_draw_cmd_undrawn_vertices(ctx);
		}

		firstPtIdx += pathPointCount;
		if (_path_has_curves (ctx, ptrPath)) {
			//skip segments lengths used in stroke
			ptrPath++;
			uint32_t totPts = 0;
			while (totPts < pathPointCount)
				totPts += (ctx->pathes[ptrPath++] & PATH_ELT_MASK);
		}else
			ptrPath++;
	}

	gluTessEndPolygon(tess);

	gluDeleteTess(tess);
}
#else
//create fill from current path with ear clipping technic
void _fill_non_zero (VkvgContext ctx){
	Vertex v = {{0},ctx->curColor, {0,0,-1}};

	uint32_t ptrPath = 0;
	uint32_t firstPtIdx = 0;

	while (ptrPath < ctx->pathPtr){
		uint32_t pathPointCount = ctx->pathes[ptrPath] & PATH_ELT_MASK;

		if (pathPointCount > 2) {
			VKVG_IBO_INDEX_TYPE firstVertIdx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);
			ear_clip_point* ecps = (ear_clip_point*)malloc(pathPointCount*sizeof(ear_clip_point));
			uint32_t ecps_count = pathPointCount;
			VKVG_IBO_INDEX_TYPE i = 0;

			//init points link list
			while (i < pathPointCount-1){
				v.pos = ctx->points[i+firstPtIdx];
				ear_clip_point ecp = {v.pos, firstVertIdx+i, &ecps[i+1]};
				ecps[i] = ecp;
				_add_vertex(ctx, v);
				i++;
			}

			v.pos = ctx->points[i+firstPtIdx];
			ear_clip_point ecp = {v.pos, firstVertIdx+i, ecps};
			ecps[i] = ecp;
			_add_vertex(ctx, v);

			ear_clip_point* ecp_current = ecps;
			uint32_t tries = 0;

			while (ecps_count > 3) {
				if (tries > ecps_count) {
					break;
				}
				ear_clip_point* v0 = ecp_current->next,
						*v1 = ecp_current, *v2 = ecp_current->next->next;
				if (ecp_zcross (v0, v2, v1)<0){
					ecp_current = ecp_current->next;
					tries++;
					continue;
				}
				ear_clip_point* vP = v2->next;
				bool isEar = true;
				while (vP!=v1){
					if (ptInTriangle (vP->pos, v0->pos, v2->pos, v1->pos)){
						isEar = false;
						break;
					}
					vP = vP->next;
				}
				if (isEar){
					_add_triangle_indices (ctx, v0->idx, v1->idx, v2->idx);
					v1->next = v2;
					ecps_count --;
					tries = 0;
				}else{
					ecp_current = ecp_current->next;
					tries++;
				}
			}
			if (ecps_count == 3)
				_add_triangle_indices(ctx, ecp_current->next->idx, ecp_current->idx, ecp_current->next->next->idx);
			free (ecps);

			//limit batch size here to 1/3 of the ibo index type ability
			if (ctx->vertCount - ctx->curVertOffset > VKVG_IBO_MAX / 3)
				_emit_draw_cmd_undrawn_vertices(ctx);
		}

		firstPtIdx += pathPointCount;
		if (_path_has_curves (ctx, ptrPath)) {
			//skip segments lengths used in stroke
			ptrPath++;
			uint32_t totPts = 0;
			while (totPts < pathPointCount)
				totPts += (ctx->pathes[ptrPath++] & PATH_ELT_MASK);
		}else
			ptrPath++;
	}
}
#endif

void _vkvg_path_extents (VkvgContext ctx, bool transformed, float *x1, float *y1, float *x2, float *y2) {
	uint32_t ptrPath = 0;
	uint32_t firstPtIdx = 0;

	float xMin = FLT_MAX, yMin = FLT_MAX;
	float xMax = FLT_MIN, yMax = FLT_MIN;

	while (ptrPath < ctx->pathPtr){
		uint32_t pathPointCount = ctx->pathes[ptrPath] & PATH_ELT_MASK;

		for (uint32_t i = firstPtIdx; i < firstPtIdx + pathPointCount; i++){
			vec2 p = ctx->points[i];
			if (transformed)
				vkvg_matrix_transform_point (&ctx->pushConsts.mat, &p.x, &p.y);
			if (p.x < xMin)
				xMin = p.x;
			if (p.x > xMax)
				xMax = p.x;
			if (p.y < yMin)
				yMin = p.y;
			if (p.y > yMax)
				yMax = p.y;
		}

		firstPtIdx += pathPointCount;
		if (_path_has_curves (ctx, ptrPath)) {
			//skip segments lengths used in stroke
			ptrPath++;
			uint32_t totPts = 0;
			while (totPts < pathPointCount)
				totPts += (ctx->pathes[ptrPath++] & PATH_ELT_MASK);
		}else
			ptrPath++;
	}
	*x1 = xMin;
	*x2 = xMax;
	*y1 = yMin;
	*y2 = yMax;
}

void _draw_full_screen_quad (VkvgContext ctx, vec4* scissor) {
#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_cmd_label_start(ctx->cmd, "_draw_full_screen_quad", DBG_LAB_COLOR_FSQ);
#endif
	if (scissor) {
		VkRect2D r = {
			{(int32_t)MAX(scissor->xMin, 0), (int32_t)MAX(scissor->yMin, 0)},
			{(int32_t)MAX(scissor->xMax - (int32_t)scissor->xMin + 1, 1), (int32_t)MAX(scissor->yMax - (int32_t)scissor->yMin + 1, 1)}
		};
		CmdSetScissor(ctx->cmd, 0, 1, &r);
	}

	uint32_t firstVertIdx = ctx->vertCount;
	_ensure_vertex_cache_size (ctx, 3);

	_add_vertexf_unchecked (ctx, -1, -1);
	_add_vertexf_unchecked (ctx,  3, -1);
	_add_vertexf_unchecked (ctx, -1,  3);

	ctx->curVertOffset = ctx->vertCount;

	ctx->pushConsts.fsq_patternType |= FULLSCREEN_BIT;
	CmdPushConstants(ctx->cmd, ctx->dev->pipelineLayout,
					   VK_SHADER_STAGE_VERTEX_BIT, 24, 4,&ctx->pushConsts.fsq_patternType);
	CmdDraw (ctx->cmd,3,1,firstVertIdx,0);
	ctx->pushConsts.fsq_patternType &= ~FULLSCREEN_BIT;
	CmdPushConstants(ctx->cmd, ctx->dev->pipelineLayout,
					   VK_SHADER_STAGE_VERTEX_BIT, 24, 4,&ctx->pushConsts.fsq_patternType);
	if (scissor)
		CmdSetScissor(ctx->cmd, 0, 1, &ctx->bounds);

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_cmd_label_end (ctx->cmd);
#endif
}

void _select_font_face (VkvgContext ctx, const char* name){
	if (strcmp(ctx->selectedFontName, name) == 0)
		return;
	strcpy (ctx->selectedFontName, name);
	ctx->currentFont = NULL;
	ctx->currentFontSize = NULL;
}

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

#include "vkvg_device_internal.h"
#include "vkvg_context_internal.h"
#include "vkvg_surface_internal.h"
#include "vkvg_pattern.h"
#include "vkh_queue.h"

#ifdef DEBUG
static vec2 debugLinePoints[1000];
static uint32_t dlpCount = 0;
#if defined (VKVG_DBG_UTILS)
const float DBG_LAB_COLOR_SAV[4] = {1,0,1,1};
const float DBG_LAB_COLOR_CLIP[4] = {0,1,1,1};
#endif
#endif


//todo:this could be used to define a default background
static VkClearValue clearValues[3] = {
	{ .color.float32 = {0,0,0,0} },
	{ .depthStencil = {1.0f, 0} },
	{ .color.float32 = {0,0,0,0} }
};

VkvgContext vkvg_create(VkvgSurface surf)
{
	VkvgDevice dev = surf->dev;
	VkvgContext ctx = (vkvg_context*)calloc(1, sizeof(vkvg_context));

	LOG(VKVG_LOG_INFO, "CREATE Context: ctx = %p; surf = %p\n", ctx, surf);

	if (ctx==NULL) {
		dev->status = VKVG_STATUS_NO_MEMORY;
		return NULL;
	}

	ctx->sizePoints		= VKVG_PTS_SIZE;
	ctx->sizeVertices	= ctx->sizeVBO = VKVG_VBO_SIZE;
	ctx->sizeIndices	= ctx->sizeIBO = VKVG_IBO_SIZE;
	ctx->sizePathes		= VKVG_PATHES_SIZE;
	ctx->lineWidth		= 1;
	ctx->curOperator	= VKVG_OPERATOR_OVER;
	ctx->curFillRule	= VKVG_FILL_RULE_NON_ZERO;
	ctx->pSurf			= surf;

	ctx->bounds = (VkRect2D) {{0,0},{ctx->pSurf->width,ctx->pSurf->height}};
	ctx->pushConsts = (push_constants) {
			{.height=1},
			{(float)ctx->pSurf->width,(float)ctx->pSurf->height},
			VKVG_PATTERN_TYPE_SOLID,
			0,
			VKVG_IDENTITY_MATRIX,
			VKVG_IDENTITY_MATRIX
	};
	ctx->clearRect = (VkClearRect) {{{0},{ctx->pSurf->width, ctx->pSurf->height}},0,1};

	ctx->renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	ctx->renderPassBeginInfo.framebuffer = ctx->pSurf->fb;
	ctx->renderPassBeginInfo.renderArea.extent.width = ctx->pSurf->width;
	ctx->renderPassBeginInfo.renderArea.extent.height = ctx->pSurf->height;
	ctx->renderPassBeginInfo.pClearValues = clearValues;

	if (ctx->pSurf->new)
		ctx->renderPassBeginInfo.renderPass = ctx->pSurf->dev->renderPass_ClearAll;
	else
		ctx->renderPassBeginInfo.renderPass = ctx->pSurf->dev->renderPass_ClearStencil;

	ctx->pSurf->new = false;

	if (dev->samples == VK_SAMPLE_COUNT_1_BIT)
		ctx->renderPassBeginInfo.clearValueCount = 2;
	else
		ctx->renderPassBeginInfo.clearValueCount = 3;

	ctx->pPrev = surf->dev->lastCtx;
	if (ctx->pPrev != NULL)
		ctx->pPrev->pNext = ctx;
	surf->dev->lastCtx = ctx;

	ctx->points	= (vec2*)malloc (VKVG_VBO_SIZE*sizeof(vec2));
	ctx->pathes	= (uint32_t*)malloc (VKVG_PATHES_SIZE*sizeof(uint32_t));
	ctx->vertexCache = (Vertex*)malloc(ctx->sizeVertices * sizeof(Vertex));
	ctx->indexCache = (VKVG_IBO_INDEX_TYPE*)malloc(ctx->sizeIndices * sizeof(VKVG_IBO_INDEX_TYPE));
	ctx->savedStencils = malloc(0);

	ctx->selectedFontName = (char*)calloc(FONT_NAME_MAX_SIZE, sizeof(char));
	ctx->selectedCharSize = 10 << 6;
	ctx->currentFont = NULL;

	if (!ctx->points || !ctx->pathes || !ctx->vertexCache || !ctx->indexCache || !ctx->savedStencils || !ctx->selectedFontName) {
		dev->status = VKVG_STATUS_NO_MEMORY;
		if (ctx->points)
			free(ctx->points);
		if (ctx->pathes)
			free(ctx->pathes);
		if (ctx->vertexCache)
			free(ctx->vertexCache);
		if (ctx->indexCache)
			free(ctx->indexCache);
		if (ctx->savedStencils)
			free(ctx->savedStencils);
		if (ctx->selectedFontName)
			free(ctx->selectedFontName);
		return NULL;
	}

	ctx->flushFence = vkh_fence_create_signaled ((VkhDevice)dev);
	//for context to be thread safe, command pool and descriptor pool have to be created in the thread of the context.
	ctx->cmdPool = vkh_cmd_pool_create ((VkhDevice)dev, dev->gQueue->familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	_create_vertices_buff	(ctx);
	_create_gradient_buff	(ctx);
	_create_cmd_buff		(ctx);
	_createDescriptorPool	(ctx);
	_init_descriptor_sets	(ctx);
	_update_descriptor_set	(ctx, ctx->pSurf->dev->fontCache->texture, ctx->dsFont);
	_update_descriptor_set	(ctx, surf->dev->emptyImg, ctx->dsSrc);
	_update_gradient_desc_set(ctx);

	_clear_path				(ctx);

	ctx->cmd = ctx->cmdBuffers[0];//current recording buffer

	ctx->references = 1;
	ctx->status = VKVG_STATUS_SUCCESS;

	LOG(VKVG_LOG_DBG_ARRAYS, "INIT\tctx = %p; pathes:%ju pts:%ju vch:%d vbo:%d ich:%d ibo:%d\n", ctx, (uint64_t)ctx->sizePathes, (uint64_t)ctx->sizePoints, ctx->sizeVertices, ctx->sizeVBO, ctx->sizeIndices, ctx->sizeIBO);

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)ctx->cmdPool, "CTX Cmd Pool");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)ctx->cmdBuffers[0], "CTX Cmd Buff A");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)ctx->cmdBuffers[1], "CTX Cmd Buff B");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_FENCE, (uint64_t)ctx->flushFence, "CTX Flush Fence");

	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64_t)ctx->descriptorPool, "CTX Descriptor Pool");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)ctx->dsSrc, "CTX DescSet SOURCE");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)ctx->dsFont, "CTX DescSet FONT");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)ctx->dsGrad, "CTX DescSet GRADIENT");

	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_BUFFER, (uint64_t)ctx->indices.buffer, "CTX Index Buff");
	vkh_device_set_object_name((VkhDevice)dev, VK_OBJECT_TYPE_BUFFER, (uint64_t)ctx->vertices.buffer, "CTX Vertex Buff");
#endif

	//force run of one renderpass (even empty) to perform clear load op
	//_start_cmd_for_render_pass(ctx);

	return ctx;
}
void vkvg_flush (VkvgContext ctx){
	if (ctx->status)
		return;
	_flush_cmd_buff(ctx);
//	_wait_flush_fence(ctx);
/*
#ifdef DEBUG

	vec4 red = {0,0,1,1};
	vec4 green = {0,1,0,1};
	vec4 white = {1,1,1,1};

	int j = 0;
	while (j < dlpCount) {
		add_line(ctx, debugLinePoints[j], debugLinePoints[j+1],green);
		j+=2;
		add_line(ctx, debugLinePoints[j], debugLinePoints[j+1],red);
		j+=2;
		add_line(ctx, debugLinePoints[j], debugLinePoints[j+1],white);
		j+=2;
	}
	dlpCount = 0;
	CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineLineList);
	CmdDrawIndexed(ctx->cmd, ctx->indCount-ctx->curIndStart, 1, ctx->curIndStart, 0, 1);
	_flush_cmd_buff(ctx);
#endif
*/
}

void vkvg_destroy (VkvgContext ctx)
{
	ctx->references--;
	if (ctx->references > 0)
		return;

	_flush_cmd_buff(ctx);
	_wait_flush_fence(ctx);

	LOG(VKVG_LOG_INFO, "DESTROY Context: ctx = %p; surf = %p\n", ctx, ctx->pSurf);
	LOG(VKVG_LOG_DBG_ARRAYS, "END\tctx = %p; pathes:%d pts:%d vch:%d vbo:%d ich:%d ibo:%d\n", ctx, ctx->sizePathes, ctx->sizePoints, ctx->sizeVertices, ctx->sizeVBO, ctx->sizeIndices, ctx->sizeIBO);

	if (ctx->pattern)
		vkvg_pattern_destroy (ctx->pattern);

	VkDevice dev = ctx->pSurf->dev->vkDev;

#if VKVG_DBG_STATS
	vkvg_debug_stats_t* dbgstats = &ctx->pSurf->dev->debug_stats;
	if (dbgstats->sizePoints < ctx->sizePoints)
		dbgstats->sizePoints = ctx->sizePoints;
	if (dbgstats->sizePathes < ctx->sizePathes)
		dbgstats->sizePathes = ctx->sizePathes;
	if (dbgstats->sizeVertices < ctx->sizeVertices)
		dbgstats->sizeVertices = ctx->sizeVertices;
	if (dbgstats->sizeIndices < ctx->sizeIndices)
		dbgstats->sizeIndices = ctx->sizeIndices;
	if (dbgstats->sizeVBO < ctx->sizeVBO)
		dbgstats->sizeVBO = ctx->sizeVBO;
	if (dbgstats->sizeIBO < ctx->sizeIBO)
		dbgstats->sizeIBO = ctx->sizeIBO;

#endif

	vkDestroyFence		(dev, ctx->flushFence,NULL);
	vkFreeCommandBuffers(dev, ctx->cmdPool, 2, ctx->cmdBuffers);
	vkDestroyCommandPool(dev, ctx->cmdPool, NULL);

	VkDescriptorSet dss[] = {ctx->dsFont,ctx->dsSrc, ctx->dsGrad};
	vkFreeDescriptorSets	(dev, ctx->descriptorPool, 3, dss);

	vkDestroyDescriptorPool (dev, ctx->descriptorPool,NULL);

	vkvg_buffer_destroy (&ctx->uboGrad);
	vkvg_buffer_destroy (&ctx->indices);
	vkvg_buffer_destroy (&ctx->vertices);

	free(ctx->vertexCache);
	free(ctx->indexCache);

	//TODO:check this for source counter
	//vkh_image_destroy	  (ctx->source);

	free(ctx->selectedFontName);
	free(ctx->pathes);
	free(ctx->points);
	if (ctx->dashCount > 0)
		free(ctx->dashes);

	//free saved context stack elmt
	vkvg_context_save_t* next = ctx->pSavedCtxs;
	while (next != NULL) {
		vkvg_context_save_t* cur = next;
		next = cur->pNext;
		_free_ctx_save (cur);
		if (cur->pattern)
			vkvg_pattern_destroy (cur->pattern);
	}
	//free additional stencil use in save/restore process
	uint8_t curSaveStencil = ctx->curSavBit / 6;
	for (int i=curSaveStencil;i>0;i--)
		vkh_image_destroy(ctx->savedStencils[i-1]);

	free(ctx->savedStencils);

	//remove context from double linked list of context in device
	if (ctx->pSurf->dev->lastCtx == ctx){
		ctx->pSurf->dev->lastCtx = ctx->pPrev;
		if (ctx->pPrev != NULL)
			ctx->pPrev->pNext = NULL;
	}else if (ctx->pPrev == NULL){
		//first elmt, and it's not last one so pnext is not null
		ctx->pNext->pPrev = NULL;
	}else{
		ctx->pPrev->pNext = ctx->pNext;
		ctx->pNext->pPrev = ctx->pPrev;
	}

	free(ctx);
}
VkvgContext vkvg_reference (VkvgContext ctx) {
	ctx->references++;
	return ctx;
}
uint32_t vkvg_get_reference_count (VkvgContext ctx) {
	return ctx->references;
}
void vkvg_new_sub_path (VkvgContext ctx){
	if (ctx->status)
		return;
	_finish_path(ctx);
}
void vkvg_new_path (VkvgContext ctx){
	if (ctx->status)
		return;
	_clear_path(ctx);
}
void vkvg_close_path (VkvgContext ctx){
	if (ctx->status)
		return;
	if (ctx->pathes[ctx->pathPtr] & PATH_CLOSED_BIT) //already closed
		return;
	//check if at least 3 points are present
	if (ctx->pathes[ctx->pathPtr] < 3)
		return;

	//prevent closing on the same point
	if (vec2_equ(ctx->points[ctx->pointCount-1],
				 ctx->points[ctx->pointCount - ctx->pathes[ctx->pathPtr]])) {
		if (ctx->pathes[ctx->pathPtr] < 4)//ensure enough points left for closing
			return;
		_remove_last_point(ctx);
	}

	ctx->pathes[ctx->pathPtr] |= PATH_CLOSED_BIT;

	_finish_path(ctx);
}
void vkvg_rel_line_to (VkvgContext ctx, float dx, float dy){
	if (ctx->status)
		return;
	if (_current_path_is_empty(ctx))
		_add_point(ctx, 0, 0);
	vec2 cp = _get_current_position(ctx);
	vkvg_line_to(ctx, cp.x + dx, cp.y + dy);
}
void vkvg_line_to (VkvgContext ctx, float x, float y)
{
	if (ctx->status)
		return;
	vec2 p = {x,y};
	if (!_current_path_is_empty (ctx)){
		//prevent adding the same point
		if (vec2_equ (_get_current_position (ctx), p))
			return;
	}
	_add_point (ctx, x, y);
}
void vkvg_arc (VkvgContext ctx, float xc, float yc, float radius, float a1, float a2){
	if (ctx->status)
		return;
	while (a2 < a1)//positive arc must have a1<a2
		a2 += 2.f*M_PIF;

	if (a2 - a1 > 2.f * M_PIF) //limit arc to 2PI
		a2 = a1 + 2.f * M_PIF;

	vec2 v = {cosf(a1)*radius + xc, sinf(a1)*radius + yc};

	float step = _get_arc_step(ctx, radius);
	float a = a1;

	if (_current_path_is_empty(ctx)){
		_set_curve_start (ctx);
		_add_point (ctx, v.x, v.y);
	}else{
		vkvg_line_to(ctx, v.x, v.y);
		_set_curve_start (ctx);
	}

	a+=step;

	if (EQUF(a2, a1))
		return;

	while(a < a2){
		v.x = cosf(a)*radius + xc;
		v.y = sinf(a)*radius + yc;
		_add_point (ctx, v.x, v.y);
		a+=step;
	}

	if (EQUF(a2-a1,M_PIF*2.f)){//if arc is complete circle, last point is the same as the first one
		_set_curve_end(ctx);
		vkvg_close_path(ctx);
		return;
	}
	a = a2;
	//vec2 lastP = v;
	v.x = cosf(a)*radius + xc;
	v.y = sinf(a)*radius + yc;
	//if (!vec2_equ (v,lastP))//this test should not be required
		_add_point (ctx, v.x, v.y);
	_set_curve_end(ctx);
}
void vkvg_arc_negative (VkvgContext ctx, float xc, float yc, float radius, float a1, float a2) {
	if (ctx->status)
		return;
	while (a2 > a1)
		a2 -= 2.f*M_PIF;
	if (a1 - a2 > a1 + 2.f * M_PIF) //limit arc to 2PI
		a2 = a1 - 2.f * M_PIF;

	vec2 v = {cosf(a1)*radius + xc, sinf(a1)*radius + yc};

	float step = _get_arc_step(ctx, radius);
	float a = a1;

	if (_current_path_is_empty(ctx)){
		_set_curve_start (ctx);
		_add_point (ctx, v.x, v.y);
	}else{
		vkvg_line_to(ctx, v.x, v.y);
		_set_curve_start (ctx);
	}

	a-=step;

	if (EQUF(a2, a1))
		return;

	while(a > a2){
		v.x = cosf(a)*radius + xc;
		v.y = sinf(a)*radius + yc;
		_add_point (ctx,v.x,v.y);
		a-=step;
	}

	if (EQUF(a1-a2,M_PIF*2.f))//if arc is complete circle, last point is the same as the first one
		return;

	a = a2;
	//vec2 lastP = v;
	v.x = cosf(a)*radius + xc;
	v.y = sinf(a)*radius + yc;
	//if (!vec2_equ (v,lastP))
		_add_point (ctx, v.x, v.y);
	_set_curve_end(ctx);
}
void vkvg_rel_move_to (VkvgContext ctx, float x, float y)
{
	if (ctx->status)
		return;
	if (_current_path_is_empty(ctx))
		_add_point(ctx, 0, 0);
	vec2 cp = _get_current_position(ctx);
	vkvg_move_to(ctx, cp.x + x, cp.y + y);
}
void vkvg_move_to (VkvgContext ctx, float x, float y)
{
	if (ctx->status)
		return;
	_finish_path(ctx);
	_add_point (ctx, x, y);
}
void vkvg_get_current_point (VkvgContext ctx, float* x, float* y) {
	if (_current_path_is_empty(ctx)) {
		*x = *y = 0;
		return;
	}
	vec2 cp = _get_current_position(ctx);
	*x = cp.x;
	*y = cp.y;
}
void vkvg_rel_quadratic_to (VkvgContext ctx, float x1, float y1, float x2, float y2) {
	if (ctx->status)
		return;
	vec2 cp = _get_current_position(ctx);
	vkvg_quadratic_to (ctx, cp.x + x1, cp.y + y1, cp.x + x2, cp.y + y2);
}
const double quadraticFact = 2.0/3.0;
void vkvg_quadratic_to (VkvgContext ctx, float x1, float y1, float x2, float y2) {
	if (ctx->status)
		return;

	float x0, y0;
	if (_current_path_is_empty(ctx)) {
		x0 = x1;
		y0 = y1;
	} else
		vkvg_get_current_point (ctx, &x0, &y0);
	vkvg_curve_to (ctx,
					x0 + (x1 - x0) * quadraticFact,
					y0 + (y1 - y0) * quadraticFact,
					x2 + (x1 - x2) * quadraticFact,
					y2 + (y1 - y2) * quadraticFact,
					x2, y2);
}
void vkvg_curve_to (VkvgContext ctx, float x1, float y1, float x2, float y2, float x3, float y3) {
	if (ctx->status)
		return;
	//prevent running _recursive_bezier when all 4 curve points are equal
	if (EQUF(x1,x2) && EQUF(x2,x3) && EQUF(y1,y2) && EQUF(y2,y3)) {
		if (_current_path_is_empty(ctx) || (EQUF(_get_current_position(ctx).x,x1) && EQUF(_get_current_position(ctx).y,y1)))
			return;
	}
	_set_curve_start (ctx);
	if (_current_path_is_empty(ctx))
		_add_point(ctx, x1, y1);

	vec2 cp = _get_current_position(ctx);

	//compute dyn distanceTolerance depending on current transform
	float dx = 1, dy = 1;
	vkvg_matrix_transform_distance (&ctx->pushConsts.mat, &dx, &dy);
	float distanceTolerance = 0.02f / fmaxf(dx,dy);

	_recursive_bezier (ctx, distanceTolerance, cp.x, cp.y, x1, y1, x2, y2, x3, y3, 0);
	/*cp.x = x3;
	cp.y = y3;
	if (!vec2_equ(ctx->points[ctx->pointCount-1],cp))*/
		_add_point(ctx,x3,y3);
	_set_curve_end (ctx);
}
void vkvg_rel_curve_to (VkvgContext ctx, float x1, float y1, float x2, float y2, float x3, float y3) {
	if (ctx->status)
		return;
	vec2 cp = _get_current_position(ctx);
	vkvg_curve_to (ctx, cp.x + x1, cp.y + y1, cp.x + x2, cp.y + y2, cp.x + x3, cp.y + y3);
}
void vkvg_fill_rectangle (VkvgContext ctx, float x, float y, float w, float h){
	if (ctx->status)
		return;
	_vao_add_rectangle (ctx,x,y,w,h);
	//_record_draw_cmd(ctx);
}

void vkvg_rectangle (VkvgContext ctx, float x, float y, float w, float h){
	if (ctx->status)
		return;
	_finish_path (ctx);

	if (w <= 0 || h <= 0) {
		ctx->status = VKVG_STATUS_INVALID_RECT;
		return;
	}

	_add_point (ctx, x, y);
	_add_point (ctx, x + w, y);
	_add_point (ctx, x + w, y + h);
	_add_point (ctx, x, y + h);	

	ctx->pathes[ctx->pathPtr] |= PATH_CLOSED_BIT;

	_finish_path(ctx);
}
static const VkClearAttachment clearStencil		   = {VK_IMAGE_ASPECT_STENCIL_BIT, 1, {{{0}}}};
static const VkClearAttachment clearColorAttach	   = {VK_IMAGE_ASPECT_COLOR_BIT,   0, {{{0}}}};

void vkvg_reset_clip (VkvgContext ctx){
	if (ctx->status)
		return;
	_emit_draw_cmd_undrawn_vertices(ctx);
	if (!ctx->cmdStarted) {
		//if command buffer is not already started and in a renderpass, we use the renderpass
		//with the loadop clear for stencil
		ctx->renderPassBeginInfo.renderPass = ctx->pSurf->dev->renderPass_ClearStencil;
		//force run of one renderpass (even empty) to perform clear load op
		_start_cmd_for_render_pass(ctx);
		return;
	}
	vkCmdClearAttachments(ctx->cmd, 1, &clearStencil, 1, &ctx->clearRect);
}
void vkvg_clear (VkvgContext ctx){
	if (ctx->status)
		return;
	_emit_draw_cmd_undrawn_vertices(ctx);
	if (!ctx->cmdStarted) {
		ctx->renderPassBeginInfo.renderPass = ctx->pSurf->dev->renderPass_ClearAll;
		_start_cmd_for_render_pass(ctx);
		return;
	}
	VkClearAttachment ca[2] = {clearColorAttach, clearStencil};
	vkCmdClearAttachments(ctx->cmd, 2, ca, 1, &ctx->clearRect);
}

void vkvg_clip (VkvgContext ctx){
	vkvg_clip_preserve(ctx);
	_clear_path(ctx);
}
void vkvg_stroke (VkvgContext ctx)
{
	vkvg_stroke_preserve(ctx);
	_clear_path(ctx);
}
void vkvg_fill (VkvgContext ctx){
	vkvg_fill_preserve(ctx);
	_clear_path(ctx);
}
void vkvg_clip_preserve (VkvgContext ctx){
	if (ctx->status)
		return;

	_finish_path(ctx);

	if (!ctx->pathPtr)//nothing to clip
		return;

	_emit_draw_cmd_undrawn_vertices(ctx);

	LOG(VKVG_LOG_INFO, "CLIP: ctx = %p; path cpt = %d;\n", ctx, ctx->pathPtr / 2);

	_ensure_renderpass_is_started(ctx);

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_cmd_label_start(ctx->cmd, "clip", DBG_LAB_COLOR_CLIP);
#endif

	if (ctx->curFillRule == VKVG_FILL_RULE_EVEN_ODD){
		_poly_fill (ctx);
		CmdBindPipeline			(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineClipping);
	}else{
		CmdBindPipeline			(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineClipping);
		CmdSetStencilReference	(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_FILL_BIT);
		CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
		CmdSetStencilWriteMask	(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_FILL_BIT);
		_fill_ec(ctx);
		_emit_draw_cmd_undrawn_vertices(ctx);
	}
	CmdSetStencilReference	(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
	CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_FILL_BIT);
	CmdSetStencilWriteMask	(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_ALL_BIT);

	_draw_full_screen_quad (ctx, false);

	_bind_draw_pipeline (ctx);
	CmdSetStencilCompareMask (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_cmd_label_end (ctx->cmd);
#endif
}
void vkvg_path_extents (VkvgContext ctx, float *x1, float *y1, float *x2, float *y2) {
	if (ctx->status)
		return;

	_finish_path(ctx);

	if (!ctx->pathPtr) {//no path
		*x1 = *x2 = *y1 = *y2 = 0;
		return;
	}

	*x1 = ctx->xMin;
	*x2 = ctx->xMax;
	*y1 = ctx->yMin;
	*y2 = ctx->yMax;
}
void vkvg_fill_preserve (VkvgContext ctx){
	if (ctx->status)
		return;

	_finish_path(ctx);

	if (!ctx->pathPtr)//nothing to fill
		return;

	LOG(VKVG_LOG_INFO, "FILL: ctx = %p; path cpt = %d;\n", ctx, ctx->pathPtr / 2);

	 if (ctx->curFillRule == VKVG_FILL_RULE_EVEN_ODD){
		 _emit_draw_cmd_undrawn_vertices(ctx);

		_poly_fill (ctx);
		_bind_draw_pipeline (ctx);
		CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_FILL_BIT);
		_draw_full_screen_quad (ctx, true);
		CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
		return;
	}

	if (ctx->vertCount - ctx->curVertOffset + ctx->pointCount > VKVG_IBO_MAX)
		_emit_draw_cmd_undrawn_vertices(ctx);//limit draw call to addressable vx with choosen index type

	if (ctx->pattern)//if not solid color, source img or gradient has to be bound
		_ensure_renderpass_is_started(ctx);
	_fill_ec(ctx);
}

void vkvg_stroke_preserve (VkvgContext ctx)
{
	if (ctx->status)
		return;

	_finish_path(ctx);

	if (!ctx->pathPtr)//nothing to stroke
		return;

	LOG(VKVG_LOG_INFO, "STROKE: ctx = %p; path ptr = %d;\n", ctx, ctx->pathPtr);

	stroke_context_t str = {0};
	uint32_t ptrPath = 0;
	float hw = ctx->lineWidth / 2.0f;

	while (ptrPath < ctx->pathPtr){
		uint32_t ptrSegment = 0, lastSegmentPointIdx = 0;
		uint32_t firstPathPointIdx = str.cp;
		uint32_t pathPointCount = ctx->pathes[ptrPath]&PATH_ELT_MASK;
		uint32_t lastPathPointIdx = str.cp + pathPointCount - 1;

		dash_context_t dc = {0};

		if (_path_has_curves (ctx,ptrPath)) {
			ptrSegment = 1;
			lastSegmentPointIdx = str.cp + (ctx->pathes[ptrPath+ptrSegment]&PATH_ELT_MASK)-1;
		}

		str.firstIdx = (VKVG_IBO_INDEX_TYPE)(ctx->vertCount - ctx->curVertOffset);

		//LOG(VKVG_LOG_INFO_PATH, "\tPATH: start=%d end=%d", ctx->pathes[ptrPath]&PATH_ELT_MASK, ctx->pathes[ptrPath+1]&PATH_ELT_MASK);
		LOG(VKVG_LOG_INFO_PATH, "end = %d\n", lastPathPointIdx);

		if (ctx->dashCount > 0) {
			//init dash stroke
			dc.dashOn = true;
			dc.curDash = 0;	//current dash index
			dc.totDashLength = 0;//limit offset to total length of dashes
			for (uint32_t i=0;i<ctx->dashCount;i++)
				dc.totDashLength += ctx->dashes[i];
			if (dc.totDashLength == 0){
				ctx->status = VKVG_STATUS_INVALID_DASH;
				return;
			}
			dc.curDashOffset = fmodf(ctx->dashOffset, dc.totDashLength);	//cur dash offset between defined path point and last dash segment(on/off) start
			str.iL = lastPathPointIdx;
		} else if (_path_is_closed(ctx,ptrPath)){
			str.iL = lastPathPointIdx;
		}else{
			_draw_stoke_cap(ctx, hw, ctx->points[str.cp], vec2_line_norm(ctx->points[str.cp], ctx->points[str.cp+1]), true);
			str.iL = str.cp++;
		}

		if (_path_has_curves (ctx,ptrPath)) {
			while (str.cp < lastPathPointIdx){

				bool curved = ctx->pathes [ptrPath + ptrSegment] & PATH_HAS_CURVES_BIT;
				if (lastSegmentPointIdx == lastPathPointIdx)//last segment of path, dont draw end point here
					lastSegmentPointIdx--;
				while (str.cp <= lastSegmentPointIdx)
					_draw_segment(ctx, hw, &str, &dc, curved);

				ptrSegment ++;
				uint32_t cptSegPts = ctx->pathes [ptrPath + ptrSegment]&PATH_ELT_MASK;
				lastSegmentPointIdx = str.cp + cptSegPts - 1;
				if (lastSegmentPointIdx == lastPathPointIdx && cptSegPts == 1) {
					//single point last segment
					ptrSegment++;
					break;
				}
			}
		}else while (str.cp < lastPathPointIdx)
			_draw_segment(ctx, hw, &str, &dc, false);

		if (ctx->dashCount > 0) {
			if (_path_is_closed(ctx,ptrPath)){
				str.iR = firstPathPointIdx;

				_draw_dashed_segment(ctx, hw, &str, &dc, false);

				str.iL++;
				str.cp++;
			}
			if (!dc.dashOn){
				//finishing last dash that is already started, draw end caps but not too close to start
				//the default gap is the next void
				int32_t prevDash = (int32_t)dc.curDash-1;
				if (prevDash < 0)
					dc.curDash = ctx->dashCount-1;
				float m = fminf (ctx->dashes[prevDash] - dc.curDashOffset, ctx->dashes[dc.curDash]);
				vec2 p = vec2_sub(ctx->points[str.iR], vec2_mult_s(dc.normal, m));
				_draw_stoke_cap (ctx, hw, p, dc.normal, false);
			}
		} else if (_path_is_closed(ctx,ptrPath)){
			str.iR = firstPathPointIdx;
			float cross = _build_vb_step (ctx, hw, &str, false);

			VKVG_IBO_INDEX_TYPE* inds = &ctx->indexCache [ctx->indCount-6];
			VKVG_IBO_INDEX_TYPE ii = str.firstIdx;
			if (cross < 0 && ctx->lineJoin != VKVG_LINE_JOIN_MITER){
				inds[1] = ii+1;
				inds[4] = ii+1;
				inds[5] = ii;
			}else{
				inds[1] = ii;
				inds[4] = ii;
				inds[5] = ii+1;
			}
			str.cp++;
		}else
			_draw_stoke_cap (ctx, hw, ctx->points[str.cp], vec2_line_norm(ctx->points[str.cp-1], ctx->points[str.cp]), false);

		str.cp = firstPathPointIdx + pathPointCount;

		if (ptrSegment > 0)
			ptrPath += ptrSegment;
		else
			ptrPath++;

		//limit batch size here to 1/3 of the ibo index type ability
		if (ctx->vertCount - ctx->curVertOffset > VKVG_IBO_MAX / 3)
			_emit_draw_cmd_undrawn_vertices(ctx);
	}

}
void vkvg_paint (VkvgContext ctx){
	if (ctx->status)
		return;

	_finish_path (ctx);

	if (ctx->pathPtr) {
		vkvg_fill(ctx);
		return;
	}

	_ensure_renderpass_is_started (ctx);
	_draw_full_screen_quad (ctx, true);
}
void vkvg_set_source_color (VkvgContext ctx, uint32_t c) {
	if (ctx->status)
		return;
	ctx->curColor = c;
	_update_cur_pattern (ctx, NULL);
}
void vkvg_set_source_rgb (VkvgContext ctx, float r, float g, float b) {
	vkvg_set_source_rgba (ctx, r, g, b, 1);
}
void vkvg_set_source_rgba (VkvgContext ctx, float r, float g, float b, float a)
{
	if (ctx->status)
		return;
	ctx->curColor = CreateRgbaf(r,g,b,a);
	_update_cur_pattern (ctx, NULL);
}
void vkvg_set_source_surface(VkvgContext ctx, VkvgSurface surf, float x, float y){
	if (ctx->status)
		return;
	_update_cur_pattern (ctx, vkvg_pattern_create_for_surface(surf));
	ctx->pushConsts.source.x = x;
	ctx->pushConsts.source.y = y;
	ctx->pushCstDirty = true;
}
void vkvg_set_source (VkvgContext ctx, VkvgPattern pat){
	if (ctx->status)
		return;
	_update_cur_pattern (ctx, pat);
	vkvg_pattern_reference	(pat);
}
void vkvg_set_line_width (VkvgContext ctx, float width){
	ctx->lineWidth = width;
}
void vkvg_set_line_cap (VkvgContext ctx, vkvg_line_cap_t cap){
	ctx->lineCap = cap;
}
void vkvg_set_line_join (VkvgContext ctx, vkvg_line_join_t join){
	ctx->lineJoin = join;
}
void vkvg_set_operator (VkvgContext ctx, vkvg_operator_t op){
	if (ctx->status)
		return;
	if (op == ctx->curOperator)
		return;

	_emit_draw_cmd_undrawn_vertices(ctx);//draw call with different ops cant be combined, so emit draw cmd for previous vertices.

	ctx->curOperator = op;

	if (ctx->cmdStarted)
		_bind_draw_pipeline (ctx);
}
void vkvg_set_fill_rule (VkvgContext ctx, vkvg_fill_rule_t fr){
#ifndef __APPLE__
	ctx->curFillRule = fr;
#endif
}
vkvg_fill_rule_t vkvg_get_fill_rule (VkvgContext ctx){
	return ctx->curFillRule;
}
float vkvg_get_line_width (VkvgContext ctx){
	return ctx->lineWidth;
}
void vkvg_set_dash (VkvgContext ctx, const float* dashes, uint32_t num_dashes, float offset){
	if (ctx->status)
		return;
	if (ctx->dashCount > 0)
		free (ctx->dashes);
	ctx->dashCount = num_dashes;
	ctx->dashOffset = offset;
	if (ctx->dashCount == 0)
		return;
	ctx->dashes = (float*)malloc (sizeof(float) * ctx->dashCount);
	memcpy (ctx->dashes, dashes, sizeof(float) * ctx->dashCount);
}
void vkvg_get_dash (VkvgContext ctx, const float* dashes, uint32_t* num_dashes, float* offset){
	*num_dashes = ctx->dashCount;
	*offset = ctx->dashOffset;
	if (ctx->dashCount == 0 || dashes == NULL)
		return;
	memcpy ((float*)dashes, ctx->dashes, sizeof(float) * ctx->dashCount);
}


vkvg_line_cap_t vkvg_get_line_cap (VkvgContext ctx){
	return ctx->lineCap;
}
vkvg_line_join_t vkvg_get_line_join (VkvgContext ctx){
	return ctx->lineJoin;
}
vkvg_operator_t vkvg_get_operator (VkvgContext ctx){
	return ctx->curOperator;
}
VkvgPattern vkvg_get_source (VkvgContext ctx){
	vkvg_pattern_reference (ctx->pattern);
	return ctx->pattern;
}

void vkvg_select_font_face (VkvgContext ctx, const char* name){
	if (ctx->status)
		return;
	_select_font_face (ctx, name);
}
void vkvg_select_font_path (VkvgContext ctx, const char* path){
	if (ctx->status)
		return;
	//_select_font_path (ctx, path);
}
void vkvg_set_font_size (VkvgContext ctx, uint32_t size){
	if (ctx->status)
		return;
	FT_F26Dot6 newSize = size << 6;
	if (ctx->selectedCharSize == newSize)
		return;
	ctx->selectedCharSize = newSize;
	ctx->currentFont = NULL;
	ctx->currentFontSize = NULL;
}

void vkvg_set_text_direction (vkvg_context* ctx, vkvg_direction_t direction){

}

void vkvg_show_text (VkvgContext ctx, const char* text){
	if (ctx->status)
		return;
	//_ensure_renderpass_is_started(ctx);
	_show_text (ctx, text);
	//_flush_undrawn_vertices (ctx);
}

VkvgText vkvg_text_run_create (VkvgContext ctx, const char* text) {
	if (ctx->status)
		return NULL;
	VkvgText tr = (vkvg_text_run_t*)calloc(1, sizeof(vkvg_text_run_t));
	_create_text_run(ctx, text, tr);
	return tr;
}
void vkvg_text_run_destroy (VkvgText textRun) {
	_destroy_text_run (textRun);
	free (textRun);
}
void vkvg_show_text_run (VkvgContext ctx, VkvgText textRun) {
	if (ctx->status)
		return;
	_show_text_run(ctx, textRun);
}
void vkvg_text_run_get_extents (VkvgText textRun, vkvg_text_extents_t* extents) {
	extents = &textRun->extents;
}

void vkvg_text_extents (VkvgContext ctx, const char* text, vkvg_text_extents_t* extents) {
	if (ctx->status)
		return;
	_text_extents(ctx, text, extents);
}
void vkvg_font_extents (VkvgContext ctx, vkvg_font_extents_t* extents) {
	if (ctx->status)
		return;
	_font_extents(ctx, extents);
}

void vkvg_save (VkvgContext ctx){
	if (ctx->status)
		return;
	LOG(VKVG_LOG_INFO, "SAVE CONTEXT: ctx = %p\n", ctx);

	_flush_cmd_buff (ctx);
	_wait_flush_fence (ctx);

	VkvgDevice dev = ctx->pSurf->dev;
	vkvg_context_save_t* sav = (vkvg_context_save_t*)calloc(1,sizeof(vkvg_context_save_t));

	uint8_t curSaveStencil = ctx->curSavBit / 6;

	if (ctx->curSavBit > 0 && ctx->curSavBit % 6 == 0){//new save/restore stencil image have to be created
		VkhImage* savedStencilsPtr = (VkhImage*)realloc(ctx->savedStencils, curSaveStencil * sizeof(VkhImage));
		if (savedStencilsPtr == NULL) {
			free(sav);
			ctx->status = VKVG_STATUS_NO_MEMORY;
			return;
		}
		ctx->savedStencils = savedStencilsPtr;
		VkhImage savStencil = vkh_image_ms_create ((VkhDevice)dev, dev->stencilFormat, dev->samples, ctx->pSurf->width, ctx->pSurf->height,
								VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		ctx->savedStencils[curSaveStencil-1] = savStencil;

		vkh_cmd_begin (ctx->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		ctx->cmdStarted = true;

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
		vkh_cmd_label_start(ctx->cmd, "new save/restore stencil", DBG_LAB_COLOR_SAV);
#endif

		vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencil, VK_IMAGE_ASPECT_STENCIL_BIT,
							  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		vkh_image_set_layout (ctx->cmd, savStencil, VK_IMAGE_ASPECT_STENCIL_BIT,
							  VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							  VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		VkImageCopy cregion = { .srcSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1},
								.dstSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1},
								.extent = {ctx->pSurf->width,ctx->pSurf->height,1}};
		vkCmdCopyImage(ctx->cmd,
					   vkh_image_get_vkimage (ctx->pSurf->stencil),VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					   vkh_image_get_vkimage (savStencil),		 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					   1, &cregion);

		vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencil, VK_IMAGE_ASPECT_STENCIL_BIT,
							  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
							  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
		vkh_cmd_label_end (ctx->cmd);
#endif

		VK_CHECK_RESULT(vkEndCommandBuffer(ctx->cmd));
		_wait_and_submit_cmd(ctx);
	}

	uint8_t curSaveBit = 1 << (ctx->curSavBit % 6 + 2);

	_start_cmd_for_render_pass (ctx);

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_cmd_label_start(ctx->cmd, "save rp", DBG_LAB_COLOR_SAV);
#endif

	CmdBindPipeline			(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineClipping);

	CmdSetStencilReference	(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT|curSaveBit);
	CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
	CmdSetStencilWriteMask	(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, curSaveBit);

	_draw_full_screen_quad (ctx, false);

	_bind_draw_pipeline (ctx);
	CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_cmd_label_end (ctx->cmd);
#endif

	sav->dashOffset = ctx->dashOffset;
	sav->dashCount	= ctx->dashCount;
	if (ctx->dashCount > 0) {
		sav->dashes = (float*)malloc (sizeof(float) * ctx->dashCount);
		memcpy (sav->dashes, ctx->dashes, sizeof(float) * ctx->dashCount);
	}
	sav->lineWidth	= ctx->lineWidth;
	sav->curOperator= ctx->curOperator;
	sav->lineCap	= ctx->lineCap;
	sav->lineWidth	= ctx->lineWidth;
	sav->curFillRule= ctx->curFillRule;

	sav->selectedCharSize = ctx->selectedCharSize;
	sav->selectedFontName = (char*)calloc(FONT_NAME_MAX_SIZE,sizeof(char));
	strcpy (sav->selectedFontName, ctx->selectedFontName);

	sav->currentFont  = ctx->currentFont;
	sav->textDirection= ctx->textDirection;
	sav->pushConsts	  = ctx->pushConsts;
	//sav->pattern		= ctx->pattern;//TODO:pattern sav must be imutable (copy?)

	sav->pNext		= ctx->pSavedCtxs;
	ctx->pSavedCtxs = sav;
	ctx->curSavBit++;

	/*if (ctx->pattern)
		vkvg_pattern_reference (ctx->pattern);*/
}
void vkvg_restore (VkvgContext ctx){
	if (ctx->status)
		return;

	if (ctx->pSavedCtxs == NULL){
		ctx->status = VKVG_STATUS_INVALID_RESTORE;
		return;
	}

	LOG(VKVG_LOG_INFO, "RESTORE CONTEXT: ctx = %p\n", ctx);

	_flush_cmd_buff (ctx);
	_wait_flush_fence (ctx);

	vkvg_context_save_t* sav = ctx->pSavedCtxs;
	ctx->pSavedCtxs = sav->pNext;

	ctx->pushConsts	  = sav->pushConsts;

	/*if (sav->pattern)
		_update_cur_pattern (ctx, sav->pattern);*/

	ctx->curSavBit--;

	uint8_t curSaveBit = 1 << (ctx->curSavBit % 6 + 2);

	_start_cmd_for_render_pass (ctx);

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_cmd_label_start(ctx->cmd, "restore rp", DBG_LAB_COLOR_SAV);
#endif

	CmdBindPipeline			(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineClipping);

	CmdSetStencilReference	(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT|curSaveBit);
	CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, curSaveBit);
	CmdSetStencilWriteMask	(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);

	_draw_full_screen_quad (ctx, false);

	_bind_draw_pipeline (ctx);
	CmdSetStencilCompareMask (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
	vkh_cmd_label_end (ctx->cmd);
#endif

	_flush_cmd_buff (ctx);
	_wait_flush_fence (ctx);

	uint8_t curSaveStencil = ctx->curSavBit / 6;
	if (ctx->curSavBit > 0 && ctx->curSavBit % 6 == 0){//addtional save/restore stencil image have to be copied back to surf stencil first
		VkhImage savStencil = ctx->savedStencils[curSaveStencil-1];

		vkh_cmd_begin (ctx->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		ctx->cmdStarted = true;

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
		vkh_cmd_label_start(ctx->cmd, "additional stencil copy while restoring", DBG_LAB_COLOR_SAV);
#endif

		vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencil, VK_IMAGE_ASPECT_STENCIL_BIT,
							  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		vkh_image_set_layout (ctx->cmd, savStencil, VK_IMAGE_ASPECT_STENCIL_BIT,
							  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		VkImageCopy cregion = { .srcSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1},
								.dstSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1},
								.extent = {ctx->pSurf->width,ctx->pSurf->height,1}};
		vkCmdCopyImage(ctx->cmd,
					   vkh_image_get_vkimage (savStencil),		 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					   vkh_image_get_vkimage (ctx->pSurf->stencil),VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					   1, &cregion);
		vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencil, VK_IMAGE_ASPECT_STENCIL_BIT,
							  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
							  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

#if defined(DEBUG) && defined (VKVG_DBG_UTILS)
		vkh_cmd_label_end (ctx->cmd);
#endif

		VK_CHECK_RESULT(vkEndCommandBuffer(ctx->cmd));
		_wait_and_submit_cmd (ctx);
		_wait_flush_fence (ctx);
		vkh_image_destroy (savStencil);
	}

	ctx->dashOffset = sav->dashOffset;
	if (ctx->dashCount > 0)
		free (ctx->dashes);
	ctx->dashCount	= sav->dashCount;
	if (ctx->dashCount > 0) {
		ctx->dashes = (float*)malloc (sizeof(float) * ctx->dashCount);
		memcpy (ctx->dashes, sav->dashes, sizeof(float) * ctx->dashCount);
	}

	ctx->lineWidth	= sav->lineWidth;
	ctx->curOperator= sav->curOperator;
	ctx->lineCap	= sav->lineCap;
	ctx->lineJoin	= sav->lineJoint;
	ctx->curFillRule= sav->curFillRule;

	ctx->selectedCharSize = sav->selectedCharSize;
	strcpy (ctx->selectedFontName, sav->selectedFontName);

	ctx->currentFont  = sav->currentFont;
	ctx->textDirection= sav->textDirection;

	_free_ctx_save(sav);
}

void vkvg_translate (VkvgContext ctx, float dx, float dy){
	if (ctx->status)
		return;
	_emit_draw_cmd_undrawn_vertices(ctx);
	vkvg_matrix_translate (&ctx->pushConsts.mat, dx, dy);
	_set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_scale (VkvgContext ctx, float sx, float sy){
	if (ctx->status)
		return;
	_emit_draw_cmd_undrawn_vertices(ctx);
	vkvg_matrix_scale (&ctx->pushConsts.mat, sx, sy);
	_set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_rotate (VkvgContext ctx, float radians){
	if (ctx->status)
		return;
	_emit_draw_cmd_undrawn_vertices(ctx);
	vkvg_matrix_rotate (&ctx->pushConsts.mat, radians);
	_set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_transform (VkvgContext ctx, const vkvg_matrix_t* matrix) {
	if (ctx->status)
		return;
	_emit_draw_cmd_undrawn_vertices(ctx);
	vkvg_matrix_t res;
	vkvg_matrix_multiply (&res, &ctx->pushConsts.mat, matrix);
	ctx->pushConsts.mat = res;
	_set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_identity_matrix (VkvgContext ctx) {
	if (ctx->status)
		return;
	_emit_draw_cmd_undrawn_vertices(ctx);
	vkvg_matrix_t im = VKVG_IDENTITY_MATRIX;
	ctx->pushConsts.mat = im;
	_set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_set_matrix (VkvgContext ctx, const vkvg_matrix_t* matrix){
	if (ctx->status)
		return;
	_emit_draw_cmd_undrawn_vertices(ctx);
	ctx->pushConsts.mat = (*matrix);
	_set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_get_matrix (VkvgContext ctx, const vkvg_matrix_t* matrix){
	memcpy ((void*)matrix, &ctx->pushConsts.mat, sizeof(vkvg_matrix_t));
}

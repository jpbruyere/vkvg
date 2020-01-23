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

//credits for bezier algorithms to:
//      Anti-Grain Geometry (AGG) - Version 2.5
//      A high quality rendering engine for C++
//      Copyright (C) 2002-2006 Maxim Shemanarev
//      Contact: mcseem@antigrain.com
//               mcseemagg@yahoo.com
//               http://antigrain.com


#include "vkvg_surface_internal.h"
#include "vkvg_context_internal.h"
#include "vkvg_device_internal.h"
#include "vkvg_pattern.h"
#include "vkh_queue.h"
#include "vkh_image.h"

void _check_flush_needed (VkvgContext ctx) {
    if (!ctx->cmdStarted)
        return;
    if (ctx->pointCount * 4 < ctx->sizeIndices - ctx->indCount)
        return;
    _flush_cmd_buff(ctx);
}
void _check_vbo_size (VkvgContext ctx) {
    if (ctx->sizeVertices - ctx->vertCount > VKVG_ARRAY_THRESHOLD)
        return;
    ctx->sizeVertices += VKVG_VBO_SIZE;
    Vertex* tmp = (Vertex*) realloc (ctx->vertexCache, ctx->sizeVertices * sizeof(Vertex));
    if (tmp == NULL)
        ctx->status = VKVG_STATUS_NO_MEMORY;
    else
        ctx->vertexCache = tmp;
}
void _check_ibo_size (VkvgContext ctx) {
    if (ctx->sizeIndices - ctx->indCount > VKVG_ARRAY_THRESHOLD)
        return;
    ctx->sizeIndices += VKVG_IBO_SIZE;
    VKVG_IBO_INDEX_TYPE* tmp = (VKVG_IBO_INDEX_TYPE*) realloc (ctx->indexCache, ctx->sizeIndices * sizeof(VKVG_IBO_INDEX_TYPE));
    if (tmp == NULL)
        ctx->status = VKVG_STATUS_NO_MEMORY;
    else
        ctx->indexCache = tmp;
}
void _check_pathes_array (VkvgContext ctx){
    if (ctx->sizePathes - ctx->pathPtr - ctx->curvePtr > VKVG_ARRAY_THRESHOLD)
        return;
    ctx->sizePathes += VKVG_PATHES_SIZE;
    uint32_t* tmp = (uint32_t*) realloc (ctx->pathes, ctx->sizePathes * sizeof(uint32_t));
    if (tmp == NULL){
        ctx->status = VKVG_STATUS_NO_MEMORY;
        ctx->pathPtr = 0 + (ctx->pathPtr % 2);
    }else
        ctx->pathes = tmp;
}
void _check_point_array (VkvgContext ctx){
    if (ctx->sizePoints - ctx->pointCount > VKVG_ARRAY_THRESHOLD)
        return;
    ctx->sizePoints += VKVG_PATHES_SIZE;
    vec2* tmp = (vec2*) realloc (ctx->points, ctx->sizePoints * sizeof(vec2));
    if (tmp == NULL){
        ctx->status = VKVG_STATUS_NO_MEMORY;
        ctx->pointCount = 0;
    }else
        ctx->points = tmp;
}
//when empty, ptr is even, else it's odd
//when empty, no current point is defined.
bool _current_path_is_empty (VkvgContext ctx) {
    return ctx->pathPtr % 2 == 0;
}
//this function expect that current point exists
vec2 _get_current_position (VkvgContext ctx) {
    return ctx->points[ctx->pointCount-1];
}
//set curve start point and set path has curve bit
void _set_curve_start (VkvgContext ctx) {
    ctx->pathes[ctx->pathPtr+ctx->curvePtr+1] = (ctx->pointCount - 1);
    ctx->pathes[ctx->pathPtr-1] |= PATH_HAS_CURVES_BIT;
}
//set curve end point and set path has curve bit
void _set_curve_end (VkvgContext ctx) {
    ctx->pathes[ctx->pathPtr+ctx->curvePtr+2] = (ctx->pointCount - 1)|PATH_IS_CURVE_BIT;
    ctx->curvePtr+=2;
    _check_pathes_array(ctx);
}
//path start pointed at ptrPath has curve bit
bool _path_has_curves (VkvgContext ctx, uint32_t ptrPath) {
    return ctx->pathes[ptrPath] & PATH_HAS_CURVES_BIT;
}
//this function expect that current path is empty
void _start_sub_path (VkvgContext ctx, float x, float y) {
    //set start to current idx in point array
    ctx->pathes[ctx->pathPtr] = ctx->pointCount;
    _add_point(ctx, x, y);
    _check_pathes_array(ctx);
    ctx->pathPtr++;
}
void _finish_path (VkvgContext ctx){
    if (_current_path_is_empty(ctx))
        return;
    if (ctx->pathes[ctx->pathPtr-1] == ctx->pointCount - 1){
        //only current pos is in path
        ctx->pathPtr--;
        return;
    }

    //set end index of current path to last point in points array
    ctx->pathes[ctx->pathPtr] = ctx->pointCount - 1;
    _check_pathes_array(ctx);
    ctx->pathPtr += ctx->curvePtr + 1;
    ctx->curvePtr = 0;
}
void _clear_path (VkvgContext ctx){
    ctx->pathPtr = 0;
    ctx->pointCount = 0;
    ctx->curvePtr = 0;
    _resetMinMax(ctx);
}
bool _path_is_closed (VkvgContext ctx, uint32_t ptrPath){
    return ctx->pathes[ptrPath] & PATH_CLOSED_BIT;
}
void _resetMinMax (VkvgContext ctx) {
    ctx->xMin = ctx->yMin = FLT_MAX;
    ctx->xMax = ctx->yMax = FLT_MIN;
}
void _add_point (VkvgContext ctx, float x, float y){
    ctx->points[ctx->pointCount] = (vec2){x,y};
    ctx->pointCount++;

    _check_point_array(ctx);

    //bounds are computed here to scissor the painting operation
    //that speed up fill drastically.
    vkvg_matrix_transform_point (&ctx->pushConsts.mat, &x, &y);

    if (x < ctx->xMin)
        ctx->xMin = x;
    if (x > ctx->xMax)
        ctx->xMax = x;
    if (y < ctx->yMin)
        ctx->yMin = y;
    if (y > ctx->yMax)
        ctx->yMax = y;
}
float _normalizeAngle(float a)
{
    float res = ROUND_DOWN(fmodf(a,2.0f*M_PIF),100);
    if (res < 0.0f)
        return res + 2.0f*M_PIF;
    else
        return res;
}
float _get_arc_step (float radius) {
    return M_PIF/sqrtf(radius)*0.35f;
}
void _create_gradient_buff (VkvgContext ctx){
    vkvg_buffer_create (ctx->pSurf->dev,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        sizeof(vkvg_gradient_t), &ctx->uboGrad);
}
void _create_vertices_buff (VkvgContext ctx){
    vkvg_buffer_create (ctx->pSurf->dev,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        ctx->sizeVBO * sizeof(Vertex), &ctx->vertices);
    vkvg_buffer_create (ctx->pSurf->dev,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        ctx->sizeIBO * sizeof(VKVG_IBO_INDEX_TYPE), &ctx->indices);
}
void _resize_vbo (VkvgContext ctx, size_t new_size) {
    _wait_flush_fence (ctx);//wait previous cmd if not completed
    ctx->sizeVBO = new_size;
    ctx->sizeVBO += ctx->sizeVBO % VKVG_VBO_SIZE;
    vkvg_buffer_destroy (&ctx->vertices);
    vkvg_buffer_create (ctx->pSurf->dev,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        ctx->sizeVBO * sizeof(Vertex), &ctx->vertices);
}
void _resize_ibo (VkvgContext ctx, size_t new_size) {
    _wait_flush_fence (ctx);//wait previous cmd if not completed
    ctx->sizeIBO = ctx->sizeIndices;
    ctx->sizeIBO += ctx->sizeIBO % VKVG_IBO_SIZE;
    vkvg_buffer_destroy (&ctx->indices);
    vkvg_buffer_create (ctx->pSurf->dev,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        ctx->sizeIBO * sizeof(VKVG_IBO_INDEX_TYPE), &ctx->indices);
}
void _add_vertexf (VkvgContext ctx, float x, float y){
    Vertex* pVert = &ctx->vertexCache[ctx->vertCount];
    pVert->pos.x = x;
    pVert->pos.y = y;
    pVert->uv = (vec3){0,0,-1};
    ctx->vertCount++;

    _check_vbo_size(ctx);
}
void _add_vertex(VkvgContext ctx, Vertex v){
    ctx->vertexCache[ctx->vertCount] = v;
    ctx->vertCount++;

    _check_vbo_size(ctx);
}
void _set_vertex(VkvgContext ctx, uint32_t idx, Vertex v){
    ctx->vertexCache[idx] = v;
}
void _add_tri_indices_for_rect (VkvgContext ctx, VKVG_IBO_INDEX_TYPE i){
    VKVG_IBO_INDEX_TYPE* inds = &ctx->indexCache[ctx->indCount];
    inds[0] = i;
    inds[1] = i+2;
    inds[2] = i+1;
    inds[3] = i+1;
    inds[4] = i+2;
    inds[5] = i+3;
    ctx->indCount+=6;

    _check_ibo_size(ctx);
    LOG(LOG_INFO, "Rectangle IDX: %d %d %d | %d %d %d (count=%d)\n", inds[0], inds[1], inds[2], inds[3], inds[4], inds[5], ctx->indCount);
}
void _add_triangle_indices(VkvgContext ctx, VKVG_IBO_INDEX_TYPE i0, VKVG_IBO_INDEX_TYPE i1, VKVG_IBO_INDEX_TYPE i2){
    VKVG_IBO_INDEX_TYPE* inds = &ctx->indexCache[ctx->indCount];
    inds[0] = i0;
    inds[1] = i1;
    inds[2] = i2;
    ctx->indCount+=3;

    _check_ibo_size(ctx);
    LOG(LOG_INFO, "Triangle IDX: %d %d %d (count=%d)\n", i0,i1,i2,ctx->indCount);
}
void _vao_add_rectangle (VkvgContext ctx, float x, float y, float width, float height){
    Vertex v[4] =
    {
        {{x,y},             {0,0,-1}},
        {{x,y+height},      {0,0,-1}},
        {{x+width,y},       {0,0,-1}},
        {{x+width,y+height},{0,0,-1}}
    };
    VKVG_IBO_INDEX_TYPE firstIdx = ctx->vertCount - ctx->curVertOffset;
    Vertex* pVert = &ctx->vertexCache[ctx->vertCount];
    memcpy (pVert,v,4*sizeof(Vertex));
    ctx->vertCount+=4;

    _check_vbo_size(ctx);

    _add_tri_indices_for_rect(ctx, firstIdx);
}

void _check_cmd_buff_state (VkvgContext ctx) {
    if (!ctx->cmdStarted)
        _start_cmd_for_render_pass(ctx);
    else if (ctx->pushCstDirty)
        _update_push_constants(ctx);
}
void _create_cmd_buff (VkvgContext ctx){
    vkh_cmd_buffs_create((VkhDevice)ctx->pSurf->dev, ctx->cmdPool,VK_COMMAND_BUFFER_LEVEL_PRIMARY, 2, ctx->cmdBuffers);
#if defined(DEBUG) && defined(ENABLE_VALIDATION)
    vkh_device_set_object_name((VkhDevice)ctx->pSurf->dev, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, (uint64_t)ctx->cmd, "vkvgCtxCmd");
#endif
}
void _clear_attachment (VkvgContext ctx) {

}
void _wait_flush_fence (VkvgContext ctx) {
    vkWaitForFences (ctx->pSurf->dev->vkDev, 1, &ctx->flushFence, VK_TRUE, VKVG_FENCE_TIMEOUT);
}
void _reset_flush_fence (VkvgContext ctx) {
    vkResetFences (ctx->pSurf->dev->vkDev, 1, &ctx->flushFence);
}
void _wait_and_submit_cmd (VkvgContext ctx){
    if (!ctx->cmdStarted)//current cmd buff is empty, be aware that wait is also canceled!!
        return;

    _wait_flush_fence (ctx);
    _reset_flush_fence(ctx);

    _submit_cmd (ctx->pSurf->dev, &ctx->cmd, ctx->flushFence);

    if (ctx->cmd == ctx->cmdBuffers[0])
        ctx->cmd = ctx->cmdBuffers[1];
    else
        ctx->cmd = ctx->cmdBuffers[0];

    vkResetCommandBuffer (ctx->cmd, 0);
    ctx->cmdStarted = false;
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
    _wait_flush_fence (ctx);

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
void _flush_vertices_caches (VkvgContext ctx) {
    //copy vertex and index caches to the vbo and ibo vkbuffers
    _wait_flush_fence (ctx);

    memcpy(ctx->vertices.allocInfo.pMappedData, ctx->vertexCache, ctx->vertCount * sizeof (Vertex));
    memcpy(ctx->indices.allocInfo.pMappedData, ctx->indexCache, ctx->indCount * sizeof (VKVG_IBO_INDEX_TYPE));

    ctx->vertCount = ctx->indCount = ctx->curIndStart = ctx->curVertOffset = 0;
}
//this func expect cmdStarted to be true
void _end_render_pass (VkvgContext ctx) {
    LOG(LOG_INFO, "END RENDER PASS: ctx = %lu;\n", ctx);
    CmdEndRenderPass      (ctx->cmd);
#ifdef DEBUG
    vkh_cmd_label_end (ctx->cmd);
#endif
    ctx->renderPassBeginInfo.renderPass = ctx->pSurf->dev->renderPass;
}
void _record_draw_cmd (VkvgContext ctx){
    if (ctx->indCount == ctx->curIndStart)
        return;

    if (ctx->vertCount > ctx->sizeVBO || ctx->indCount > ctx->sizeIBO){
        //vbo or ibo buffers too small
        if (ctx->cmdStarted) {
            //if cmd is started buffers, are already bound, so no resize is possible
            //instead we flush, and clear vbo and ibo caches
            _end_render_pass (ctx);
            _flush_vertices_caches_until_vertex_base (ctx);
            vkh_cmd_end (ctx->cmd);
            _wait_and_submit_cmd (ctx);
            //we could resize here
            _resize_vbo(ctx, ctx->sizeVertices);
            _resize_ibo(ctx, ctx->sizeIndices);
        }else{
            //should resize vbo here
            _resize_vbo(ctx, ctx->sizeVertices);
            _resize_ibo(ctx, ctx->sizeIndices);
        }
    }

    _check_cmd_buff_state(ctx);
    CmdDrawIndexed(ctx->cmd, ctx->indCount - ctx->curIndStart, 1, ctx->curIndStart, (int32_t)ctx->curVertOffset, 0);

    LOG(LOG_INFO, "RECORD DRAW CMD: ctx = %lu; vertices = %d; indices = %d (vxOff = %d idxStart = %d idxTot = %d )\n",
        (uint64_t)ctx, ctx->vertCount - ctx->curVertOffset,
        ctx->indCount - ctx->curIndStart, ctx->curVertOffset, ctx->curIndStart, ctx->indCount);

#ifdef VKVG_WIRED_DEBUG
    CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineWired);
    CmdDrawIndexed(ctx->cmd, ctx->indCount - ctx->curIndStart, 1, ctx->curIndStart, (int32_t)ctx->curVertOffset, 0);
    CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipe_OVER);
#endif

    ctx->curIndStart = ctx->indCount;
    ctx->curVertOffset = ctx->vertCount;
}
void _flush_cmd_buff (VkvgContext ctx){
    if (!ctx->cmdStarted)
        return;

    _record_draw_cmd        (ctx);
    _end_render_pass        (ctx);
    _flush_vertices_caches  (ctx);
    vkh_cmd_end             (ctx->cmd);

    LOG(LOG_INFO, "FLUSH CTX: ctx = %lu; vertices = %d; indices = %d\n", ctx, ctx->vertCount, ctx->indCount);
    _wait_and_submit_cmd(ctx);
}

//bind correct draw pipeline depending on current OPERATOR
void _bind_draw_pipeline (VkvgContext ctx) {
    switch (ctx->curOperator) {
    case VKVG_OPERATOR_OVER:
        CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipe_OVER);
        break;
    case VKVG_OPERATOR_CLEAR:
        vkvg_set_source_rgba(ctx,0,0,0,0);
        CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipe_CLEAR);
        break;
    default:
        CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipe_OVER);
        break;
    }
}
const float LAB_COLOR_RP[4] = {0,0,1,1};

void _start_cmd_for_render_pass (VkvgContext ctx) {
    LOG(LOG_INFO, "START RENDER PASS: ctx = %lu\n", ctx);
    vkh_cmd_begin (ctx->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    if (ctx->pSurf->img->layout == VK_IMAGE_LAYOUT_UNDEFINED){
        VkhImage imgMs = ctx->pSurf->imgMS;
        if (imgMs != NULL)
            vkh_image_set_layout(ctx->cmd, imgMs, VK_IMAGE_ASPECT_COLOR_BIT,
                                 VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        vkh_image_set_layout(ctx->cmd, ctx->pSurf->img, VK_IMAGE_ASPECT_COLOR_BIT,
                         VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

#ifdef DEBUG
    vkh_cmd_label_start(ctx->cmd, "ctx render pass", LAB_COLOR_RP);
#endif

    CmdBeginRenderPass (ctx->cmd, &ctx->renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    VkViewport viewport = {0,0,ctx->pSurf->width,ctx->pSurf->height,0,1};
    CmdSetViewport(ctx->cmd, 0, 1, &viewport);

    CmdSetScissor(ctx->cmd, 0, 1, &ctx->bounds);

    VkDescriptorSet dss[] = {ctx->dsFont,ctx->dsSrc,ctx->dsGrad};
    CmdBindDescriptorSets(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineLayout,
                            0, 3, dss, 0, NULL);

    VkDeviceSize offsets[1] = { 0 };
    CmdBindVertexBuffers(ctx->cmd, 0, 1, &ctx->vertices.buffer, offsets);
    if (sizeof (VKVG_IBO_INDEX_TYPE) == 4)
        CmdBindIndexBuffer(ctx->cmd, ctx->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
    else
        CmdBindIndexBuffer(ctx->cmd, ctx->indices.buffer, 0, VK_INDEX_TYPE_UINT16);

    _update_push_constants  (ctx);

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
    CmdPushConstants(ctx->cmd, ctx->pSurf->dev->pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constants),&ctx->pushConsts);
    ctx->pushCstDirty = false;
}
void _update_cur_pattern (VkvgContext ctx, VkvgPattern pat) {
    VkvgPattern lastPat = ctx->pattern;
    ctx->pattern = pat;

    ctx->pushConsts.patternType = pat->type;
    ctx->pushCstDirty = true;

    switch (pat->type)  {
    case VKVG_PATTERN_TYPE_SOLID:
        memcpy (&ctx->pushConsts.source, ctx->pattern->data, sizeof(vkvg_color_t));

        if (lastPat && lastPat->type == VKVG_PATTERN_TYPE_SURFACE){
            _flush_cmd_buff             (ctx);
            _update_descriptor_set      (ctx, ctx->pSurf->dev->emptyImg, ctx->dsSrc);
            //_init_cmd_buff              (ctx);//push csts updated by init
        }//else
            //_update_push_constants (ctx);

        break;
    case VKVG_PATTERN_TYPE_SURFACE:
    {
        VkvgSurface surf = (VkvgSurface)pat->data;

        //flush ctx in two steps to add the src transitioning in the cmd buff
        if (ctx->cmdStarted){//transition of img without appropriate dependencies in subpass must be done outside renderpass.
            _record_draw_cmd (ctx);//ensure all vertices are flushed
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

        vkh_cmd_end (ctx->cmd);
        _wait_and_submit_cmd (ctx);

        ctx->source = surf->img;

        //if (vkh_image_get_sampler (ctx->source) == VK_NULL_HANDLE){
            VkSamplerAddressMode addrMode;
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
            }
            vkh_image_create_sampler(ctx->source, filter, filter,
                                 VK_SAMPLER_MIPMAP_MODE_NEAREST, addrMode);
        //}
        /*if (vkh_image_get_layout (ctx->source) != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
            vkh_cmd_begin (ctx->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

            vkh_image_set_layout (ctx->cmd, ctx->source, VK_IMAGE_ASPECT_COLOR_BIT,
                                  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            vkh_cmd_end (ctx->cmd);

            _submit_wait_and_reset_cmd  (ctx);
        }*/

        _update_descriptor_set          (ctx, ctx->source, ctx->dsSrc);

        vec4 srcRect = {0,0,surf->width,surf->height};
        ctx->pushConsts.source = srcRect;

        //_init_cmd_buff                  (ctx);
        break;
    }
    case VKVG_PATTERN_TYPE_LINEAR:
        _flush_cmd_buff (ctx);

        if (lastPat && lastPat->type == VKVG_PATTERN_TYPE_SURFACE)
            _update_descriptor_set (ctx, ctx->pSurf->dev->emptyImg, ctx->dsSrc);

        vec4 bounds = {ctx->pSurf->width, ctx->pSurf->height, 0, 0};//store img bounds in unused source field
        ctx->pushConsts.source = bounds;

        //transform control point with current ctx matrix
        vkvg_gradient_t grad = {0};
        memcpy(&grad, pat->data, sizeof(vkvg_gradient_t));

        vkvg_matrix_transform_point(&ctx->pushConsts.mat, &grad.cp[0].x, &grad.cp[0].y);
        vkvg_matrix_transform_point(&ctx->pushConsts.mat, &grad.cp[1].x, &grad.cp[1].y);
        //to do, scale radial radiuses in cp[2]

        memcpy(ctx->uboGrad.allocInfo.pMappedData , &grad, sizeof(vkvg_gradient_t));

        //_init_cmd_buff (ctx);
        break;
    }

    if (lastPat)
        vkvg_pattern_destroy    (lastPat);
}
void _update_descriptor_set (VkvgContext ctx, VkhImage img, VkDescriptorSet ds){
    _wait_flush_fence(ctx);//descriptorSet update invalidate cmd buffs
    VkDescriptorImageInfo descSrcTex = vkh_image_get_descriptor (img, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VkWriteDescriptorSet writeDescriptorSet = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = ds,
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &descSrcTex
    };
    vkUpdateDescriptorSets(ctx->pSurf->dev->vkDev, 1, &writeDescriptorSet, 0, NULL);
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
    vkUpdateDescriptorSets(ctx->pSurf->dev->vkDev, 1, &writeDescriptorSet, 0, NULL);
}
/*
 * Reset currently bound descriptor which image could be destroyed
 */
/*void _reset_src_descriptor_set (VkvgContext ctx){
    VkvgDevice dev = ctx->pSurf->dev;
    //VkDescriptorSet dss[] = {ctx->dsSrc};
    vkFreeDescriptorSets    (dev->vkDev, ctx->descriptorPool, 1, &ctx->dsSrc);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                                              .descriptorPool = ctx->descriptorPool,
                                                              .descriptorSetCount = 1,
                                                              .pSetLayouts = &dev->dslSrc };
    VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &ctx->dsSrc));
}*/

void _createDescriptorPool (VkvgContext ctx) {
    VkvgDevice dev = ctx->pSurf->dev;
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
    VkvgDevice dev = ctx->pSurf->dev;
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                                              .descriptorPool = ctx->descriptorPool,
                                                              .descriptorSetCount = 1,
                                                              .pSetLayouts = &dev->dslFont };
    VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &ctx->dsFont));
    descriptorSetAllocateInfo.pSetLayouts = &dev->dslSrc;
    VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &ctx->dsSrc));
    descriptorSetAllocateInfo.pSetLayouts = &dev->dslGrad;
    VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &ctx->dsGrad));
}

float _build_vb_step (vkvg_context* ctx, Vertex v, float hw, vec2 pL, vec2 p0, vec2 pR, bool isCurve){
    //if two of the three points are equal, normal is null
    vec2 v0n = vec2_line_norm(pL, p0);
    if (vec2_isnan(v0n))
        return 0;
    vec2 v1n = vec2_line_norm(p0, pR);
    if (vec2_isnan(v1n))
        return 0;

    vec2 bisec = vec2_norm(vec2_add(v0n,v1n));

    float dot = v0n.x * v1n.x + v0n.y * v1n.y;
    float alpha = acosf(dot)/2;
    float cross = v0n.x * v1n.y - v0n.y * v1n.x;

    if (cross<0)
        alpha = -alpha;

    float lh = hw / cosf(alpha);
    bisec = vec2_perp(bisec);
    bisec = vec2_mult(bisec,lh);

    VKVG_IBO_INDEX_TYPE idx = ctx->vertCount - ctx->curVertOffset;

    if (ctx->lineJoin == VKVG_LINE_JOIN_MITER || isCurve){
        v.pos = vec2_add(p0, bisec);
        _add_vertex(ctx, v);
        v.pos = vec2_sub(p0, bisec);
        _add_vertex(ctx, v);
        _add_tri_indices_for_rect(ctx, idx);
    }else{
        vec2 vp = vec2_perp(v0n);
        if (cross<0){
            v.pos = vec2_add (p0, bisec);
            _add_vertex(ctx, v);
            v.pos = vec2_sub (p0, vec2_mult (vp, hw));
        }else{
            v.pos = vec2_add (p0, vec2_mult (vp, hw));
            _add_vertex(ctx, v);
            v.pos = vec2_sub (p0, bisec);
        }
        _add_vertex(ctx, v);

        if (ctx->lineJoin == VKVG_LINE_JOIN_BEVEL){
            if (cross<0){
                _add_triangle_indices(ctx, idx, idx+2, idx+1);
                _add_triangle_indices(ctx, idx+2, idx+4, idx+0);
                _add_triangle_indices(ctx, idx, idx+3, idx+4);
            }else{
                _add_triangle_indices(ctx, idx, idx+2, idx+1);
                _add_triangle_indices(ctx, idx+2, idx+3, idx+1);
                _add_triangle_indices(ctx, idx+1, idx+3, idx+4);
            }
        }else if (ctx->lineJoin == VKVG_LINE_JOIN_ROUND){
            float step = M_PIF / hw;
            float a = acosf(vp.x);
            if (vp.y < 0)
                a = -a;

            if (cross<0){
                a+=M_PIF;
                float a1 = a + alpha*2;
                a-=step;
                while (a > a1){
                    _add_vertexf(ctx, cosf(a) * hw + p0.x, sinf(a) * hw + p0.y);
                    a-=step;
                }
            }else{
                float a1 = a + alpha*2;
                a+=step;
                while (a < a1){
                    _add_vertexf(ctx, cosf(a) * hw + p0.x, sinf(a) * hw + p0.y);
                    a+=step;
                }
            }
            VKVG_IBO_INDEX_TYPE p0Idx = ctx->vertCount - ctx->curVertOffset;
            _add_triangle_indices(ctx, idx, idx+2, idx+1);
            if (cross<0){
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

        vp = vec2_mult (vec2_perp(v1n), hw);
        if (cross<0)
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
    return cross;
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
    free(sav->selectedFont.fontFile);
    free (sav);
}


#define m_approximation_scale   1.0
#define m_angle_tolerance       0.01
#define m_distance_tolerance    1.0
#define m_cusp_limit            0.01
#define curve_recursion_limit   10
#define curve_collinearity_epsilon 1.7
#define curve_angle_tolerance_epsilon 0.001

void _recursive_bezier (VkvgContext ctx,
                        float x1, float y1, float x2, float y2,
                        float x3, float y3, float x4, float y4,
                        unsigned level) {
    if(level > curve_recursion_limit)
    {
        return;
    }

    // Calculate all the mid-points of the line segments
    //----------------------
    float x12   = (x1 + x2) / 2;
    float y12   = (y1 + y2) / 2;
    float x23   = (x2 + x3) / 2;
    float y23   = (y2 + y3) / 2;
    float x34   = (x3 + x4) / 2;
    float y34   = (y3 + y4) / 2;
    float x123  = (x12 + x23) / 2;
    float y123  = (y12 + y23) / 2;
    float x234  = (x23 + x34) / 2;
    float y234  = (y23 + y34) / 2;
    float x1234 = (x123 + x234) / 2;
    float y1234 = (y123 + y234) / 2;

    if(level > 0) // Enforce subdivision first time
    {
        // Try to approximate the full cubic curve by a single straight line
        //------------------
        float dx = x4-x1;
        float dy = y4-y1;

        float d2 = fabs(((x2 - x4) * dy - (y2 - y4) * dx));
        float d3 = fabs(((x3 - x4) * dy - (y3 - y4) * dx));

        float da1, da2;

        if(d2 > curve_collinearity_epsilon && d3 > curve_collinearity_epsilon)
        {
            // Regular care
            //-----------------
            if((d2 + d3)*(d2 + d3) <= m_distance_tolerance * (dx*dx + dy*dy))
            {
                // If the curvature doesn't exceed the distance_tolerance value
                // we tend to finish subdivisions.
                //----------------------
                if(m_angle_tolerance < curve_angle_tolerance_epsilon)
                {
                    _add_point (ctx, x1234, y1234);
                    return;
                }

                // Angle & Cusp Condition
                //----------------------
                float a23 = atan2(y3 - y2, x3 - x2);
                da1 = fabs(a23 - atan2(y2 - y1, x2 - x1));
                da2 = fabs(atan2(y4 - y3, x4 - x3) - a23);
                if(da1 >= M_PIF) da1 = M_2_PI - da1;
                if(da2 >= M_PIF) da2 = M_2_PI - da2;

                if(da1 + da2 < m_angle_tolerance)
                {
                    // Finally we can stop the recursion
                    //----------------------
                    _add_point (ctx, x1234, y1234);
                    return;
                }

                if(m_cusp_limit != 0.0)
                {
                    if(da1 > m_cusp_limit)
                    {
                        _add_point (ctx, x2, y2);
                        return;
                    }

                    if(da2 > m_cusp_limit)
                    {
                        _add_point (ctx, x3, y3);
                        return;
                    }
                }
            }
        } else {
            if(d2 > curve_collinearity_epsilon)
            {
                // p1,p3,p4 are collinear, p2 is considerable
                //----------------------
                if(d2 * d2 <= m_distance_tolerance * (dx*dx + dy*dy))
                {
                    if(m_angle_tolerance < curve_angle_tolerance_epsilon)
                    {
                        _add_point (ctx, x1234, y1234);
                        return;
                    }

                    // Angle Condition
                    //----------------------
                    da1 = fabs(atan2(y3 - y2, x3 - x2) - atan2(y2 - y1, x2 - x1));
                    if(da1 >= M_PIF) da1 = M_2_PI - da1;

                    if(da1 < m_angle_tolerance)
                    {
                        _add_point (ctx, x2, y2);
                        _add_point (ctx, x3, y3);
                        return;
                    }

                    if(m_cusp_limit != 0.0)
                    {
                        if(da1 > m_cusp_limit)
                        {
                            _add_point (ctx, x2, y2);
                            return;
                        }
                    }
                }
            } else if(d3 > curve_collinearity_epsilon) {
                // p1,p2,p4 are collinear, p3 is considerable
                //----------------------
                if(d3 * d3 <= m_distance_tolerance * (dx*dx + dy*dy))
                {
                    if(m_angle_tolerance < curve_angle_tolerance_epsilon)
                    {
                        _add_point (ctx, x1234, y1234);
                        return;
                    }

                    // Angle Condition
                    //----------------------
                    da1 = fabs(atan2(y4 - y3, x4 - x3) - atan2(y3 - y2, x3 - x2));
                    if(da1 >= M_PIF) da1 = M_2_PI - da1;

                    if(da1 < m_angle_tolerance)
                    {
                        _add_point (ctx, x2, y2);
                        _add_point (ctx, x3, y3);
                        return;
                    }

                    if(m_cusp_limit != 0.0)
                    {
                        if(da1 > m_cusp_limit)
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
                if(dx*dx + dy*dy <= m_distance_tolerance)
                {
                    _add_point (ctx, x1234, y1234);
                    return;
                }
            }
        }
    }

    // Continue subdivision
    //----------------------
    _recursive_bezier (ctx, x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
    _recursive_bezier (ctx, x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);
}
void _poly_fill (VkvgContext ctx){
    //we anticipate the check for vbo buffer size
    if (ctx->vertCount + ctx->pointCount > ctx->sizeVBO) {
        if (ctx->cmdStarted) {
            _end_render_pass(ctx);
            _flush_vertices_caches(ctx);
            vkh_cmd_end(ctx->cmd);
            _wait_and_submit_cmd(ctx);
            if (ctx->vertCount + ctx->pointCount > ctx->sizeVBO)
                _resize_vbo(ctx, ctx->vertCount + ctx->pointCount);
        }else
            _resize_vbo(ctx, ctx->vertCount + ctx->pointCount);

        _start_cmd_for_render_pass(ctx);
    }else
        _check_cmd_buff_state(ctx);

    CmdBindPipeline (ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelinePolyFill);

    uint32_t ptrPath = 0;
    Vertex v = {{0},{0,0,-1}};

    while (ptrPath < ctx->pathPtr){
        if (ctx->pathes[ptrPath+1]&PATH_IS_CURVE_BIT){
            ptrPath += 2;
            continue;
        }
        //close path
        ctx->pathes[ptrPath] |= PATH_CLOSED_BIT;// ctx->pathes[ptrPath];//close path by setting start and end equal

        uint32_t firstPtIdx = ctx->pathes [ptrPath] & PATH_ELT_MASK;
        uint32_t lastPtIdx  = ctx->pathes [ptrPath+1] & PATH_ELT_MASK;//_get_last_point_of_closed_path (ctx, ptrPath);
        uint32_t pathPointCount = lastPtIdx - firstPtIdx + 1;

        VKVG_IBO_INDEX_TYPE firstVertIdx = ctx->vertCount;

        for (uint32_t i = 0; i < pathPointCount; i++) {
            v.pos = ctx->points [i+firstPtIdx];
            ctx->vertexCache[ctx->vertCount] = v;
            ctx->vertCount++;
        }

        LOG(LOG_INFO_PATH, "\tpoly fill: point count = %d; 1st vert = %d; vert count = %d\n", pathPointCount, firstVertIdx, ctx->vertCount - firstVertIdx);
        CmdDraw (ctx->cmd, pathPointCount, 1, firstVertIdx ,0);

        ptrPath+=2;
    }
    ctx->curVertOffset = ctx->vertCount;
}
void _fill_ec (VkvgContext ctx){
    uint32_t ptrPath = 0;;
    Vertex v = {0};
    v.uv.z = -1;

    while (ptrPath < ctx->pathPtr){
        if (ctx->pathes[ptrPath+1]&PATH_IS_CURVE_BIT){
            ptrPath += 2;
            continue;
        }
        ctx->pathes[ptrPath]|=PATH_CLOSED_BIT;//close path

        uint32_t firstPtIdx = ctx->pathes[ptrPath]&PATH_ELT_MASK;
        uint32_t lastPtIdx = ctx->pathes[ptrPath+1]&PATH_ELT_MASK;
        uint32_t pathPointCount = lastPtIdx - firstPtIdx + 1;
        uint32_t firstVertIdx = ctx->vertCount-ctx->curVertOffset;

        ear_clip_point* ecps = (ear_clip_point*)malloc(pathPointCount*sizeof(ear_clip_point));
        uint32_t ecps_count = pathPointCount;
        uint32_t i = 0;

        //init points link list
        while (i < pathPointCount-1){
            v.pos = ctx->points[i+firstPtIdx];
            ear_clip_point ecp = {v.pos, i+firstVertIdx, &ecps[i+1]};
            ecps[i] = ecp;
            _add_vertex(ctx, v);
            i++;
        }

        v.pos = ctx->points[i+firstPtIdx];
        ear_clip_point ecp = {v.pos, i+firstVertIdx, ecps};
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

        ptrPath+=2;
        free (ecps);
    }
    _record_draw_cmd(ctx);
}

static const uint32_t one = 1;
static const uint32_t zero = 0;
void _draw_full_screen_quad (VkvgContext ctx, bool useScissor) {
    if (ctx->xMin < 0 || ctx->yMin < 0)
        useScissor = false;
    if (useScissor && ctx->xMin < FLT_MAX) {
        VkRect2D r = {{ctx->xMin, ctx->yMin}, {ctx->xMax - ctx->xMin, ctx->yMax - ctx->yMin}};
        CmdSetScissor(ctx->cmd, 0, 1, &r);
    }
    CmdPushConstants(ctx->cmd, ctx->pSurf->dev->pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT, 28, 4,&one);
    CmdDraw (ctx->cmd,3,1,0,0);
    CmdPushConstants(ctx->cmd, ctx->pSurf->dev->pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT, 28, 4,&zero);
    if (useScissor && ctx->xMin < FLT_MAX)
        CmdSetScissor(ctx->cmd, 0, 1, &ctx->bounds);
}

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

void _check_pathes_array (VkvgContext ctx){
    if (ctx->sizePathes - ctx->pathPtr > VKVG_ARRAY_THRESHOLD)
        return;
    ctx->sizePathes += VKVG_PATHES_SIZE;
    ctx->pathes = (uint32_t*) realloc (ctx->pathes, ctx->sizePathes*sizeof(uint32_t));
}
//when empty, ptr is even, else it's odd
//when empty, no current point is defined.
inline bool _current_path_is_empty (VkvgContext ctx) {
    return ctx->pathPtr % 2 == 0;
}
//this function expect that current point exists
inline vec2 _get_current_position (VkvgContext ctx) {
    return ctx->points[ctx->pointCount-1];
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
    ctx->pathPtr++;
}
void _clear_path (VkvgContext ctx){
    ctx->pathPtr = 0;
    ctx->pointCount = 0;
}
inline bool _path_is_closed (VkvgContext ctx, uint32_t ptrPath){
    return (ctx->pathes[ptrPath] == ctx->pathes[ptrPath+1]);
}
uint32_t _get_last_point_of_closed_path(VkvgContext ctx, uint32_t ptrPath){
    if (ptrPath+2 < ctx->pathPtr)			//this is not the last path
        return ctx->pathes[ptrPath+2]-1;    //last p is p prior to first idx of next path
    return ctx->pointCount-1;				//last point of path is last point of point array
}
void _add_point (VkvgContext ctx, float x, float y){
    vec2 v = {x,y};
    ctx->points[ctx->pointCount] = v;
    ctx->pointCount++;
}
void _add_point_vec2(VkvgContext ctx, vec2 v){
    ctx->points[ctx->pointCount] = v;
    ctx->pointCount++;
}
float _normalizeAngle(float a)
{
    float res = ROUND_DOWN(fmod(a,2.0f*M_PI),100);
    if (res < 0.0f)
        return res + 2.0f*M_PI;
    else
        return res;
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
        ctx->sizeVertices * sizeof(Vertex), &ctx->vertices);
    vkvg_buffer_create (ctx->pSurf->dev,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        ctx->sizeIndices * sizeof(uint32_t), &ctx->indices);
}
const vec3 blankuv = {};
void _add_vertexf (VkvgContext ctx, float x, float y){
    Vertex* pVert = (Vertex*)(ctx->vertices.allocInfo.pMappedData + ctx->vertCount * sizeof(Vertex));
    pVert->pos.x = x;
    pVert->pos.y = y;
    pVert->uv = blankuv;
    ctx->vertCount++;
}
void _add_vertex(VkvgContext ctx, Vertex v){
    Vertex* pVert = (Vertex*)(ctx->vertices.allocInfo.pMappedData + ctx->vertCount * sizeof(Vertex));
    *pVert = v;
    ctx->vertCount++;
}
void _set_vertex(VkvgContext ctx, uint32_t idx, Vertex v){
    Vertex* pVert = (Vertex*)(ctx->vertices.allocInfo.pMappedData + idx * sizeof(Vertex));
    *pVert = v;
}
void _add_tri_indices_for_rect (VkvgContext ctx, uint32_t i){
    uint32_t* inds = (uint32_t*)(ctx->indices.allocInfo.pMappedData + (ctx->indCount * sizeof(uint32_t)));
    inds[0] = i;
    inds[1] = i+2;
    inds[2] = i+1;
    inds[3] = i+1;
    inds[4] = i+2;
    inds[5] = i+3;
    ctx->indCount+=6;
}
void _add_triangle_indices(VkvgContext ctx, uint32_t i0, uint32_t i1, uint32_t i2){
    uint32_t* inds = (uint32_t*)(ctx->indices.allocInfo.pMappedData + (ctx->indCount * sizeof(uint32_t)));
    inds[0] = i0;
    inds[1] = i1;
    inds[2] = i2;
    ctx->indCount+=3;
}
void _vao_add_rectangle (VkvgContext ctx, float x, float y, float width, float height){
    Vertex v[4] =
    {
        {{x,y},             {0,0,-1}},
        {{x,y+height},      {0,0,-1}},
        {{x+width,y},       {0,0,-1}},
        {{x+width,y+height},{0,0,-1}}
    };
    uint32_t firstIdx = ctx->vertCount;
    Vertex* pVert = (Vertex*)(ctx->vertices.allocInfo.pMappedData + ctx->vertCount * sizeof(Vertex));
    memcpy (pVert,v,4*sizeof(Vertex));
    ctx->vertCount+=4;
    _add_tri_indices_for_rect(ctx, firstIdx);
}
void _create_cmd_buff (VkvgContext ctx){
    ctx->cmd = vkh_cmd_buff_create(ctx->pSurf->dev, ctx->cmdPool,VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}
void _record_draw_cmd (VkvgContext ctx){
    if (ctx->indCount == ctx->curIndStart)
        return;
    vkCmdDrawIndexed(ctx->cmd, ctx->indCount - ctx->curIndStart, 1, ctx->curIndStart, 0, 1);

    //DEBUG
    /*vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineWired);
    vkCmdDrawIndexed(ctx->cmd, ctx->indCount - ctx->curIndStart, 1, ctx->curIndStart, 0, 1);
    vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipeline);*/
    //////////

    ctx->curIndStart = ctx->indCount;
}

inline void _submit_ctx_cmd(VkvgContext ctx){
    vkh_cmd_submit (ctx->pSurf->dev->gQueue, &ctx->cmd, ctx->flushFence);
}
void _wait_and_reset_ctx_cmd (VkvgContext ctx){
    vkWaitForFences(ctx->pSurf->dev->vkDev,1,&ctx->flushFence,VK_TRUE,UINT64_MAX);
    vkResetFences(ctx->pSurf->dev->vkDev,1,&ctx->flushFence);
    vkResetCommandBuffer(ctx->cmd,0);
}

inline void _submit_wait_and_reset_cmd (VkvgContext ctx){
    _submit_ctx_cmd(ctx);
    _wait_and_reset_ctx_cmd(ctx);
}
void _explicit_ms_resolve (VkvgContext ctx){
    vkh_image_set_layout (ctx->cmd, ctx->pSurf->imgMS, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    vkh_image_set_layout (ctx->cmd, ctx->pSurf->img, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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
    vkh_image_set_layout (ctx->cmd, ctx->pSurf->imgMS, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

void _flush_cmd_buff (VkvgContext ctx){
    _record_draw_cmd        (ctx);
    vkCmdEndRenderPass      (ctx->cmd);
    //_explicit_ms_resolve    (ctx);
    vkh_cmd_end             (ctx->cmd);

    _submit_wait_and_reset_cmd(ctx);
}
//bind correct draw pipeline depending on current OPERATOR
void _bind_draw_pipeline (VkvgContext ctx) {
    switch (ctx->curOperator) {
    case VKVG_OPERATOR_OVER:
        vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipe_OVER);
        break;
    case VKVG_OPERATOR_CLEAR:
        vkvg_set_source_rgba(ctx,0,0,0,0);
        vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipe_CLEAR);
        break;
    default:
        vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipe_OVER);
        break;
    }
}
void _init_cmd_buff (VkvgContext ctx){
    //full surf quad triangles is at the beginning
    ctx->vertCount = 4;
    ctx->indCount = 6;
    ctx->curIndStart = 6;
    //VkClearValue clearValues[2];
    //clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    //clearValues[1].depthStencil = { 1.0f, 0 };
    VkClearValue clearValues[4] = {
        { 0.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, 0 },
        { 1.0f, 0 }
    };
    VkRenderPassBeginInfo renderPassBeginInfo = { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                                  .renderPass = ctx->pSurf->dev->renderPass,
                                                  .framebuffer = ctx->pSurf->fb,
                                                  .renderArea.extent = {ctx->pSurf->width,ctx->pSurf->height}};
                                                  //.clearValueCount = 4,
                                                  //.pClearValues = clearValues};

    vkh_cmd_begin (ctx->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkCmdBeginRenderPass (ctx->cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    VkViewport viewport = {0,0,ctx->pSurf->width,ctx->pSurf->height,0,1};
    vkCmdSetViewport(ctx->cmd, 0, 1, &viewport);
    VkRect2D scissor = {{0,0},{ctx->pSurf->width,ctx->pSurf->height}};
    vkCmdSetScissor(ctx->cmd, 0, 1, &scissor);

    VkDescriptorSet dss[] = {ctx->dsFont,ctx->dsSrc,ctx->dsGrad};
    vkCmdBindDescriptorSets(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineLayout,
                            0, 3, dss, 0, NULL);
    VkDeviceSize offsets[1] = { 0 };
    vkCmdBindVertexBuffers(ctx->cmd, 0, 1, &ctx->vertices.buffer, offsets);
    vkCmdBindIndexBuffer(ctx->cmd, ctx->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

    _bind_draw_pipeline (ctx);
    vkCmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);

    _update_push_constants  (ctx);
}
//compute inverse mat used in shader when context matrix has changed
//then trigger push constants command
void _set_mat_inv_and_vkCmdPush (VkvgContext ctx) {
    ctx->pushConsts.matInv = ctx->pushConsts.mat;
    vkvg_matrix_invert (&ctx->pushConsts.matInv);
    _update_push_constants (ctx);
}
inline void _update_push_constants (VkvgContext ctx) {
    vkCmdPushConstants(ctx->cmd, ctx->pSurf->dev->pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constants),&ctx->pushConsts);
}
void _update_cur_pattern (VkvgContext ctx, VkvgPattern pat) {
    VkvgPattern lastPat = ctx->pattern;
    ctx->pattern = pat;

    ctx->pushConsts.patternType = pat->type;

    switch (pat->type)  {
    case VKVG_PATTERN_TYPE_SOLID:
        memcpy (&ctx->pushConsts.source, ctx->pattern->data, sizeof(vkvg_color_t));

        if (lastPat && lastPat->type == VKVG_PATTERN_TYPE_SURFACE){
            _flush_cmd_buff             (ctx);
            _reset_src_descriptor_set   (ctx);
            _init_cmd_buff              (ctx);//push csts updated by init
        }else
            _update_push_constants (ctx);

        break;
    case VKVG_PATTERN_TYPE_SURFACE:
        _flush_cmd_buff(ctx);

        VkvgSurface surf = (VkvgSurface)pat->data;
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
        if (vkh_image_get_layout (ctx->source) != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
            vkh_cmd_begin (ctx->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

            vkh_image_set_layout        (ctx->cmd, ctx->source, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            vkh_cmd_end                 (ctx->cmd);

            _submit_wait_and_reset_cmd  (ctx);
        }

        _update_descriptor_set          (ctx, ctx->source, ctx->dsSrc);

        vec4 srcRect = {0,0,surf->width,surf->height};
        ctx->pushConsts.source = srcRect;

        _init_cmd_buff                  (ctx);
        break;
    case VKVG_PATTERN_TYPE_LINEAR:
        _flush_cmd_buff (ctx);

        if (lastPat && lastPat->type == VKVG_PATTERN_TYPE_SURFACE)
            _reset_src_descriptor_set (ctx);

        vec4 bounds = {ctx->pSurf->width, ctx->pSurf->height, 0, 0};//store img bounds in unused source field
        ctx->pushConsts.source = bounds;

        //transform control point with current ctx matrix
        vkvg_gradient_t grad = {};
        memcpy(&grad, pat->data, sizeof(vkvg_gradient_t));

        vkvg_matrix_transform_point(&ctx->pushConsts.mat, &grad.cp[0].x, &grad.cp[0].y);
        vkvg_matrix_transform_point(&ctx->pushConsts.mat, &grad.cp[1].x, &grad.cp[1].y);
        //to do, scale radial radiuses in cp[2]

        memcpy(ctx->uboGrad.allocInfo.pMappedData , &grad, sizeof(vkvg_gradient_t));

        _init_cmd_buff (ctx);
        break;
    }

    if (lastPat)
        vkvg_pattern_destroy    (lastPat);
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
void _reset_src_descriptor_set (VkvgContext ctx){
    VkvgDevice dev = ctx->pSurf->dev;
    //VkDescriptorSet dss[] = {ctx->dsSrc};
    vkFreeDescriptorSets    (dev->vkDev, ctx->descriptorPool, 1, &ctx->dsSrc);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                                              .descriptorPool = ctx->descriptorPool,
                                                              .descriptorSetCount = 1,
                                                              .pSetLayouts = &dev->dslSrc };
    VK_CHECK_RESULT(vkAllocateDescriptorSets(dev->vkDev, &descriptorSetAllocateInfo, &ctx->dsSrc));
}

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
void add_line(vkvg_context* ctx, vec2 p1, vec2 p2, vec4 col){
    Vertex v = {{p1.x,p1.y},{0,0,-1}};
    _add_vertex(ctx, v);
    v.pos = p2;
    _add_vertex(ctx, v);
    uint32_t* inds = (uint32_t*)(ctx->indices.allocInfo.pMappedData  + (ctx->indCount * sizeof(uint32_t)));
    inds[0] = ctx->vertCount - 2;
    inds[1] = ctx->vertCount - 1;
    ctx->indCount+=2;
}

void _build_vb_step (vkvg_context* ctx, Vertex v, float hw, uint32_t iL, uint32_t i, uint32_t iR){
    vec2 v0n = vec2_line_norm(ctx->points[iL], ctx->points[i]);
    vec2 v1n = vec2_line_norm(ctx->points[i], ctx->points[iR]);

    vec2 bisec = vec2_norm(vec2_add(v0n,v1n));

    float alpha = acos(v0n.x * v1n.x + v0n.y * v1n.y)/2;
    float cross = v0n.x * v1n.y - v0n.y * v1n.x;

    if (cross<0)
        alpha = -alpha;

    float lh = hw / cos(alpha);
    bisec = vec2_perp(bisec);
    bisec = vec2_mult(bisec,lh);

    uint32_t idx = ctx->vertCount;

    if (ctx->lineJoin == VKVG_LINE_JOIN_MITER){
        v.pos = vec2_add(ctx->points[i], bisec);
        _add_vertex(ctx, v);
        v.pos = vec2_sub(ctx->points[i], bisec);
        _add_vertex(ctx, v);
        _add_tri_indices_for_rect(ctx, idx);
    }else{
        vec2 vp = vec2_perp(v0n);
        if (cross<0){
            v.pos = vec2_sub (ctx->points[i], vec2_mult (vp, hw));
            _add_vertex(ctx, v);
            v.pos = vec2_add (ctx->points[i], bisec);
        }else{
            v.pos = vec2_add (ctx->points[i], vec2_mult (vp, hw));
            _add_vertex(ctx, v);
            v.pos = vec2_sub (ctx->points[i], bisec);
        }
        _add_vertex(ctx, v);

        if (ctx->lineJoin == VKVG_LINE_JOIN_BEVEL){
            _add_triangle_indices(ctx, idx, idx+2, idx+1);
            _add_triangle_indices(ctx, idx+2, idx+3, idx+1);
            _add_triangle_indices(ctx, idx+1, idx+3, idx+4);
        }else if (ctx->lineJoin == VKVG_LINE_JOIN_ROUND){
            float step = M_PI / hw;
            float a = acos(vp.x);
            if (vp.y < 0)
                a = -a;

            if (cross<0){
                a+=M_PI;
                float a1 = a + alpha*2;
                a-=step;
                while (a > a1){
                    _add_vertexf(ctx, cos(a) * hw + ctx->points[i].x, sin(a) * hw + ctx->points[i].y);
                    a-=step;
                }
            }else{
                float a1 = a + alpha*2;
                a+=step;
                while (a < a1){
                    _add_vertexf(ctx, cos(a) * hw + ctx->points[i].x, sin(a) * hw + ctx->points[i].y);
                    a+=step;
                }
            }
            uint32_t p0Idx = ctx->vertCount;
            _add_triangle_indices(ctx, idx, idx+2, idx+1);
            for (int p = idx+2; p < p0Idx; p++)
                _add_triangle_indices(ctx, p, p+1, idx+1);

            _add_triangle_indices(ctx, p0Idx, p0Idx+1, idx+1);
            _add_triangle_indices(ctx, idx+1, p0Idx+1, p0Idx+2);
        }

        vp = vec2_mult (vec2_perp(v1n), hw);
        if (cross<0)
            v.pos = vec2_sub (ctx->points[i], vp);
        else
            v.pos = vec2_add (ctx->points[i], vp);
        _add_vertex(ctx, v);

    }

/*
#ifdef DEBUG

    debugLinePoints[dlpCount] = ctx->points[i];
    debugLinePoints[dlpCount+1] = _v2add(ctx->points[i], _vec2dToVec2(_v2Multd(v0n,10)));
    dlpCount+=2;
    debugLinePoints[dlpCount] = ctx->points[i];
    debugLinePoints[dlpCount+1] = _v2add(ctx->points[i], _vec2dToVec2(_v2Multd(v1n,10)));
    dlpCount+=2;
    debugLinePoints[dlpCount] = ctx->points[i];
    debugLinePoints[dlpCount+1] = ctx->points[iR];
    dlpCount+=2;
#endif*/
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
    free(sav->pathes);
    free(sav->points);
    free(sav->selectedFont.fontFile);
    vkh_image_destroy   (sav->stencilMS);
    free (sav);
}


#define m_approximation_scale   1.0
#define m_angle_tolerance       0.05
#define m_distance_tolerance    0.1
#define m_cusp_limit            0.25
#define curve_recursion_limit   16
#define curve_collinearity_epsilon 0.001
#define curve_angle_tolerance_epsilon 0.1

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
                if(da1 >= M_PI) da1 = M_2_PI - da1;
                if(da2 >= M_PI) da2 = M_2_PI - da2;

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
                    if(da1 >= M_PI) da1 = M_2_PI - da1;

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
                    if(da1 >= M_PI) da1 = M_2_PI - da1;

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
    _recursive_bezier(ctx, x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
    _recursive_bezier(ctx, x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);
}


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

#include "vkvg_device_internal.h"
#include "vkvg_context_internal.h"
#include "vkvg_surface_internal.h"
#include "vkvg_pattern.h"
#include "vkh_queue.h"

#include "nanosvg.h"

#ifdef DEBUG
static vec2 debugLinePoints[1000];
static uint32_t dlpCount = 0;
#endif

static VkClearValue clearValues[3] = {
    { 0.0f, 0.0f, 0.0f, 0.0f },
    { 1.0f, 0 },
    { 0.0f, 0.0f, 0.0f, 0.0f }
};

/**
 * @brief create new context for surface
 * @param drawing operation output surface
 * @return newly created context pointer
 */
VkvgContext vkvg_create(VkvgSurface surf)
{
    VkvgDevice dev = surf->dev;
    VkvgContext ctx = (vkvg_context*)calloc(1, sizeof(vkvg_context));

    LOG(LOG_INFO, "CREATE Context: ctx = %lu; surf = %lu\n", (ulong)ctx, (ulong)surf);

    if (ctx==NULL) {
        dev->status = VKVG_STATUS_NO_MEMORY;
        return NULL;
    }

    ctx->sizePoints     = VKVG_PTS_SIZE;
    ctx->sizeVertices   = VKVG_VBO_SIZE;
    ctx->sizeIndices    = VKVG_IBO_SIZE;
    ctx->sizePathes     = VKVG_PATHES_SIZE;
    ctx->lineWidth      = 1;
    ctx->pSurf          = surf;
    ctx->curOperator    = VKVG_OPERATOR_OVER;
    ctx->curFillRule    = VKVG_FILL_RULE_NON_ZERO;
    ctx->curSavBit      = 0;
    ctx->vertCount      = 0;
    ctx->indCount       = 0;
    ctx->curIndStart    = 0;

    VkRect2D scissor = {{0,0},{ctx->pSurf->width,ctx->pSurf->height}};
    ctx->bounds        = scissor;

    ctx->savedStencils = malloc(0);

    push_constants pc = {
            {.height=1},
            {(float)ctx->pSurf->width,(float)ctx->pSurf->height},
            VKVG_PATTERN_TYPE_SOLID,
            0,
            VKVG_IDENTITY_MATRIX,
            VKVG_IDENTITY_MATRIX
    };
    ctx->pushConsts = pc;

    const VkClearRect cr = {{{0},{ctx->pSurf->width, ctx->pSurf->height}},0,1};
    ctx->clearRect = cr;
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

    ctx->selectedFont.fontFile = (char*)calloc(FONT_FILE_NAME_MAX_SIZE,sizeof(char));
    ctx->currentFont           = NULL;

    ctx->flushFence = vkh_fence_create_signaled ((VkhDevice)dev);

    ctx->points = (vec2*)       malloc (VKVG_VBO_SIZE*sizeof(vec2));
    ctx->pathes = (uint32_t*)   malloc (VKVG_PATHES_SIZE*sizeof(uint32_t));

    //for context to be thread safe, command pool and descriptor pool have to be created in the thread of the context.
    ctx->cmdPool = vkh_cmd_pool_create ((VkhDevice)dev, dev->gQueue->familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    _create_vertices_buff   (ctx);
    _create_gradient_buff   (ctx);
    _create_cmd_buff        (ctx);
    _createDescriptorPool   (ctx);
    _init_descriptor_sets   (ctx);
    _update_descriptor_set  (ctx, ctx->pSurf->dev->fontCache->texture, ctx->dsFont);
    _update_descriptor_set  (ctx, surf->dev->emptyImg, ctx->dsSrc);
    _update_gradient_desc_set(ctx);

    _clear_path             (ctx);

    ctx->cmd = ctx->cmdBuffers[0];//current recording buffer

    ctx->references = 1;
    ctx->status = VKVG_STATUS_SUCCESS;
    return ctx;
}
/**
 * @brief explicitly flush pending drawing operations on context
 * @param context pointer to flush
 */
void vkvg_flush (VkvgContext ctx){
    _flush_cmd_buff(ctx);
    //_wait_flush_fence(ctx);
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

/**
 * @brief decrement reference count on context and release ressources if reference equal 0.
 * @param context to destroy
 */
void vkvg_destroy (VkvgContext ctx)
{
    ctx->references--;
    if (ctx->references > 0)
        return;

    _flush_cmd_buff(ctx);

    vkWaitForFences (ctx->pSurf->dev->vkDev, 1, &ctx->flushFence, VK_TRUE, VKVG_FENCE_TIMEOUT);

    LOG(LOG_INFO, "DESTROY Context: ctx = %lu; surf = %lu\n", (ulong)ctx, (ulong)ctx->pSurf);

    if (ctx->pattern)
        vkvg_pattern_destroy (ctx->pattern);

    VkDevice dev = ctx->pSurf->dev->vkDev;

    vkDestroyFence      (dev, ctx->flushFence,NULL);
    vkFreeCommandBuffers(dev, ctx->cmdPool, 2, ctx->cmdBuffers);
    vkDestroyCommandPool(dev, ctx->cmdPool, NULL);

    VkDescriptorSet dss[] = {ctx->dsFont, ctx->dsSrc, ctx->dsGrad};
    vkFreeDescriptorSets    (dev, ctx->descriptorPool, 3, dss);

    vkDestroyDescriptorPool (dev, ctx->descriptorPool,NULL);

    vkvg_buffer_destroy (&ctx->uboGrad);
    vkvg_buffer_destroy (&ctx->indices);
    vkvg_buffer_destroy (&ctx->vertices);

    free(ctx->vertexCache);
    free(ctx->indexCache);

    //vkh_image_destroy   (ctx->source);

    free(ctx->selectedFont.fontFile);
    free(ctx->pathes);
    free(ctx->points);

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
/**
 * @brief increment reference count on context
 * @param context pointer
 * @return
 */
VkvgContext vkvg_reference (VkvgContext ctx) {
    ctx->references++;
    return ctx;
}
/**
 * @brief get current reference count for context
 * @param context pointer
 * @return
 */
uint32_t vkvg_get_reference_count (VkvgContext ctx) {
    return ctx->references;
}
/**
 * @brief Start new sub path, no current point is defined
 * @param context pointer
 */
void vkvg_new_sub_path (VkvgContext ctx){
    _finish_path(ctx);
}
/**
 * @brief clear current context path without drawing anything
 * @param context pointer
 */
void vkvg_new_path (VkvgContext ctx){
    _clear_path(ctx);
}
//path closing is done by setting the endpoint of the path to the same index
//as the start point.
//I'll test if closing by adding a new point with the same x,y as the start point
//would not make more sense.
/**
 * @brief Close current path if at least 3 points are present
 * @param context pointer
 */
void vkvg_close_path (VkvgContext ctx){
    if (_current_path_is_empty(ctx)){
        ctx->status = VKVG_STATUS_NO_CURRENT_POINT;
        return;
    }
    //check if at least 3 points are present
    if (ctx->pointCount - (ctx->pathes [ctx->pathPtr-1]&PATH_ELT_MASK) > 2){
        ctx->pathes[ctx->pathPtr] = ctx->pointCount - 1;
        ctx->pathes[ctx->pathPtr-1] |= PATH_CLOSED_BIT;
        _check_pathes_array(ctx);
        ctx->pathPtr += ctx->curvePtr + 1;
        ctx->curvePtr = 0;
    }else
        _finish_path(ctx);
}
/**
 * @brief draw line with second point coordinates relative to current point
 * @param context pointer
 * @param delta x
 * @param delta y
 */
void vkvg_rel_line_to (VkvgContext ctx, float x, float y){
    if (_current_path_is_empty(ctx)){
        ctx->status = VKVG_STATUS_NO_CURRENT_POINT;
        return;
    }
    vec2 cp = _get_current_position(ctx);
    vkvg_line_to(ctx, cp.x + x, cp.y + y);
}
/**
 * @brief Draw line from current point, if no current point is defined, only a move to will be executed.
 * @param context pointer
 * @param absolute x coordinate of second point
 * @param aboslute y coordinate of second point
 */
void vkvg_line_to (VkvgContext ctx, float x, float y)
{
    vec2 p = {x,y};
    if (_current_path_is_empty(ctx)){
        vkvg_move_to(ctx, x,y);
        return;
    }else if (vec2_equ(_get_current_position(ctx),p))
        return;

    _add_point(ctx,x,y);
}
/**
 * @brief Draw arc
 * @param context pointer
 * @param center x coordinate
 * @param center y coordinate
 * @param radius
 * @param start angle of arc
 * @param end angle of arc
 */
void vkvg_arc (VkvgContext ctx, float xc, float yc, float radius, float a1, float a2){
    while (a2 < a1)//positive arc must have a1<a2
        a2 += 2.f*M_PIF;

    if (a2 - a1 > 2.f * M_PIF) //limit arc to 2PI
        a2 = a1 + 2.f * M_PIF;

    vec2 v = {cosf(a1)*radius + xc, sinf(a1)*radius + yc};

    float step = M_PIF/radius*0.5f;
    float a = a1;

    if (_current_path_is_empty(ctx))
        vkvg_move_to(ctx, v.x, v.y);
    else
        vkvg_line_to(ctx, v.x, v.y);

    a+=step;

    if (EQUF(a2, a1))
        return;

    _set_curve_start (ctx);

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
    vec2 lastP = v;
    v.x = cosf(a)*radius + xc;
    v.y = sinf(a)*radius + yc;
    //if (!vec2_equ (v,lastP))//this test should not be required
        _add_point (ctx, v.x, v.y);
    _set_curve_end(ctx);
}
void vkvg_arc_negative (VkvgContext ctx, float xc, float yc, float radius, float a1, float a2) {
    while (a2 > a1)
        a2 -= 2.f*M_PIF;
    if (a1 - a2 > a1 + 2.f * M_PIF) //limit arc to 2PI
        a2 = a1 - 2.f * M_PIF;

    vec2 v = {cosf(a1)*radius + xc, sinf(a1)*radius + yc};

    float step = M_PIF/radius*1.5f;
    float a = a1;

    if (_current_path_is_empty(ctx))
        vkvg_move_to(ctx, v.x, v.y);
    else {
        vkvg_line_to(ctx, v.x, v.y);
    }

    a-=step;

    if (EQUF(a2, a1))
        return;

    _set_curve_start (ctx);

    while(a > a2){
        v.x = cosf(a)*radius + xc;
        v.y = sinf(a)*radius + yc;
        _add_point (ctx,v.x,v.y);
        a-=step;
    }

    if (EQUF(a1-a2,M_PIF*2.f))//if arc is complete circle, last point is the same as the first one
        return;

    a = a2;
    vec2 lastP = v;
    v.x = cosf(a)*radius + xc;
    v.y = sinf(a)*radius + yc;
    //if (!vec2_equ (v,lastP))
        _add_point (ctx, v.x, v.y);
    _set_curve_end(ctx);
}
void vkvg_rel_move_to (VkvgContext ctx, float x, float y)
{
    if (_current_path_is_empty(ctx)){
        ctx->status = VKVG_STATUS_NO_CURRENT_POINT;
        return;
    }
    vec2 cp = _get_current_position(ctx);
    vkvg_move_to(ctx, cp.x + x, cp.y + y);
}
void vkvg_move_to (VkvgContext ctx, float x, float y)
{
    _finish_path(ctx);
    _start_sub_path(ctx, x, y);
}
void vkvg_curve_to (VkvgContext ctx, float x1, float y1, float x2, float y2, float x3, float y3) {
    if (_current_path_is_empty(ctx))
        vkvg_move_to(ctx, x1, y1);

    vec2 cp = _get_current_position(ctx);

    _set_curve_start (ctx);
    _recursive_bezier (ctx, cp.x, cp.y, x1, y1, x2, y2, x3, y3, 0);
    /*cp.x = x3;
    cp.y = y3;
    if (!vec2_equ(ctx->points[ctx->pointCount-1],cp))*/
    _add_point(ctx,x3,y3);
    _set_curve_end (ctx);
}
void vkvg_rel_curve_to (VkvgContext ctx, float x1, float y1, float x2, float y2, float x3, float y3) {
    if (_current_path_is_empty(ctx)){
        ctx->status = VKVG_STATUS_NO_CURRENT_POINT;
        return;
    }
    vec2 cp = _get_current_position(ctx);
    vkvg_curve_to (ctx, cp.x + x1, cp.y + y1, cp.x + x2, cp.y + y2, cp.x + x3, cp.y + y3);
}

void vkvg_rectangle (VkvgContext ctx, float x, float y, float w, float h){
    _finish_path (ctx);

    _start_sub_path(ctx, x, y);
    _add_point (ctx, x + w, y);
    _add_point (ctx, x + w, y + h);
    _add_point (ctx, x, y + h);

    vkvg_close_path (ctx);
}
static const VkClearAttachment clearStencil        = {VK_IMAGE_ASPECT_STENCIL_BIT, 1, {0}};
static const VkClearAttachment clearColorAttach    = {VK_IMAGE_ASPECT_COLOR_BIT,   0, {0}};

void vkvg_reset_clip (VkvgContext ctx){
    _check_cmd_buff_state (ctx);
    vkCmdClearAttachments(ctx->cmd, 1, &clearStencil, 1, &ctx->clearRect);
}
void vkvg_clear (VkvgContext ctx){
    _check_cmd_buff_state (ctx);
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
    if (ctx->pathPtr == 0)      //nothing to fill
        return;
    _finish_path(ctx);

    LOG(LOG_INFO, "CLIP: ctx = %lu; path cpt = %d;\n", ctx, ctx->pathPtr / 2);

    if (ctx->pointCount * 4 > ctx->sizeIndices - ctx->indCount)//flush if vk buff is full
        _flush_cmd_buff(ctx);

    if (ctx->curFillRule == VKVG_FILL_RULE_EVEN_ODD){
        _check_cmd_buff_state(ctx);
        _poly_fill (ctx);
        CmdBindPipeline         (ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineClipping);
        CmdSetStencilReference  (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
        CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_FILL_BIT);
        CmdSetStencilWriteMask  (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_ALL_BIT);
    }else{
        _check_cmd_buff_state(ctx);
        CmdBindPipeline         (ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineClipping);
        CmdSetStencilReference  (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_FILL_BIT);
        CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
        CmdSetStencilWriteMask  (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_FILL_BIT);
        _fill_ec(ctx);
        CmdSetStencilReference  (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
        CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_FILL_BIT);
        CmdSetStencilWriteMask  (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_ALL_BIT);
    }
    _draw_full_screen_quad (ctx, false);
    //should test current operator to bind correct pipeline
    _bind_draw_pipeline (ctx);
    CmdSetStencilCompareMask (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
}
void vkvg_fill_preserve (VkvgContext ctx){
    if (ctx->pathPtr == 0)      //nothing to fill
        return;
    _finish_path(ctx);

    LOG(LOG_INFO, "FILL: ctx = %lu; path cpt = %d;\n", ctx, ctx->pathPtr / 2);

    if (ctx->pointCount * 4 > ctx->sizeIndices - ctx->indCount)//flush if vk buff is full
        _flush_cmd_buff(ctx);

    _check_cmd_buff_state(ctx);

    if (ctx->curFillRule == VKVG_FILL_RULE_EVEN_ODD){
        _poly_fill (ctx);
        _bind_draw_pipeline (ctx);
        CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_FILL_BIT);
        _draw_full_screen_quad(ctx,true);
        CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
    }else{
        CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
        _fill_ec(ctx);
    }
}
void vkvg_stroke_preserve (VkvgContext ctx)
{
    if (ctx->pathPtr == 0)//nothing to stroke
        return;
    _finish_path(ctx);

    LOG(LOG_INFO, "STROKE: ctx = %lu; path cpt = %d;\n", ctx, ctx->pathPtr / 2);

    if (ctx->pointCount * 4 > ctx->sizeIndices - ctx->indCount)
        _flush_cmd_buff(ctx);

    Vertex v = {};
    v.uv.z = -1;

    float hw = ctx->lineWidth / 2.0f;
    uint i = 0, ptrPath = 0;

    uint32_t lastPathPointIdx, iL, iR;

    while (ptrPath < ctx->pathPtr){
        uint ptrCurve = 0;
        uint32_t firstIdx = ctx->vertCount;
        i = ctx->pathes[ptrPath]&PATH_ELT_MASK;

        LOG(LOG_INFO_PATH, "\tPATH: start = %d; ", ctx->pathes[ptrPath]&PATH_ELT_MASK, ctx->pathes[ptrPath+1]&PATH_ELT_MASK);

        if (_path_is_closed(ctx,ptrPath)){
            lastPathPointIdx = ctx->pathes[ptrPath+1]&PATH_ELT_MASK;
            LOG(LOG_INFO_PATH, "end = %d\n", lastPathPointIdx);
            //prevent closing on the same position, this could be generalize
            //to prevent processing of two consecutive point at the same position
            if (vec2_equ(ctx->points[i], ctx->points[lastPathPointIdx]))
                lastPathPointIdx--;
            iL = lastPathPointIdx;
        }else{
            lastPathPointIdx = ctx->pathes[ptrPath+1]&PATH_ELT_MASK;
            LOG(LOG_INFO_PATH, "end = %d\n", lastPathPointIdx);

            vec2 n = vec2_line_norm(ctx->points[i], ctx->points[i+1]);

            vec2 p0 = ctx->points[i];
            vec2 vhw = vec2_mult(n,hw);

            if (ctx->lineCap == VKVG_LINE_CAP_SQUARE)
                p0 = vec2_sub(p0, vhw);

            vhw = vec2_perp(vhw);

            if (ctx->lineCap == VKVG_LINE_CAP_ROUND){
                float step = M_PIF / hw;
                float a = acosf(n.x) + M_PIF_2;
                if (n.y < 0)
                    a = M_PIF-a;
                float a1 = a + M_PIF;

                a+=step;
                while (a < a1){
                    _add_vertexf(ctx, cosf(a) * hw + p0.x, sinf(a) * hw + p0.y);
                    a+=step;
                }
                uint32_t p0Idx = ctx->vertCount;
                for (uint p = firstIdx; p < p0Idx; p++)
                    _add_triangle_indices(ctx, p0Idx+1, p, p+1);
                firstIdx = p0Idx;
            }

            v.pos = vec2_add(p0, vhw);
            _add_vertex(ctx, v);
            v.pos = vec2_sub(p0, vhw);
            _add_vertex(ctx, v);

            _add_tri_indices_for_rect(ctx, firstIdx);

            iL = i++;
        }

        if (_path_has_curves (ctx,ptrPath)) {
            while (i < lastPathPointIdx){
                if (ptrPath + ptrCurve + 2 < ctx->pathPtr && (ctx->pathes [ptrPath + 2 + ptrCurve]&PATH_ELT_MASK) == i){
                    uint32_t lastCurvePoint = ctx->pathes[ptrPath + 3 + ptrCurve]&PATH_ELT_MASK;
                    while (i<lastCurvePoint){
                        iR = i+1;
                        _build_vb_step (ctx, v, hw, iL, i, iR, true);
                        iL = i++;
                    }
                    ptrCurve += 2;
                }else{
                    iR = i+1;
                    _build_vb_step (ctx, v, hw, iL, i, iR, false);
                    iL = i++;
                }
            }
        }else{
            while (i < lastPathPointIdx){
                iR = i+1;
                _build_vb_step(ctx,v,hw,iL,i,iR, false);
                iL = i++;
            }
        }

        if (!_path_is_closed(ctx,ptrPath)){
            vec2 n = vec2_line_norm(ctx->points[i-1], ctx->points[i]);
            vec2 p0 = ctx->points[i];
            vec2 vhw = vec2_mult(n, hw);

            if (ctx->lineCap == VKVG_LINE_CAP_SQUARE)
                p0 = vec2_add(p0, vhw);

            vhw = vec2_perp(vhw);

            v.pos = vec2_add(p0, vhw);
            _add_vertex(ctx, v);
            v.pos = vec2_sub(p0, vhw);
            _add_vertex(ctx, v);

            if (ctx->lineCap == VKVG_LINE_CAP_ROUND){
                firstIdx = ctx->vertCount;
                float step = M_PIF / hw;
                float a = acosf(n.x)+ M_PIF_2;
                if (n.y < 0)
                    a = M_PIF-a;
                float a1 = a - M_PIF;
                a-=step;
                while ( a > a1){
                    _add_vertexf(ctx, cosf(a) * hw + p0.x, sinf(a) * hw + p0.y);
                    a-=step;
                }

                uint32_t p0Idx = ctx->vertCount-1;
                for (uint p = firstIdx-1 ; p < p0Idx; p++)
                    _add_triangle_indices(ctx, p+1, p, firstIdx-2);
            }

            i++;
        }else{
            iR = ctx->pathes[ptrPath]&PATH_ELT_MASK;
            _build_vb_step(ctx,v,hw,iL,i,iR, false);

            VKVG_IBO_INDEX_TYPE* inds = &ctx->indexCache [ctx->indCount-6];
            VKVG_IBO_INDEX_TYPE ii = firstIdx;
            inds[1] = ii;
            inds[4] = ii;
            inds[5] = ii+1;
            i++;
        }

        ptrPath+=2+ptrCurve;
    }
    _record_draw_cmd(ctx);
}
void vkvg_paint (VkvgContext ctx){
    _check_cmd_buff_state(ctx);
    _draw_full_screen_quad(ctx,true);
}
inline void vkvg_set_source_rgb (VkvgContext ctx, float r, float g, float b) {
    vkvg_set_source_rgba (ctx, r, g, b, 1);
}
void vkvg_set_source_rgba (VkvgContext ctx, float r, float g, float b, float a)
{
    _update_cur_pattern (ctx, vkvg_pattern_create_rgba (r,g,b,a));
}
void vkvg_set_source_surface(VkvgContext ctx, VkvgSurface surf, float x, float y){
    _update_cur_pattern (ctx, vkvg_pattern_create_for_surface(surf));
    ctx->pushConsts.source.x = x;
    ctx->pushConsts.source.y = y;
    ctx->pushCstDirty = true;
}
void vkvg_set_source (VkvgContext ctx, VkvgPattern pat){
    _update_cur_pattern (ctx, pat);
    vkvg_pattern_reference  (pat);
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
    ctx->curOperator = op;
    _bind_draw_pipeline (ctx);
}
void vkvg_set_fill_rule (VkvgContext ctx, vkvg_fill_rule_t fr){
    ctx->curFillRule = fr;
}
vkvg_fill_rule_t vkvg_get_fill_rule (VkvgContext ctx){
    return ctx->curFillRule;
}
float vkvg_get_line_width (VkvgContext ctx){
    return ctx->lineWidth;
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
    _select_font_face (ctx, name);
}
void vkvg_set_font_size (VkvgContext ctx, uint32_t size){
    _set_font_size (ctx,size);
}

void vkvg_set_text_direction (vkvg_context* ctx, vkvg_direction_t direction){

}

void vkvg_show_text (VkvgContext ctx, const char* text){
    _check_cmd_buff_state(ctx);
    _show_text (ctx, text);
    _record_draw_cmd (ctx);
}

VkvgText vkvg_text_run_create (VkvgContext ctx, const char* text) {
    VkvgText tr = (vkvg_text_run_t*)calloc(1, sizeof(vkvg_text_run_t));
    _create_text_run(ctx, text, tr);
    return tr;
}
void vkvg_text_run_destroy (VkvgText textRun) {
    _destroy_text_run (textRun);
    free (textRun);
}
void vkvg_show_text_run (VkvgContext ctx, VkvgText textRun) {
    _show_text_run(ctx, textRun);
}
void vkvg_text_run_get_extents (VkvgText textRun, vkvg_text_extents_t* extents) {
    extents = &textRun->extents;
}

void vkvg_text_extents (VkvgContext ctx, const char* text, vkvg_text_extents_t* extents) {
    _text_extents(ctx, text, extents);
}
void vkvg_font_extents (VkvgContext ctx, vkvg_font_extents_t* extents) {
    _font_extents(ctx, extents);
}

void vkvg_save (VkvgContext ctx){
    LOG(LOG_INFO, "SAVE CONTEXT: ctx = %lu\n", (ulong)ctx);

    _flush_cmd_buff (ctx);
    _wait_flush_fence (ctx);

    VkvgDevice dev = ctx->pSurf->dev;
    vkvg_context_save_t* sav = (vkvg_context_save_t*)calloc(1,sizeof(vkvg_context_save_t));

    uint8_t curSaveStencil = ctx->curSavBit / 6;

    if (ctx->curSavBit > 0 && ctx->curSavBit % 6 == 0){//new save/restore stencil image have to be created
        ctx->savedStencils = (VkhImage*)realloc(ctx->savedStencils, curSaveStencil * sizeof (VkhImage));
        VkhImage savStencil = vkh_image_ms_create ((VkhDevice)dev,FB_STENCIL_FORMAT, dev->samples, ctx->pSurf->width, ctx->pSurf->height,
                                VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        ctx->savedStencils[curSaveStencil-1] = savStencil;

        vkh_cmd_begin (ctx->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        ctx->cmdStarted = true;

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
                       vkh_image_get_vkimage (savStencil),       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &cregion);

        vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencil, VK_IMAGE_ASPECT_STENCIL_BIT,
                              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                              VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

        VK_CHECK_RESULT(vkEndCommandBuffer(ctx->cmd));
        _wait_and_submit_cmd(ctx);
    }

    uint8_t curSaveBit = 1 << (ctx->curSavBit % 6 + 2);

    _start_cmd_for_render_pass (ctx);

    CmdBindPipeline         (ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineClipping);

    CmdSetStencilReference  (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT|curSaveBit);
    CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
    CmdSetStencilWriteMask  (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, curSaveBit);

    _draw_full_screen_quad (ctx, false);

    _bind_draw_pipeline (ctx);
    CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);


    sav->lineWidth  = ctx->lineWidth;
    sav->curOperator= ctx->curOperator;
    sav->lineCap    = ctx->lineCap;
    sav->lineWidth  = ctx->lineWidth;
    sav->curFillRule= ctx->curFillRule;

    sav->selectedFont = ctx->selectedFont;
    sav->selectedFont.fontFile = (char*)calloc(FONT_FILE_NAME_MAX_SIZE,sizeof(char));
    strcpy (sav->selectedFont.fontFile, ctx->selectedFont.fontFile);

    sav->currentFont  = ctx->currentFont;
    sav->textDirection= ctx->textDirection;
    sav->pushConsts   = ctx->pushConsts;
    sav->pattern      = ctx->pattern;

    sav->pNext      = ctx->pSavedCtxs;
    ctx->pSavedCtxs = sav;
    ctx->curSavBit++;

    if (ctx->pattern)
        vkvg_pattern_reference (ctx->pattern);
}
void vkvg_restore (VkvgContext ctx){
    if (ctx->pSavedCtxs == NULL){
        ctx->status = VKVG_STATUS_INVALID_RESTORE;
        return;
    }

    LOG(LOG_INFO, "RESTORE CONTEXT: ctx = %lu\n", ctx);

    vkvg_context_save_t* sav = ctx->pSavedCtxs;
    ctx->pSavedCtxs = sav->pNext;

    ctx->pushConsts   = sav->pushConsts;

    if (sav->pattern)
        _update_cur_pattern (ctx, sav->pattern);

    _flush_cmd_buff (ctx);
    _wait_flush_fence (ctx);

    ctx->curSavBit--;

    uint8_t curSaveBit = 1 << (ctx->curSavBit % 6 + 2);

    _start_cmd_for_render_pass (ctx);

    CmdBindPipeline         (ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineClipping);

    CmdSetStencilReference  (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT|curSaveBit);
    CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, curSaveBit);
    CmdSetStencilWriteMask  (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);

    _draw_full_screen_quad (ctx, false);

    _bind_draw_pipeline (ctx);
    CmdSetStencilCompareMask (ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);

    _flush_cmd_buff (ctx);

    uint8_t curSaveStencil = ctx->curSavBit / 6;
    if (ctx->curSavBit > 0 && ctx->curSavBit % 6 == 0){//addtional save/restore stencil image have to be copied back to surf stencil first
        VkhImage savStencil = ctx->savedStencils[curSaveStencil-1];

        vkh_cmd_begin (ctx->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        ctx->cmdStarted = true;

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
                       vkh_image_get_vkimage (savStencil),       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       vkh_image_get_vkimage (ctx->pSurf->stencil),VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &cregion);
        vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencil, VK_IMAGE_ASPECT_STENCIL_BIT,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                              VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

        VK_CHECK_RESULT(vkEndCommandBuffer(ctx->cmd));
        _wait_and_submit_cmd (ctx);

        vkh_image_destroy (savStencil);
    }

    ctx->lineWidth  = sav->lineWidth;
    ctx->curOperator= sav->curOperator;
    ctx->lineCap    = sav->lineCap;
    ctx->lineJoin   = sav->lineJoint;
    ctx->curFillRule= sav->curFillRule;

    ctx->selectedFont.charSize = sav->selectedFont.charSize;
    strcpy (ctx->selectedFont.fontFile, sav->selectedFont.fontFile);

    ctx->currentFont  = sav->currentFont;
    ctx->textDirection= sav->textDirection;

    _free_ctx_save(sav);
}

void vkvg_translate (VkvgContext ctx, float dx, float dy){
    vkvg_matrix_translate (&ctx->pushConsts.mat, dx, dy);
    _set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_scale (VkvgContext ctx, float sx, float sy){
    vkvg_matrix_scale (&ctx->pushConsts.mat, sx, sy);
    _set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_rotate (VkvgContext ctx, float radians){
    vkvg_matrix_rotate (&ctx->pushConsts.mat, radians);
    _set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_transform (VkvgContext ctx, const vkvg_matrix_t* matrix) {
    vkvg_matrix_t res;
    vkvg_matrix_multiply (&res, &ctx->pushConsts.mat, matrix);
    ctx->pushConsts.mat = res;
    _set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_identity_matrix (VkvgContext ctx) {
    vkvg_matrix_t im = VKVG_IDENTITY_MATRIX;
    ctx->pushConsts.mat = im;
    _set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_set_matrix (VkvgContext ctx, const vkvg_matrix_t* matrix){
    ctx->pushConsts.mat = (*matrix);
    _set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_get_matrix (VkvgContext ctx, const vkvg_matrix_t* matrix){
    memcpy (matrix, &ctx->pushConsts.mat, sizeof(vkvg_matrix_t));
}

void vkvg_render_svg (VkvgContext ctx, NSVGimage* svg, char *subId){
    NSVGshape* shape;
    NSVGpath* path;

    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);

    vkvg_set_source_rgba(ctx,0.0,0.0,0.0,1);

    for (shape = svg->shapes; shape != NULL; shape = shape->next) {
        if (subId != NULL) {
            if (strcmp(shape->id, subId)!=0)
                continue;
        }

        vkvg_new_path(ctx);

        float o = shape->opacity;

        vkvg_set_line_width(ctx, shape->strokeWidth);

        for (path = shape->paths; path != NULL; path = path->next) {
            float* p = path->pts;
            vkvg_move_to(ctx, p[0],p[1]);
            for (int i = 1; i < path->npts-2; i += 3) {
                p = &path->pts[i*2];
                vkvg_curve_to(ctx, p[0],p[1], p[2],p[3], p[4],p[5]);
            }
            if (path->closed)
                vkvg_close_path(ctx);
        }

        if (shape->fill.type == NSVG_PAINT_COLOR)
            _svg_set_color(ctx, shape->fill.color, o);
        else if (shape->fill.type == NSVG_PAINT_LINEAR_GRADIENT){
            NSVGgradient* g = shape->fill.gradient;
            _svg_set_color(ctx, g->stops[0].color, o);
        }

        if (shape->fill.type != NSVG_PAINT_NONE){
            if (shape->stroke.type == NSVG_PAINT_NONE){
                vkvg_fill(ctx);
                continue;
            }
            vkvg_fill_preserve (ctx);
        }

        if (shape->stroke.type == NSVG_PAINT_COLOR)
            _svg_set_color(ctx, shape->stroke.color, o);
        else if (shape->stroke.type == NSVG_PAINT_LINEAR_GRADIENT){
            NSVGgradient* g = shape->stroke.gradient;
            _svg_set_color(ctx, g->stops[0].color, o);
        }

        vkvg_stroke(ctx);
    }

}

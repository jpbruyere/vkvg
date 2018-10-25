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

#ifdef DEBUG
static vec2 debugLinePoints[1000];
static uint32_t dlpCount = 0;
#endif

VkvgContext vkvg_create(VkvgSurface surf)
{
    LOG(LOG_INFO, "CREATE Context: surf = %lu\n", surf);

    VkvgDevice dev = surf->dev;
    VkvgContext ctx = (vkvg_context*)calloc(1, sizeof(vkvg_context));

    if (ctx==NULL)
        return NULL;

    ctx->sizePoints     = VKVG_PTS_SIZE;
    ctx->sizeVertices   = VKVG_VBO_SIZE;
    ctx->sizeIndices    = VKVG_IBO_SIZE;
    ctx->sizePathes     = VKVG_PATHES_SIZE;
    ctx->lineWidth      = 1;
    ctx->pSurf          = surf;
    ctx->curOperator    = VKVG_OPERATOR_OVER;

    push_constants pc = {
            {0,0,0,1},
            {(float)ctx->pSurf->width,(float)ctx->pSurf->height},
            VKVG_PATTERN_TYPE_SOLID,
            0,
            VKVG_IDENTITY_MATRIX,
            VKVG_IDENTITY_MATRIX
    };
    ctx->pushConsts = pc;

    const VkClearRect cr = {{{0},{ctx->pSurf->width, ctx->pSurf->height}},0,1};
    ctx->clearRect = cr;

    ctx->pPrev          = surf->dev->lastCtx;
    if (ctx->pPrev != NULL)
        ctx->pPrev->pNext = ctx;
    surf->dev->lastCtx = ctx;

    ctx->selectedFont.fontFile = (char*)calloc(FONT_FILE_NAME_MAX_SIZE,sizeof(char));

    ctx->flushFence = vkh_fence_create(dev);

    ctx->points = (vec2*)       malloc (VKVG_VBO_SIZE*sizeof(vec2));
    ctx->pathes = (uint32_t*)   malloc (VKVG_PATHES_SIZE*sizeof(uint32_t));

    ctx->cmdPool = vkh_cmd_pool_create (dev, dev->gQueue->familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    _create_vertices_buff   (ctx);
    _create_gradient_buff   (ctx);
    _create_cmd_buff        (ctx);
    _createDescriptorPool   (ctx);
    _init_descriptor_sets   (ctx);
    _update_descriptor_set  (ctx, ctx->pSurf->dev->fontCache->cacheTex, ctx->dsFont);
    _update_gradient_desc_set(ctx);

    //add full screen quad at the beginning, recurently used
    _vao_add_rectangle (ctx,0,0,ctx->pSurf->width,ctx->pSurf->height);

    _init_cmd_buff          (ctx);
    _clear_path             (ctx);

    ctx->references = 1;
    ctx->status = VKVG_STATUS_SUCCESS;
    return ctx;
}
void vkvg_flush (VkvgContext ctx){
    ctx->status = VKVG_STATUS_SUCCESS;
    _flush_cmd_buff(ctx);
    _init_cmd_buff(ctx);
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

    if (ctx->pattern)
        vkvg_pattern_destroy (ctx->pattern);

    VkDevice dev = ctx->pSurf->dev->vkDev;

    vkDestroyFence      (dev, ctx->flushFence,NULL);
    vkFreeCommandBuffers(dev, ctx->cmdPool, 1, &ctx->cmd);
    vkDestroyCommandPool(dev, ctx->cmdPool, NULL);

    VkDescriptorSet dss[] = {ctx->dsFont, ctx->dsSrc, ctx->dsGrad};
    vkFreeDescriptorSets    (dev, ctx->descriptorPool, 3, dss);

    vkDestroyDescriptorPool (dev, ctx->descriptorPool,NULL);

    vkvg_buffer_destroy (&ctx->uboGrad);
    vkvg_buffer_destroy (&ctx->indices);
    vkvg_buffer_destroy (&ctx->vertices);

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
    ctx->status = VKVG_STATUS_SUCCESS;
    _finish_path(ctx);
}
void vkvg_new_path (VkvgContext ctx){
    ctx->status = VKVG_STATUS_SUCCESS;
    _clear_path(ctx);
}
//path closing is done by setting the endpoint of the path to the same index
//as the start point.
//I'll test if closing by adding a new point with the same x,y as the start point
//would not make more sense.
void vkvg_close_path (VkvgContext ctx){
    ctx->status = VKVG_STATUS_SUCCESS;
    if (_current_path_is_empty(ctx)){
        ctx->status = VKVG_STATUS_NO_CURRENT_POINT;
        return;
    }
    //check if at least 3 points are present
    if (ctx->pointCount - ctx->pathes [ctx->pathPtr-1] > 2){
        //set end idx of path to the same as start idx
        ctx->pathes[ctx->pathPtr] = ctx->pathes [ctx->pathPtr-1];
        //if last point of path is same pos as first point, remove it
        //if (vec2_equ(ctx->points[ctx->pointCount-1], ctx->points[ctx->pathes[ctx->pathPtr]]))
        //    ctx->pointCount--;
        _check_pathes_array(ctx);
        ctx->pathPtr++;
    }else
        _finish_path(ctx);
}
void vkvg_rel_line_to (VkvgContext ctx, float x, float y){
    ctx->status = VKVG_STATUS_SUCCESS;
    if (_current_path_is_empty(ctx)){
        ctx->status = VKVG_STATUS_NO_CURRENT_POINT;
        return;
    }
    vec2 cp = _get_current_position(ctx);
    vkvg_line_to(ctx, cp.x + x, cp.y + y);
}
void vkvg_line_to (VkvgContext ctx, float x, float y)
{
    ctx->status = VKVG_STATUS_SUCCESS;
    vec2 p = {x,y};
    if (_current_path_is_empty(ctx))
        vkvg_move_to(ctx, x,y);
    else if (vec2_equ(_get_current_position(ctx),p))
        return;

    _add_point(ctx,x,y);
}

void vkvg_arc (VkvgContext ctx, float xc, float yc, float radius, float a1, float a2){
    ctx->status = VKVG_STATUS_SUCCESS;
    while (a2 < a1)
        a2 += 2*M_PI;

    vec2 v = {cos(a1)*radius + xc, sin(a1)*radius + yc};

    float step = M_PI/radius;
    float a = a1;

    if (_current_path_is_empty(ctx))
        vkvg_move_to(ctx, v.x, v.y);
    else
        vkvg_line_to(ctx, v.x, v.y);

    a+=step;

    if (a2 == a1)
        return;

    while(a < a2){
        v.x = cos(a)*radius + xc;
        v.y = sin(a)*radius + yc;
        _add_point (ctx, v.x, v.y);
        a+=step;
    }

    a = a2;
    vec2 lastP = v;
    v.x = cos(a)*radius + xc;
    v.y = sin(a)*radius + yc;
    //if (!vec2_equ (v,lastP))//this test should not be required
        _add_point (ctx, v.x, v.y);
}
void vkvg_arc_negative (VkvgContext ctx, float xc, float yc, float radius, float a1, float a2) {
    ctx->status = VKVG_STATUS_SUCCESS;
    while (a2 > a1)
        a2 -= 2*M_PI;

    vec2 v = {cos(a1)*radius + xc, sin(a1)*radius + yc};

    float step = M_PI/radius;
    float a = a1;

    if (_current_path_is_empty(ctx))
        vkvg_move_to(ctx, v.x, v.y);
    else {
        vkvg_line_to(ctx, v.x, v.y);
    }

    a-=step;

    if (a2 == a1)//double check if this test should not be in the previous if
        return;

    while(a > a2){
        v.x = cos(a)*radius + xc;
        v.y = sin(a)*radius + yc;
        _add_point (ctx,v.x,v.y);
        a-=step;
    }

    a = a2;
    vec2 lastP = v;
    v.x = cos(a)*radius + xc;
    v.y = sin(a)*radius + yc;
    //if (!vec2_equ (v,lastP))
        _add_point (ctx, v.x, v.y);
}
void vkvg_rel_move_to (VkvgContext ctx, float x, float y)
{
    ctx->status = VKVG_STATUS_SUCCESS;
    if (_current_path_is_empty(ctx)){
        ctx->status = VKVG_STATUS_NO_CURRENT_POINT;
        return;
    }
    vec2 cp = _get_current_position(ctx);
    vkvg_move_to(ctx, cp.x + x, cp.y + y);
}
void vkvg_move_to (VkvgContext ctx, float x, float y)
{
    ctx->status = VKVG_STATUS_SUCCESS;
    _finish_path(ctx);
    _start_sub_path(ctx, x, y);
}
void vkvg_curve_to (VkvgContext ctx, float x1, float y1, float x2, float y2, float x3, float y3) {
    ctx->status = VKVG_STATUS_SUCCESS;
    if (_current_path_is_empty(ctx))
        vkvg_move_to(ctx, x1, y1);

    vec2 cp = _get_current_position(ctx);
    _recursive_bezier       (ctx, cp.x, cp.y, x1, y1, x2, y2, x3, y3, 0);
    /*cp.x = x3;
    cp.y = y3;
    if (!vec2_equ(ctx->points[ctx->pointCount-1],cp))*/
    _add_point(ctx,x3,y3);
}
void vkvg_rel_curve_to (VkvgContext ctx, float x1, float y1, float x2, float y2, float x3, float y3) {
    ctx->status = VKVG_STATUS_SUCCESS;
    if (_current_path_is_empty(ctx)){
        ctx->status = VKVG_STATUS_NO_CURRENT_POINT;
        return;
    }
    vec2 cp = _get_current_position(ctx);
    vkvg_curve_to (ctx, cp.x + x1, cp.y + y1, cp.x + x2, cp.y + y2, cp.x + x3, cp.y + y3);
}

void vkvg_rectangle (VkvgContext ctx, float x, float y, float w, float h){
    ctx->status = VKVG_STATUS_SUCCESS;
    _finish_path (ctx);

    _start_sub_path(ctx, x, y);
    _add_point (ctx, x + w, y);
    _add_point (ctx, x + w, y + h);
    _add_point (ctx, x, y + h);

    vkvg_close_path (ctx);
}
const VkClearAttachment clearStencil        = {VK_IMAGE_ASPECT_STENCIL_BIT, 1, {0}};
const VkClearAttachment clearColorAttach    = {VK_IMAGE_ASPECT_COLOR_BIT,   0, {0}};

void vkvg_reset_clip (VkvgContext ctx){
    ctx->status = VKVG_STATUS_SUCCESS;
    _check_cmd_buff_state (ctx);
    vkCmdClearAttachments(ctx->cmd, 1, &clearStencil, 1, &ctx->clearRect);
}
void vkvg_clear (VkvgContext ctx){
    ctx->status = VKVG_STATUS_SUCCESS;
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
void _poly_fill (VkvgContext ctx){
    CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelinePolyFill);

    uint32_t ptrPath = 0;;
    Vertex v = {};
    v.uv.z = -1;

    while (ptrPath < ctx->pathPtr){
        if (!_path_is_closed(ctx, ptrPath))
            ctx->pathes[ptrPath+1] = ctx->pathes[ptrPath];

        uint32_t firstPtIdx = ctx->pathes[ptrPath];
        uint32_t lastPtIdx = _get_last_point_of_closed_path (ctx, ptrPath);
        uint32_t pathPointCount = lastPtIdx - ctx->pathes[ptrPath] + 1;
        uint32_t firstVertIdx = ctx->vertCount;


        for (int i = 0; i < pathPointCount; i++) {
             v.pos = ctx->points[i+firstPtIdx];
             _add_vertex(ctx, v);
        }

        LOG(LOG_INFO_PATH, "\tpoly fill: point count = %d; 1st vert = %d; vert count = %d\n", pathPointCount, firstVertIdx, ctx->vertCount - firstVertIdx);
        CmdDraw (ctx->cmd, pathPointCount, 1, firstVertIdx ,0);

        ptrPath+=2;
    }
}
void vkvg_clip_preserve (VkvgContext ctx){
    ctx->status = VKVG_STATUS_SUCCESS;
    if (ctx->pathPtr == 0)      //nothing to fill
        return;
    _finish_path(ctx);

    if (ctx->pointCount * 4 > ctx->sizeIndices - ctx->indCount)//flush if vk buff is full
        vkvg_flush(ctx);

    _check_cmd_buff_state(ctx);
    _poly_fill (ctx);

    CmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineClipping);
    CmdDrawIndexed (ctx->cmd,6,1,0,0,0);

    //should test current operator to bind correct pipeline
    _bind_draw_pipeline (ctx);
    CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
}
void vkvg_fill_preserve (VkvgContext ctx){
    ctx->status = VKVG_STATUS_SUCCESS;
    if (ctx->pathPtr == 0)      //nothing to fill
        return;
    _finish_path(ctx);

    LOG(LOG_INFO, "FILL: ctx = %lu; path cpt = %d;\n", ctx, ctx->pathPtr / 2);

    if (ctx->pointCount * 4 > ctx->sizeIndices - ctx->indCount)//flush if vk buff is full
        vkvg_flush(ctx);

    _check_cmd_buff_state(ctx);
    _poly_fill (ctx);

    _bind_draw_pipeline (ctx);
    CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_FILL_BIT);
    CmdDrawIndexed (ctx->cmd,6,1,0,0,0);
    CmdSetStencilCompareMask(ctx->cmd, VK_STENCIL_FRONT_AND_BACK, STENCIL_CLIP_BIT);
}
void vkvg_stroke_preserve (VkvgContext ctx)
{
    ctx->status = VKVG_STATUS_SUCCESS;
    if (ctx->pathPtr == 0)//nothing to stroke
        return;
    _finish_path(ctx);

    LOG(LOG_INFO, "STROKE: ctx = %lu; path cpt = %d;\n", ctx, ctx->pathPtr / 2);

    if (ctx->pointCount * 4 > ctx->sizeIndices - ctx->indCount)
        vkvg_flush(ctx);

    Vertex v = {};
    v.uv.z = -1;

    float hw = ctx->lineWidth / 2.0;
    int i = 0, ptrPath = 0;

    uint32_t lastPathPointIdx, iL, iR;

    while (ptrPath < ctx->pathPtr){
        uint32_t firstIdx = ctx->vertCount;
        i = ctx->pathes[ptrPath];

        LOG(LOG_INFO_PATH, "\tPATH: start = %d; ", ctx->pathes[ptrPath], ctx->pathes[ptrPath+1]);

        if (_path_is_closed(ctx,ptrPath)){
            lastPathPointIdx = _get_last_point_of_closed_path(ctx,ptrPath);
            LOG(LOG_INFO_PATH, "end = %d\n", lastPathPointIdx);
            //prevent closing on the same position, this could be generalize
            //to prevent processing of two consecutive point at the same position
            if (vec2_equ(ctx->points[i], ctx->points[lastPathPointIdx]))
                lastPathPointIdx--;
            iL = lastPathPointIdx;
        }else{
            lastPathPointIdx = ctx->pathes[ptrPath+1];
            LOG(LOG_INFO_PATH, "end = %d\n", lastPathPointIdx);

            vec2 n = vec2_line_norm(ctx->points[i], ctx->points[i+1]);

            vec2 p0 = ctx->points[i];
            vec2 vhw = vec2_mult(n,hw);

            if (ctx->lineCap == VKVG_LINE_CAP_SQUARE)
                p0 = vec2_sub(p0, vhw);

            vhw = vec2_perp(vhw);

            if (ctx->lineCap == VKVG_LINE_CAP_ROUND){
                float step = M_PI_2 / hw;
                float a = acos(n.x) + M_PI_2;
                if (n.y < 0)
                    a = M_PI-a;
                float a1 = a + M_PI;

                a+=step;
                while (a < a1){
                    _add_vertexf(ctx, cos(a) * hw + p0.x, sin(a) * hw + p0.y);
                    a+=step;
                }
                uint32_t p0Idx = ctx->vertCount;
                for (int p = firstIdx; p < p0Idx; p++)
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

        while (i < lastPathPointIdx){
            iR = i+1;
            _build_vb_step(ctx,v,hw,iL,i,iR);
            iL = i++;
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
                float step = M_PI_2 / hw;
                float a = acos(n.x)+ M_PI_2;
                if (n.y < 0)
                    a = M_PI-a;
                float a1 = a - M_PI;
                a-=step;
                while ( a > a1){
                    _add_vertexf(ctx, cos(a) * hw + p0.x, sin(a) * hw + p0.y);
                    a-=step;
                }

                uint32_t p0Idx = ctx->vertCount-1;
                for (int p = firstIdx-1 ; p < p0Idx; p++)
                    _add_triangle_indices(ctx, p+1, p, firstIdx-2);
            }

            i++;
        }else{
            iR = ctx->pathes[ptrPath];
            _build_vb_step(ctx,v,hw,iL,i,iR);

            uint32_t* inds = (uint32_t*)(ctx->indices.allocInfo.pMappedData + ((ctx->indCount-6) * sizeof(uint32_t)));
            uint32_t ii = firstIdx;
            inds[1] = ii;
            inds[4] = ii;
            inds[5] = ii+1;
            i++;
        }

        ptrPath+=2;
    }
    _record_draw_cmd(ctx);
}
void vkvg_paint (VkvgContext ctx){
    ctx->status = VKVG_STATUS_SUCCESS;
    _check_cmd_buff_state(ctx);
    CmdDrawIndexed (ctx->cmd,6,1,0,0,0);
}
inline void vkvg_set_source_rgb (VkvgContext ctx, float r, float g, float b) {
    ctx->status = VKVG_STATUS_SUCCESS;
    vkvg_set_source_rgba (ctx, r, g, b, 1);
}
void vkvg_set_source_rgba (VkvgContext ctx, float r, float g, float b, float a)
{
    ctx->status = VKVG_STATUS_SUCCESS;
    _update_cur_pattern (ctx, vkvg_pattern_create_rgba (r,g,b,a));
}
void vkvg_set_source_surface(VkvgContext ctx, VkvgSurface surf, float x, float y){
    ctx->status = VKVG_STATUS_SUCCESS;
    _update_cur_pattern (ctx, vkvg_pattern_create_for_surface(surf));
    ctx->pushConsts.source.x = x;
    ctx->pushConsts.source.y = y;
    ctx->pushCstDirty = true;
}
void vkvg_set_source (VkvgContext ctx, VkvgPattern pat){
    ctx->status = VKVG_STATUS_SUCCESS;
    _update_cur_pattern (ctx, pat);
    vkvg_pattern_reference  (pat);
}
void vkvg_set_line_width (VkvgContext ctx, float width){
    ctx->status = VKVG_STATUS_SUCCESS;
    ctx->lineWidth = width;
}
void vkvg_set_line_cap (VkvgContext ctx, vkvg_line_cap_t cap){
    ctx->status = VKVG_STATUS_SUCCESS;
    ctx->lineCap = cap;
}
void vkvg_set_line_join (VkvgContext ctx, vkvg_line_join_t join){
    ctx->status = VKVG_STATUS_SUCCESS;
    ctx->lineJoin = join;
}
void vkvg_set_operator (VkvgContext ctx, vkvg_operator_t op){
    ctx->status = VKVG_STATUS_SUCCESS;
    ctx->curOperator = op;
    _bind_draw_pipeline (ctx);
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
    ctx->status = VKVG_STATUS_SUCCESS;
    _select_font_face (ctx, name);
}
void vkvg_set_font_size (VkvgContext ctx, uint32_t size){
    ctx->status = VKVG_STATUS_SUCCESS;
    _set_font_size (ctx,size);
}

void vkvg_set_text_direction (vkvg_context* ctx, vkvg_direction_t direction){
    ctx->status = VKVG_STATUS_SUCCESS;
}

void vkvg_show_text (VkvgContext ctx, const char* text){
    ctx->status = VKVG_STATUS_SUCCESS;
    _check_cmd_buff_state(ctx);
    _show_text (ctx, text);
    _record_draw_cmd (ctx);
}

VkvgText vkvg_text_run_create (VkvgContext ctx, const char* text) {
    ctx->status = VKVG_STATUS_SUCCESS;
    VkvgText tr = (vkvg_text_run_t*)calloc(1, sizeof(vkvg_text_run_t));
    _create_text_run(ctx, text, tr);
    return tr;
}
void vkvg_text_run_destroy (VkvgText textRun) {
    _destroy_text_run (textRun);
    free (textRun);
}
void vkvg_show_text_run (VkvgContext ctx, VkvgText textRun) {
    ctx->status = VKVG_STATUS_SUCCESS;
    _show_text_run(ctx, textRun);
}
vkvg_text_extents_t* vkvg_text_run_get_extents (VkvgText textRun) {
    return &textRun->extents;
}

void vkvg_text_extents (VkvgContext ctx, const char* text, vkvg_text_extents_t* extents) {
    _text_extents(ctx, text, extents);
}
void vkvg_font_extents (VkvgContext ctx, vkvg_font_extents_t* extents) {
    _font_extents(ctx, extents);
}

void vkvg_save (VkvgContext ctx){
    ctx->status = VKVG_STATUS_SUCCESS;
    LOG(LOG_INFO, "SAVE CONTEXT: ctx = %lu\n", ctx);

    _flush_cmd_buff(ctx);

    VkvgDevice dev = ctx->pSurf->dev;
    vkvg_context_save_t* sav = (vkvg_context_save_t*)calloc(1,sizeof(vkvg_context_save_t));

    sav->stencilMS = vkh_image_ms_create (dev,VK_FORMAT_S8_UINT, VKVG_SAMPLES, ctx->pSurf->width, ctx->pSurf->height,
                        VMA_MEMORY_USAGE_GPU_ONLY,
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    vkh_cmd_begin (ctx->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    vkh_image_set_layout (ctx->cmd, sav->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT,
                          VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkImageCopy cregion = { .srcSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1},
                            .dstSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1},
                            .extent = {ctx->pSurf->width,ctx->pSurf->height,1}};
    vkCmdCopyImage(ctx->cmd,
                   vkh_image_get_vkimage (ctx->pSurf->stencilMS),VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   vkh_image_get_vkimage (sav->stencilMS),       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &cregion);

    vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT,
                          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

    VK_CHECK_RESULT(vkEndCommandBuffer(ctx->cmd));
    _submit_ctx_cmd(ctx);

    sav->sizePoints = ctx->sizePoints;
    sav->pointCount = ctx->pointCount;

    sav->points = (vec2*)malloc (sav->pointCount * sizeof(vec2));
    memcpy (sav->points, ctx->points, sav->pointCount * sizeof(vec2));

    sav->pathPtr    = ctx->pathPtr;
    sav->sizePathes = ctx->sizePathes;

    sav->pathes = (uint32_t*)malloc (sav->pathPtr * sizeof(uint32_t));
    memcpy (sav->pathes, ctx->pathes, sav->pathPtr * sizeof(uint32_t));

    sav->lineWidth  = ctx->lineWidth;
    sav->curOperator= ctx->curOperator;
    sav->lineCap    = ctx->lineCap;
    sav->lineWidth  = ctx->lineWidth;

    sav->selectedFont = ctx->selectedFont;
    sav->selectedFont.fontFile = (char*)calloc(FONT_FILE_NAME_MAX_SIZE,sizeof(char));
    strcpy (sav->selectedFont.fontFile, ctx->selectedFont.fontFile);

    sav->currentFont  = ctx->currentFont;
    sav->textDirection= ctx->textDirection;
    sav->pushConsts   = ctx->pushConsts;
    sav->pattern      = ctx->pattern;

    sav->pNext      = ctx->pSavedCtxs;
    ctx->pSavedCtxs = sav;

    if (ctx->pattern)
        vkvg_pattern_reference (ctx->pattern);

    _wait_and_reset_ctx_cmd (ctx);
    _init_cmd_buff          (ctx);
}
void vkvg_restore (VkvgContext ctx){
    ctx->status = VKVG_STATUS_SUCCESS;
    if (ctx->pSavedCtxs == NULL){
        ctx->status = VKVG_STATUS_INVALID_RESTORE;
        return;
    }

    LOG(LOG_INFO, "RESTORE CONTEXT: ctx = %lu\n", ctx);

    vkvg_context_save_t* sav = ctx->pSavedCtxs;
    ctx->pSavedCtxs = sav->pNext;

    ctx->pushConsts   = sav->pushConsts;

    _update_cur_pattern(ctx, sav->pattern);

    _flush_cmd_buff(ctx);

    vkh_cmd_begin (ctx->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    vkh_image_set_layout (ctx->cmd, sav->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkImageCopy cregion = { .srcSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1},
                            .dstSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1},
                            .extent = {ctx->pSurf->width,ctx->pSurf->height,1}};
    vkCmdCopyImage(ctx->cmd,
                   vkh_image_get_vkimage (sav->stencilMS),       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   vkh_image_get_vkimage (ctx->pSurf->stencilMS),VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &cregion);
    vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

    VK_CHECK_RESULT(vkEndCommandBuffer(ctx->cmd));
    _submit_ctx_cmd(ctx);

    ctx->sizePoints = sav->sizePoints;
    ctx->pointCount = sav->pointCount;

    ctx->points = (vec2*)realloc ( ctx->points, ctx->sizePoints * sizeof(vec2));
    memset (ctx->points, 0, ctx->sizePoints * sizeof(vec2));
    memcpy (ctx->points, sav->points, ctx->pointCount * sizeof(vec2));

    ctx->pathPtr    = sav->pathPtr;
    ctx->sizePathes = sav->sizePathes;

    ctx->pathes = (uint32_t*)realloc (ctx->pathes, ctx->sizePathes * sizeof(uint32_t));
    memset (ctx->pathes, 0, ctx->sizePathes * sizeof(uint32_t));
    memcpy (ctx->pathes, sav->pathes, ctx->pathPtr * sizeof(uint32_t));

    ctx->lineWidth  = sav->lineWidth;
    ctx->curOperator= sav->curOperator;
    ctx->lineCap    = sav->lineCap;
    ctx->lineJoin  = sav->lineJoint;

    ctx->selectedFont.charSize = sav->selectedFont.charSize;
    strcpy (ctx->selectedFont.fontFile, sav->selectedFont.fontFile);

    ctx->currentFont  = sav->currentFont;
    ctx->textDirection= sav->textDirection;

    _wait_and_reset_ctx_cmd (ctx);
    _init_cmd_buff          (ctx);

    _free_ctx_save(sav);
}

void vkvg_translate (VkvgContext ctx, float dx, float dy){
    ctx->status = VKVG_STATUS_SUCCESS;
    vkvg_matrix_translate (&ctx->pushConsts.mat, dx, dy);
    _set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_scale (VkvgContext ctx, float sx, float sy){
    ctx->status = VKVG_STATUS_SUCCESS;
    vkvg_matrix_scale (&ctx->pushConsts.mat, sx, sy);
    _set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_rotate (VkvgContext ctx, float radians){
    ctx->status = VKVG_STATUS_SUCCESS;
    vkvg_matrix_rotate (&ctx->pushConsts.mat, radians);
    _set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_transform (VkvgContext ctx, const vkvg_matrix_t* matrix) {
    ctx->status = VKVG_STATUS_SUCCESS;
    vkvg_matrix_t res;
    vkvg_matrix_multiply (&res, &ctx->pushConsts.mat, matrix);
    ctx->pushConsts.mat = res;
    _set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_identity_matrix (VkvgContext ctx) {
    ctx->status = VKVG_STATUS_SUCCESS;
    vkvg_matrix_t im = VKVG_IDENTITY_MATRIX;
    ctx->pushConsts.mat = im;
    _set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_set_matrix (VkvgContext ctx, const vkvg_matrix_t* matrix){
    ctx->status = VKVG_STATUS_SUCCESS;
    ctx->pushConsts.mat = (*matrix);
    _set_mat_inv_and_vkCmdPush (ctx);
}
void vkvg_get_matrix (VkvgContext ctx, const vkvg_matrix_t* matrix){
    memcpy (matrix, &ctx->pushConsts.mat, sizeof(vkvg_matrix_t));
}

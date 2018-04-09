#include "vkvg_device_internal.h"
#include "vkvg_context_internal.h"
#include "vkvg_surface_internal.h"

#ifdef DEBUG
static vec2 debugLinePoints[1000];
static uint32_t dlpCount = 0;
#endif

VkvgContext vkvg_create(VkvgSurface surf)
{
    VkvgDevice dev = surf->dev;
    VkvgContext ctx = (vkvg_context*)calloc(1, sizeof(vkvg_context));

    ctx->sizePoints     = VKVG_PTS_SIZE;
    ctx->sizeVertices   = VKVG_VBO_SIZE;
    ctx->sizeIndices    = VKVG_IBO_SIZE;
    ctx->sizePathes     = VKVG_PATHES_SIZE;
    ctx->curPos.x       = ctx->curPos.y = 0;
    ctx->lineWidth      = 1;
    ctx->pSurf          = surf;

    push_constants pc = {
            {},
            {2.0f/(float)ctx->pSurf->width,2.0f/(float)ctx->pSurf->height},
            {-1.f,-1.f},
            VKVG_SRC_SOLID
    };
    ctx->pushConsts = pc;

    ctx->pPrev          = surf->dev->lastCtx;
    if (ctx->pPrev != NULL)
        ctx->pPrev->pNext = ctx;
    surf->dev->lastCtx = ctx;

    ctx->selectedFont.fontFile = (char*)calloc(FONT_FILE_NAME_MAX_SIZE,sizeof(char));

    ctx->flushFence = vkh_fence_create(dev->vkDev);

    ctx->points = (vec2*)       malloc (VKVG_VBO_SIZE*sizeof(vec2));
    ctx->pathes = (uint32_t*)   malloc (VKVG_PATHES_SIZE*sizeof(uint32_t));

    ctx->cmdPool = vkh_cmd_pool_create (dev->vkDev, dev->qFam, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    _create_vertices_buff   (ctx);
    _create_cmd_buff        (ctx);
    _createDescriptorPool   (ctx);
    _init_descriptor_sets   (ctx);
    _update_descriptor_set  (ctx, ctx->pSurf->dev->fontCache->cacheTex, ctx->dsFont);
    _init_cmd_buff          (ctx);
    _clear_path             (ctx);

    return ctx;
}
void vkvg_flush (VkvgContext ctx){
    _flush_cmd_buff(ctx);
    _init_cmd_buff(ctx);

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
    vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineLineList);
    vkCmdDrawIndexed(ctx->cmd, ctx->indCount-ctx->curIndStart, 1, ctx->curIndStart, 0, 1);
    _flush_cmd_buff(ctx);
#endif

}

void vkvg_destroy (VkvgContext ctx)
{
    _flush_cmd_buff(ctx);

    VkDevice dev = ctx->pSurf->dev->vkDev;
    vkDestroyFence      (dev, ctx->flushFence,NULL);
    vkFreeCommandBuffers(dev, ctx->cmdPool, 1, &ctx->cmd);
    vkDestroyCommandPool(dev, ctx->cmdPool, NULL);

    VkDescriptorSet dss[] = {ctx->dsFont,ctx->dsSrc};
    vkFreeDescriptorSets    (dev, ctx->descriptorPool,2,dss);

    vkDestroyDescriptorPool (dev, ctx->descriptorPool,NULL);

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


void vkvg_close_path (VkvgContext ctx){
    if (ctx->pathPtr % 2 == 0)//current path is empty
        return;
    //check if at least 3 points are present
    if (ctx->pointCount - ctx->pathes [ctx->pathPtr-1] > 2){
        //set end idx of path to the same as start idx
        ctx->pathes[ctx->pathPtr] = ctx->pathes [ctx->pathPtr-1];
        //start new path
        _check_pathes_array(ctx);
        ctx->pathPtr++;
    }
}

void vkvg_line_to (VkvgContext ctx, float x, float y)
{
    if (ctx->pathPtr % 2 == 0){//current path is empty
        //set start to current idx in point array
        ctx->pathes[ctx->pathPtr] = ctx->pointCount;
        _check_pathes_array(ctx);
        ctx->pathPtr++;
        _add_curpos(ctx);
    }
    _add_point(ctx, x, y);
}

void vkvg_arc (VkvgContext ctx, float xc, float yc, float radius, float a1, float a2){
    float aDiff = a2 - a1;
    float aa1, aa2;
    float step = M_PI/radius;

    aa1 = _normalizeAngle(a1);
    aa2 = aa1 + aDiff;

    //if (aa1 > aa2)
     //   aa2 += M_PI * 2.0;
    float a = aa1;
    vec2 v = {cos(a)*radius + xc, sin(a)*radius + yc};

    if (ctx->pathPtr % 2 == 0){//current path is empty
        //set start to current idx in point array
        ctx->pathes[ctx->pathPtr] = ctx->pointCount;
        _check_pathes_array(ctx);
        ctx->pathPtr++;
        if (!vec2_equ(v,ctx->curPos))
            _add_curpos(ctx);
    }

    if (aDiff < 0){
        while(a > aa2){
            v.x = cos(a)*radius + xc;
            v.y = sin(a)*radius + yc;
            _add_point(ctx,v.x,v.y);
            a-=step;
        }
    }else{
        while(a < aa2){
            v.x = cos(a)*radius + xc;
            v.y = sin(a)*radius + yc;
            _add_point(ctx,v.x,v.y);
            a+=step;
        }
    }
    a = aa2;
    v.x = cos(a)*radius + xc;
    v.y = sin(a)*radius + yc;
    if (!vec2_equ (v,ctx->curPos))
        _add_point(ctx,v.x,v.y);
}

void vkvg_move_to (VkvgContext ctx, float x, float y)
{
    _finish_path(ctx);

    ctx->curPos.x = x;
    ctx->curPos.y = y;
}

void vkvg_clip_preserve (VkvgContext ctx){
    vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipelineClipping);
    vkvg_fill_preserve(ctx);
    vkvg_flush(ctx);
    //should test current operator to bind correct pipeline
    ctx->stencilRef++;
    vkCmdBindPipeline(ctx->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pSurf->dev->pipeline);
    vkCmdSetStencilReference(ctx->cmd,VK_STENCIL_FRONT_AND_BACK, ctx->stencilRef);
}
void vkvg_reset_clip (VkvgContext ctx){
    /*
    VkClearValue  clearValue = {0};
    VkClearAttachment clrAtt = {VK_IMAGE_ASPECT_STENCIL_BIT, 2 ,clearValue};
    VkClearRect clr = {{{0,0},{ctx->pSurf->width,ctx->pSurf->height}},0,1};
    ctx->stencilRef=0;
    vkCmdSetStencilReference(ctx->cmd,VK_STENCIL_FRONT_AND_BACK, ctx->stencilRef);
    vkCmdClearAttachments (ctx->cmd ,1,&clrAtt,1,&clr);
    */
    _flush_cmd_buff(ctx);
    _clear_stencil(ctx->pSurf);
    ctx->stencilRef=0;
    _init_cmd_buff(ctx);
}
void vkvg_clip (VkvgContext ctx){
    vkvg_clip_preserve(ctx);
    _clear_path(ctx);
}
void vkvg_fill_preserve (VkvgContext ctx){
    if (ctx->pathPtr == 0)      //nothing to fill
        return;
    if (ctx->pathPtr % 2 != 0)  //current path is no close
        vkvg_close_path(ctx);
    if (ctx->pointCount * 4 > ctx->sizeIndices - ctx->indCount)
        vkvg_flush(ctx);

    uint32_t lastPathPointIdx, i = 0, ptrPath = 0;;
    Vertex v = {};
    v.uv.z = -1;

    while (ptrPath < ctx->pathPtr){
        if (!_path_is_closed(ctx,ptrPath)){
            ptrPath+=2;
            continue;
        }
        lastPathPointIdx = _get_last_point_of_closed_path (ctx, ptrPath);
        uint32_t pathPointCount = lastPathPointIdx - ctx->pathes[ptrPath] + 1;
        uint32_t firstVertIdx = ctx->vertCount;

        ear_clip_point ecps[pathPointCount];
        uint32_t ecps_count = pathPointCount;

        while (i < lastPathPointIdx){
            v.pos = ctx->points[i];
            ear_clip_point ecp = {
                v.pos,
                i+firstVertIdx,
                &ecps[i+1]
            };
            ecps[i] = ecp;
            _add_vertex(ctx, v);
            i++;
        }
        v.pos = ctx->points[i];
        ear_clip_point ecp = {
            v.pos,
            i+firstVertIdx,
            ecps
        };
        ecps[i] = ecp;
        _add_vertex(ctx, v);

        ear_clip_point* ecp_current = ecps;

        while (ecps_count > 3) {
            ear_clip_point* v0 = ecp_current->next,
                    *v1 = ecp_current, *v2 = ecp_current->next->next;
            if (ecp_zcross (v0, v2, v1)<0){
                ecp_current = ecp_current->next;
                continue;
            }
            ear_clip_point* vP = v2->next;
            bool isEar = true;
            while (vP!=v1){
                if (ptInTriangle(vP->pos,v0->pos,v2->pos,v1->pos)){
                    isEar = false;
                    break;
                }
                vP = vP->next;
            }
            if (isEar){
                _add_triangle_indices(ctx, v0->idx,v1->idx,v2->idx);
                v1->next = v2;
                ecps_count --;
            }else
                ecp_current = ecp_current->next;
        }
        if (ecps_count == 3){
            _add_triangle_indices(ctx, ecp_current->next->idx,ecp_current->idx,ecp_current->next->next->idx);
        }

        ptrPath+=2;
    }
    _record_draw_cmd(ctx);
}
void vkvg_fill (VkvgContext ctx){
    vkvg_fill_preserve(ctx);
    _clear_path(ctx);
}
void vkvg_stroke_preserve (VkvgContext ctx)
{
    _finish_path(ctx);

    if (ctx->pathPtr == 0)//nothing to stroke
        return;
    if (ctx->pointCount * 4 > ctx->sizeIndices - ctx->indCount)
        vkvg_flush(ctx);

    Vertex v = {};
    v.uv.z = -1;

    float hw = ctx->lineWidth / 2.0;
    int i = 0, ptrPath = 0;

    uint32_t lastPathPointIdx, iL, iR;



    while (ptrPath < ctx->pathPtr){
        uint32_t firstIdx = ctx->vertCount;

        if (_path_is_closed(ctx,ptrPath)){
            lastPathPointIdx = _get_last_point_of_closed_path(ctx,ptrPath);
            iL = lastPathPointIdx;
        }else{
            lastPathPointIdx = ctx->pathes[ptrPath+1];
            vec2 bisec = vec2_line_norm(ctx->points[i], ctx->points[i+1]);
            bisec = vec2_perp(bisec);
            bisec = vec2_mult(bisec,hw);

            v.pos = vec2_add(ctx->points[i], bisec);
            _add_vertex(ctx, v);
            v.pos = vec2_sub(ctx->points[i], bisec);
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
            vec2 bisec = vec2_line_norm(ctx->points[i-1], ctx->points[i]);
            bisec = vec2_perp(bisec);
            bisec = vec2_mult(bisec,hw);

            v.pos = vec2_add(ctx->points[i], bisec);
            _add_vertex(ctx, v);
            v.pos = vec2_sub(ctx->points[i], bisec);
            _add_vertex(ctx, v);

            i++;
        }else{
            iR = ctx->pathes[ptrPath];
            _build_vb_step(ctx,v,hw,iL,i,iR);

            uint32_t* inds = (uint32_t*)(ctx->indices.mapped + ((ctx->indCount-6) * sizeof(uint32_t)));
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
void vkvg_stroke (VkvgContext ctx)
{
    vkvg_stroke_preserve(ctx);
    _clear_path(ctx);
}
void _vkvg_fill_rectangle (VkvgContext ctx, float x, float y, float width, float height){
    Vertex v[4] =
    {
        {{x,y},             {0,0,-1}},
        {{x,y+height},      {0,0,-1}},
        {{x+width,y},       {0,0,-1}},
        {{x+width,y+height},{0,0,-1}}
    };
    uint32_t firstIdx = ctx->vertCount;
    Vertex* pVert = (Vertex*)(ctx->vertices.mapped + ctx->vertCount * sizeof(Vertex));
    memcpy (pVert,v,4*sizeof(Vertex));
    ctx->vertCount+=4;
    _add_tri_indices_for_rect(ctx, firstIdx);
}
void vkvg_paint (VkvgContext ctx){
    _vkvg_fill_rectangle (ctx, 0, 0, ctx->pSurf->width, ctx->pSurf->height);
}

void vkvg_set_rgba (VkvgContext ctx, float r, float g, float b, float a)
{
    vec4 c = {r,g,b,a};
    ctx->pushConsts.source = c;
    ctx->pushConsts.srcType = VKVG_SRC_SOLID;
    vkCmdPushConstants(ctx->cmd, ctx->pSurf->dev->pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constants),&ctx->pushConsts);
    ctx->curRGBA.x = r;
    ctx->curRGBA.y = g;
    ctx->curRGBA.z = b;
    ctx->curRGBA.w = a;
    /*
    _flush_cmd_buff(ctx);

    vkh_cmd_begin (ctx->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkClearColorValue clr = {r,g,b,a};
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};

    set_image_layout        (ctx->cmd, ctx->source->image, VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    vkCmdClearColorImage    (ctx->cmd, ctx->source->image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,&clr,1,&range);
    set_image_layout        (ctx->cmd, ctx->source->image, VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    vkh_cmd_end                 (ctx->cmd);

    _submit_wait_and_reset_cmd  (ctx);
    _init_cmd_buff              (ctx);
    */
}
void vkvg_set_source_surface(VkvgContext ctx, VkvgSurface surf, float x, float y){
    _flush_cmd_buff(ctx);

    ctx->source = surf->img;

    vkh_image_create_sampler(ctx->source,VK_FILTER_NEAREST, VK_FILTER_NEAREST,
                             VK_SAMPLER_MIPMAP_MODE_NEAREST,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    if (vkh_image_get_layout (ctx->source) != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
        vkh_cmd_begin (ctx->cmd,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        vkh_image_set_layout        (ctx->cmd, ctx->source, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        vkh_cmd_end                 (ctx->cmd);

        _submit_wait_and_reset_cmd  (ctx);
    }

    _update_descriptor_set          (ctx, ctx->source, ctx->dsSrc);
    _init_cmd_buff                  (ctx);

    vec4 srcRect = {x,y,surf->width,surf->height};
    ctx->pushConsts.source = srcRect;
    ctx->pushConsts.srcType = VKVG_SRC_PATTERN;
    vkCmdPushConstants(ctx->cmd, ctx->pSurf->dev->pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constants),&ctx->pushConsts);
}

void vkvg_set_linewidth (VkvgContext ctx, float width){
    ctx->lineWidth = width;
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
    _show_text (ctx, text);
    _record_draw_cmd (ctx);
}

void vkvg_save (VkvgContext ctx){
    _flush_cmd_buff(ctx);

    VkvgDevice dev = ctx->pSurf->dev;
    vkvg_context_save_t* sav = (vkvg_context_save_t*)calloc(1,sizeof(vkvg_context_save_t));

    sav->stencilMS = vkh_image_ms_create(dev,VK_FORMAT_S8_UINT,VKVG_SAMPLES,ctx->pSurf->width,ctx->pSurf->height,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    vkh_cmd_begin (ctx->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    vkh_image_set_layout (ctx->cmd, sav->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    VkImageCopy cregion = { .srcSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1},
                            .dstSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1},
                            .extent = {ctx->pSurf->width,ctx->pSurf->height,1}};
    vkCmdCopyImage(ctx->cmd,
                   vkh_image_get_vkimage (ctx->pSurf->stencilMS),VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   vkh_image_get_vkimage (sav->stencilMS),       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &cregion);

    vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    VK_CHECK_RESULT(vkEndCommandBuffer(ctx->cmd));
    _submit_ctx_cmd(ctx);

    sav->stencilRef = ctx->stencilRef;
    sav->sizePoints = ctx->sizePoints;
    sav->pointCount = ctx->pointCount;

    sav->points = (vec2*)malloc (sav->pointCount * sizeof(vec2));
    memcpy (sav->points, ctx->points, sav->pointCount * sizeof(vec2));

    sav->pathPtr    = ctx->pathPtr;
    sav->sizePathes = ctx->sizePathes;

    sav->pathes = (uint32_t*)malloc (sav->pathPtr * sizeof(uint32_t));
    memcpy (sav->pathes, ctx->pathes, sav->pathPtr * sizeof(uint32_t));

    sav->curPos     = ctx->curPos;
    sav->curRGBA    = ctx->curRGBA;
    sav->lineWidth  = ctx->lineWidth;

    sav->selectedFont = ctx->selectedFont;
    sav->selectedFont.fontFile = (char*)calloc(FONT_FILE_NAME_MAX_SIZE,sizeof(char));
    strcpy (sav->selectedFont.fontFile, ctx->selectedFont.fontFile);

    sav->currentFont  = ctx->currentFont;
    sav->textDirection= ctx->textDirection;
    sav->pushConsts   = ctx->pushConsts;
    sav->source       = ctx->source;

    sav->pNext      = ctx->pSavedCtxs;
    ctx->pSavedCtxs = sav;

    _wait_and_reset_ctx_cmd (ctx);
    _init_cmd_buff          (ctx);
}
void vkvg_restore (VkvgContext ctx){
    if (ctx->pSavedCtxs == NULL)
        return;
    _flush_cmd_buff(ctx);

    vkvg_context_save_t* sav = ctx->pSavedCtxs;
    ctx->pSavedCtxs = sav->pNext;

    vkh_cmd_begin (ctx->cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    vkh_image_set_layout (ctx->cmd, sav->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    VkImageCopy cregion = { .srcSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1},
                            .dstSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1},
                            .extent = {ctx->pSurf->width,ctx->pSurf->height,1}};
    vkCmdCopyImage(ctx->cmd,
                   vkh_image_get_vkimage (sav->stencilMS),       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   vkh_image_get_vkimage (ctx->pSurf->stencilMS),VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &cregion);
    vkh_image_set_layout (ctx->cmd, ctx->pSurf->stencilMS, VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    VK_CHECK_RESULT(vkEndCommandBuffer(ctx->cmd));
    _submit_ctx_cmd(ctx);

    ctx->stencilRef = sav->stencilRef;
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

    ctx->curPos     = sav->curPos;
    ctx->curRGBA    = sav->curRGBA;
    ctx->lineWidth  = sav->lineWidth;

    ctx->selectedFont.charSize = sav->selectedFont.charSize;
    strcpy (ctx->selectedFont.fontFile, sav->selectedFont.fontFile);

    ctx->currentFont  = sav->currentFont;
    ctx->textDirection= sav->textDirection;
    ctx->pushConsts   = sav->pushConsts;
    ctx->source       = sav->source;

    _wait_and_reset_ctx_cmd (ctx);
    _init_cmd_buff          (ctx);

    _free_ctx_save(sav);
}

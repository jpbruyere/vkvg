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
#ifndef VKVG_CONTEXT_INTERNAL_H
#define VKVG_CONTEXT_INTERNAL_H

#include "vkvg_internal.h"
#include "vkvg.h"
#include "vkvg_buff.h"
#include "vkh.h"
#include "vkvg_fonts.h"

#define VKVG_PTS_SIZE				4096
#define VKVG_VBO_SIZE				4096 * 8
#define VKVG_IBO_SIZE				VKVG_VBO_SIZE * 6
#define VKVG_PATHES_SIZE			16
#define VKVG_ARRAY_THRESHOLD		4

typedef struct{
    vec2 pos;
    vec3 uv;
}Vertex;

typedef struct {
    vec4            source;
    vec2            size;
    uint32_t        patternType;
    uint32_t        fullScreenQuad;
    vkvg_matrix_t   mat;
    vkvg_matrix_t   matInv;
}push_constants;

typedef struct _vkvg_context_save_t{
    struct _vkvg_context_save_t* pNext;

    float       lineWidth;

    vkvg_operator_t     curOperator;
    vkvg_line_cap_t     lineCap;
    vkvg_line_join_t    lineJoint;
    vkvg_fill_rule_t    curFillRule;

    _vkvg_font_t        selectedFont;     //hold current face and size before cache addition
    _vkvg_font_t*       currentFont;      //font ready for lookup
    vkvg_direction_t    textDirection;
    push_constants      pushConsts;
    VkvgPattern         pattern;

}vkvg_context_save_t;

typedef struct _vkvg_context_t {
    VkvgContext         pPrev;      //double linked list of contexts
    VkvgContext         pNext;
    uint32_t            references; //reference count

    VkvgSurface         pSurf;      //surface bound to context, set on creation of ctx
    VkFence             flushFence; //context fence
    VkhImage            source;     //source of painting operation

    VkCommandPool		cmdPool;    //local pools ensure thread safety
    VkCommandBuffer     cmd;        //single cmd buff for context operations
    bool                cmdStarted; //prevent flushing empty renderpass
    bool                pushCstDirty;//prevent pushing to gpu if not requested
    VkDescriptorPool	descriptorPool;//one pool per thread
    VkDescriptorSet     dsFont;     //fonts glyphs texture atlas descriptor (local for thread safety)
    VkDescriptorSet     dsSrc;      //source ds
    VkDescriptorSet     dsGrad;     //gradient uniform buffer

    VkRect2D            bounds;

    float xMin;
    float xMax;
    float yMin;
    float yMax;

    vkvg_buff	uboGrad;        //uniform buff obj holdings gradient infos

    //vk buffers, holds data until flush
    vkvg_buff	indices;        //index buffer with persistent map memory
    size_t		sizeIndices;    //reserved size
    uint32_t	indCount;       //current indice count

    uint32_t	curIndStart;    //last index recorded in cmd buff

    vkvg_buff	vertices;       //vertex buffer with persistent mapped memory
    size_t		sizeVertices;   //reserved size
    uint32_t	vertCount;      //effective vertices count

    //pathes, exists until stroke of fill
    vec2*		points;         //points array
    size_t		sizePoints;     //reserved size
    uint32_t	pointCount;     //effective points count

    //pathes array is a list of couple (start,end) point idx refering to point array
    //it split points list in subpathes and tell if path is closed.
    //if path is closed, end index is the same as start.
    //(TODO: I should use a boolean or smthg else instead to keep last point in array)
    uint32_t	pathPtr;        //pointer in the path array, even=start point;odd=end point
    uint32_t*	pathes;
    size_t		sizePathes;

    //if current path contains curves, start/end points are store next to the path start/stop
    //curve start point = pathPtr + curvePtr
    //when closing of finishing path, pathPtr is incremented by 1 + pathPtr
    //note:number of pathes can no longuer be computed from pathPtr/2, the array contains now curves datas
    uint32_t    curvePtr;

    float		lineWidth;

    vkvg_operator_t     curOperator;
    vkvg_line_cap_t     lineCap;
    vkvg_line_join_t    lineJoin;
    vkvg_fill_rule_t    curFillRule;

    _vkvg_font_t        selectedFont;     //hold current face and size before cache addition
    _vkvg_font_t*       currentFont;      //font pointing to cached fonts ready for lookup
    vkvg_direction_t    textDirection;

    push_constants      pushConsts;
    VkvgPattern         pattern;
    vkvg_status_t       status;

    vkvg_context_save_t* pSavedCtxs;        //last ctx saved ptr
    uint8_t             curSavBit;          //current stencil bit used to save context, 6 bits used by stencil for save/restore
    VkhImage*           savedStencils;      //additional image for saving contexes once more than 6 save/restore are reached

    VkClearRect         clearRect;
    VkRenderPassBeginInfo renderPassBeginInfo;
}vkvg_context;

typedef struct _ear_clip_point{
    vec2 pos;
    uint32_t idx;
    struct _ear_clip_point* next;
}ear_clip_point;

bool _current_path_is_empty (VkvgContext ctx);
void _start_sub_path        (VkvgContext ctx, float x, float y);
void _check_pathes_array	(VkvgContext ctx);
void _finish_path			(VkvgContext ctx);
void _clear_path			(VkvgContext ctx);
bool _path_is_closed		(VkvgContext ctx, uint32_t ptrPath);
void _set_curve_start       (VkvgContext ctx);
void _set_curve_end         (VkvgContext ctx);
bool _path_has_curves       (VkvgContext ctx, uint ptrPath);

float _normalizeAngle       (float a);

vec2 _get_current_position  (VkvgContext ctx);
void _add_point         	(VkvgContext ctx, float x, float y);

void _resetMinMax           (VkvgContext ctx);

void _poly_fill             (VkvgContext ctx);
void _fill_ec               (VkvgContext ctx);//earclipping fill
void _draw_full_screen_quad (VkvgContext ctx, bool useScissor);

void _create_gradient_buff  (VkvgContext ctx);
void _create_vertices_buff	(VkvgContext ctx);
void _add_vertex			(VkvgContext ctx, Vertex v);
void _add_vertexf           (VkvgContext ctx, float x, float y);
void _set_vertex			(VkvgContext ctx, uint32_t idx, Vertex v);
void _add_triangle_indices	(VkvgContext ctx, uint32_t i0, uint32_t i1,uint32_t i2);
void _add_tri_indices_for_rect	(VkvgContext ctx, uint32_t i);
void _build_vb_step         (vkvg_context* ctx, Vertex v, float hw, uint32_t iL, uint32_t i, uint32_t iR, bool isCurve);
void _vao_add_rectangle     (VkvgContext ctx, float x, float y, float width, float height);

void _bind_draw_pipeline    (VkvgContext ctx);
void _create_cmd_buff		(VkvgContext ctx);
void _check_cmd_buff_state  (VkvgContext ctx);
void _flush_cmd_buff		(VkvgContext ctx);
void _record_draw_cmd		(VkvgContext ctx);
void _submit_wait_and_reset_cmd(VkvgContext ctx);
void _submit_ctx_cmd        (VkvgContext ctx);
void _wait_and_reset_ctx_cmd(VkvgContext ctx);
void _update_push_constants (VkvgContext ctx);
void _update_cur_pattern    (VkvgContext ctx, VkvgPattern pat);
void _set_mat_inv_and_vkCmdPush (VkvgContext ctx);
void _start_cmd_for_render_pass (VkvgContext ctx);

void _createDescriptorPool  (VkvgContext ctx);
void _init_descriptor_sets  (VkvgContext ctx);
void _update_descriptor_set (VkvgContext ctx, VkhImage img, VkDescriptorSet ds);
void _update_gradient_desc_set(VkvgContext ctx);
void _free_ctx_save         (vkvg_context_save_t* sav);

static inline float vec2_zcross (vec2 v1, vec2 v2){
    return v1.x*v2.y-v1.y*v2.x;
}
static inline float ecp_zcross (ear_clip_point* p0, ear_clip_point* p1, ear_clip_point* p2){
    return vec2_zcross (vec2_sub (p1->pos, p0->pos), vec2_sub (p2->pos, p0->pos));
}
void _recursive_bezier(VkvgContext ctx,
                       float x1, float y1, float x2, float y2,
                       float x3, float y3, float x4, float y4,
                       unsigned level);
void _bezier (VkvgContext ctx,
              float x1, float y1, float x2, float y2,
              float x3, float y3, float x4, float y4);
#endif

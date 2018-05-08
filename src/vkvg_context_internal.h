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

#define VKVG_PTS_SIZE				16384
#define VKVG_VBO_SIZE				VKVG_PTS_SIZE * 2
#define VKVG_IBO_SIZE				VKVG_VBO_SIZE * 2
#define VKVG_PATHES_SIZE			256
#define VKVG_ARRAY_THRESHOLD		4

#define ROUND_DOWN(v,p) (floorf(v * p) / p)

typedef struct{
    vec2 pos;
    vec3 uv;
}Vertex;

typedef struct {
    vec4            source;
    vec2            size;
    uint32_t        patternType;
    uint32_t        pad;
    vkvg_matrix_t   mat;
    vkvg_matrix_t   matInv;
}push_constants;

typedef struct _vkvg_context_save_t{
    struct _vkvg_context_save_t* pNext;

    VkhImage    stencilMS;
    uint32_t    stencilRef;
    vec2*		points;     //points array
    size_t		sizePoints; //reserved size
    uint32_t	pointCount; //effective points count

    uint32_t	pathPtr;
    uint32_t*	pathes;
    size_t		sizePathes;

    float       lineWidth;

    vkvg_operator_t     curOperator;
    vkvg_line_cap_t     lineCap;
    vkvg_line_join_t    lineJoint;

    _vkvg_font_t    selectedFont;     //hold current face and size before cache addition
    _vkvg_font_t*   currentFont;      //font ready for lookup
    vkvg_direction_t   textDirection;
    push_constants  pushConsts;
    VkvgPattern     pattern;

}vkvg_context_save_t;

typedef struct _vkvg_context_t {
    VkvgContext     pPrev;      //double linked list of contexts
    VkvgContext     pNext;
    uint32_t        references;

    VkvgSurface		pSurf;
    VkFence			flushFence;
    VkhImage        source;     //source of painting operation

    VkCommandPool		cmdPool;//local pools ensure thread safety
    VkCommandBuffer     cmd;    //single cmd buff for context operations
    VkDescriptorPool	descriptorPool;
    VkDescriptorSet     dsFont; //fonts glyphs texture atlas descriptor (local for thread safety)
    VkDescriptorSet     dsSrc;  //source ds
    VkDescriptorSet     dsGrad; //gradient uniform buffer

    vkvg_buff	uboGrad;//uniform buff obj holdings gradient infos

    //vk buffers, holds data until flush
    vkvg_buff	indices;        //index buffer with persistent map memory
    size_t		sizeIndices;    //reserved size
    uint32_t	indCount;       //current indice count

    uint32_t	curIndStart;

    vkvg_buff	vertices;       //vertex buffer with persistent mapped memory
    size_t		sizeVertices;   //reserved size
    uint32_t	vertCount;      //effective vertices count

    //pathes, exists until stroke of fill
    vec2*		points;     //points array
    size_t		sizePoints; //reserved size
    uint32_t	pointCount; //effective points count

    uint32_t	pathPtr;
    //pathes array is a list of couple (start,end) point idx refering to point array
    //it split points list in subpathes and tell if path is closed.
    //if path is closed, end index is the same as start.
    //(TODO: I should use a boolean or smthg else instead to keep last point in array)
    uint32_t*	pathes;
    size_t		sizePathes;

    float		lineWidth;

    vkvg_operator_t     curOperator;
    vkvg_line_cap_t     lineCap;
    vkvg_line_join_t    lineJoin;

    _vkvg_font_t  selectedFont;     //hold current face and size before cache addition
    _vkvg_font_t* currentFont;      //font ready for lookup
    vkvg_direction_t textDirection;

    push_constants  pushConsts;
    VkvgPattern     pattern;

    vkvg_context_save_t* pSavedCtxs;//last ctx saved ptr
}vkvg_context;

bool _current_path_is_empty (VkvgContext ctx);
void _start_sub_path        (VkvgContext ctx, float x, float y);
void _check_pathes_array	(VkvgContext ctx);
void _finish_path			(VkvgContext ctx);
void _clear_path			(VkvgContext ctx);
bool _path_is_closed		(VkvgContext ctx, uint32_t ptrPath);
uint32_t _get_last_point_of_closed_path (VkvgContext ctx, uint32_t ptrPath);

float _normalizeAngle       (float a);

vec2 _get_current_position  (VkvgContext ctx);
void _add_point         	(VkvgContext ctx, float x, float y);
void _add_point_vec2			(VkvgContext ctx, vec2 v);

void _create_vertices_buff	(VkvgContext ctx);
void _add_vertex			(VkvgContext ctx, Vertex v);
void _add_vertexf           (VkvgContext ctx, float x, float y);
void _set_vertex			(VkvgContext ctx, uint32_t idx, Vertex v);
void _add_triangle_indices	(VkvgContext ctx, uint32_t i0, uint32_t i1,uint32_t i2);
void _add_tri_indices_for_rect	(VkvgContext ctx, uint32_t i);
void _build_vb_step         (vkvg_context* ctx, Vertex v, float hw, uint32_t iL, uint32_t i, uint32_t iR);
void _vao_add_rectangle     (VkvgContext ctx, float x, float y, float width, float height);

void _bind_draw_pipeline    (VkvgContext ctx);
void _create_cmd_buff		(VkvgContext ctx);
void _init_cmd_buff			(VkvgContext ctx);
void _flush_cmd_buff		(VkvgContext ctx);
void _record_draw_cmd		(VkvgContext ctx);
void _submit_wait_and_reset_cmd(VkvgContext ctx);
void _submit_ctx_cmd        (VkvgContext ctx);
void _wait_and_reset_ctx_cmd(VkvgContext ctx);
void _update_push_constants (VkvgContext ctx);
void _update_cur_pattern    (VkvgContext ctx, VkvgPattern pat);
void _set_mat_inv_and_vkCmdPush (VkvgContext ctx);

void _createDescriptorPool  (VkvgContext ctx);
void _init_descriptor_sets  (VkvgContext ctx);
void _update_descriptor_set (VkvgContext ctx, VkhImage img, VkDescriptorSet ds);
void _update_gradient_desc_set(VkvgContext ctx);
void _reset_src_descriptor_set(VkvgContext ctx);
void _free_ctx_save         (vkvg_context_save_t* sav);

static inline float vec2_zcross (vec2 v1, vec2 v2){
    return v1.x*v2.y-v1.y*v2.x;
}
void _recursive_bezier(VkvgContext ctx,
                       float x1, float y1, float x2, float y2,
                       float x3, float y3, float x4, float y4,
                       unsigned level);
void _bezier (VkvgContext ctx,
              float x1, float y1, float x2, float y2,
              float x3, float y3, float x4, float y4);
#endif

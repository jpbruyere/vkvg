#ifndef VKVG_CONTEXT_INTERNAL_H
#define VKVG_CONTEXT_INTERNAL_H

#include "vkvg_internal.h"
#include "vkvg.h"
#include "vkvg_buff.h"
#include "vkh.h"
#include "vkvg_fonts.h"

#define VKVG_PTS_SIZE				4096
#define VKVG_VBO_SIZE				VKVG_PTS_SIZE * 2
#define VKVG_IBO_SIZE				VKVG_VBO_SIZE * 2
#define VKVG_PATHES_SIZE			128
#define VKVG_ARRAY_THRESHOLD		4

#define ROUND_DOWN(v,p) (floorf(v * p) / p)

typedef struct{
    vec2 pos;
    vec3 uv;
}Vertex;

typedef struct _ear_clip_point{
    vec2 pos;
    uint32_t idx;
    struct _ear_clip_point* next;
}ear_clip_point;

#define VKVG_SRC_SOLID      0
#define VKVG_SRC_PATTERN    1

typedef struct {
    vec4    source;
    vec2    scale;
    vec2    translate;
    int     srcType;
}push_constants;

typedef struct _vkvg_context_save_t{
    struct _vkvg_context_save_t* pNext;

    VkhImage    source;
    VkhImage    stencilMS;
    uint32_t    stencilRef;
    vec2*		points;     //points array
    size_t		sizePoints; //reserved size
    uint32_t	pointCount; //effective points count

    uint32_t	pathPtr;
    uint32_t*	pathes;
    size_t		sizePathes;

    vec2		curPos;
    vec4		curRGBA;
    float       lineWidth;

    _vkvg_font_t    selectedFont;     //hold current face and size before cache addition
    _vkvg_font_t*   currentFont;      //font ready for lookup
    vkvg_direction_t   textDirection;
    push_constants  pushConsts;

}vkvg_context_save_t;

typedef struct _vkvg_context_t {
    VkvgContext     pPrev;      //double linked list of contexts
    VkvgContext     pNext;

    VkvgSurface		pSurf;
    VkFence			flushFence;
    uint32_t        stencilRef;
    VkhImage        source;

    VkCommandPool		cmdPool;
    VkCommandBuffer     cmd;
    VkDescriptorPool	descriptorPool;
    VkDescriptorSet     dsFont;
    VkDescriptorSet     dsSrc;

    VkCommandPool		cmdPoolCompute;
    VkFence             computeFence;

    //vk buffers, holds data until flush
    vkvg_buff	indices;
    size_t		sizeIndices;
    uint32_t	indCount;

    uint32_t	curIndStart;

    vkvg_buff	vertices;
    size_t		sizeVertices;
    uint32_t	vertCount;

    //pathes, exists until stroke of fill
    vec2*		points;     //points array
    size_t		sizePoints; //reserved size
    uint32_t	pointCount; //effective points count

    uint32_t	pathPtr;
    uint32_t*	pathes;
    size_t		sizePathes;

    vec2		curPos;
    vec4		curRGBA;
    float		lineWidth;

    _vkvg_font_t  selectedFont;     //hold current face and size before cache addition
    _vkvg_font_t* currentFont;      //font ready for lookup
    vkvg_direction_t textDirection;

    push_constants  pushConsts;

    vkvg_context_save_t* pSavedCtxs;//last ctx saved ptr
}vkvg_context;

void _check_pathes_array	(VkvgContext ctx);
float _normalizeAngle       (float a);

void _add_point         	(VkvgContext ctx, float x, float y);
void _add_point_cp_update	(VkvgContext ctx, float x, float y);
void _add_point_v2			(VkvgContext ctx, vec2 v);
void _add_curpos			(VkvgContext ctx);
void _vkvg_fill_rectangle   (VkvgContext ctx, float x, float y, float width, float height);

void _create_vertices_buff	(VkvgContext ctx);
void _add_vertex			(VkvgContext ctx, Vertex v);
void _set_vertex			(VkvgContext ctx, uint32_t idx, Vertex v);
void _add_triangle_indices	(VkvgContext ctx, uint32_t i0, uint32_t i1,uint32_t i2);
void _add_tri_indices_for_rect	(VkvgContext ctx, uint32_t i);

void _create_cmd_buff		(VkvgContext ctx);
void _init_cmd_buff			(VkvgContext ctx);
void _flush_cmd_buff		(VkvgContext ctx);
void _record_draw_cmd		(VkvgContext ctx);
void _submit_wait_and_reset_cmd(VkvgContext ctx);
void _submit_ctx_cmd        (VkvgContext ctx);
void _wait_and_reset_ctx_cmd(VkvgContext ctx);

void _finish_path			(VkvgContext ctx);
void _clear_path			(VkvgContext ctx);
bool _path_is_closed		(VkvgContext ctx, uint32_t ptrPath);
uint32_t _get_last_point_of_closed_path (VkvgContext ctx, uint32_t ptrPath);

void _createDescriptorPool  (VkvgContext ctx);
void _init_descriptor_sets  (VkvgContext ctx);
void _update_descriptor_set (VkvgContext ctx, VkhImage img, VkDescriptorSet ds);
void _reset_src_descriptor_set(VkvgContext ctx);
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

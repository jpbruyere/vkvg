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
#ifndef VKVG_CONTEXT_INTERNAL_H
#define VKVG_CONTEXT_INTERNAL_H

#include "vkvg_internal.h"
#include "vkvg.h"
#include "vkvg_buff.h"
#include "vkh.h"
#include "vkvg_fonts.h"

#define VKVG_PTS_SIZE				1024
#define VKVG_VBO_SIZE				(VKVG_PTS_SIZE * 4)
#define VKVG_IBO_SIZE				(VKVG_VBO_SIZE * 6)
#define VKVG_PATHES_SIZE			16
#define VKVG_ARRAY_THRESHOLD		8

#define VKVG_IBO_16					0
#define VKVG_IBO_32					1

#define VKVG_CUR_IBO_TYPE			VKVG_IBO_16//change this only

#if VKVG_CUR_IBO_TYPE == VKVG_IBO_16
	#define VKVG_IBO_MAX			UINT16_MAX
	#define VKVG_IBO_INDEX_TYPE		uint16_t
	#define VKVG_VK_INDEX_TYPE		VK_INDEX_TYPE_UINT16
#else
	#define VKVG_IBO_MAX			UINT32_MAX
	#define VKVG_IBO_INDEX_TYPE		uint32_t
	#define VKVG_VK_INDEX_TYPE		VK_INDEX_TYPE_UINT32
#endif



#define CreateRgba(r, g, b, a) ((a << 24) | (r << 16) | (g << 8) | b)
#ifdef VKVG_PREMULT_ALPHA
	#define CreateRgbaf(r, g, b, a) (((int)(a * 255.0f) << 24) | ((int)(b * a * 255.0f) << 16) | ((int)(g * a * 255.0f) << 8) | (int)(r * a * 255.0f))
#else
	#define CreateRgbaf(r, g, b, a) (((int)(a * 255.0f) << 24) | ((int)(b * 255.0f) << 16) | ((int)(g * 255.0f) << 8) | (int)(r * 255.0f))
#endif

typedef struct{
	vec2 pos;
	uint32_t color;
	vec3 uv;
}Vertex;

typedef struct {
	vec4			source;
	vec2			size;
	uint32_t		patternType;
	uint32_t		fullScreenQuad;
	vkvg_matrix_t	mat;
	vkvg_matrix_t	matInv;
}push_constants;

typedef struct _vkvg_context_save_t{
	struct _vkvg_context_save_t* pNext;

	float		lineWidth;
	uint32_t	dashCount;		//value count in dash array, 0 if dash not set.
	float		dashOffset;		//an offset for dash
	float*		dashes;			//an array of alternate lengths of on and off stroke.


	vkvg_operator_t		curOperator;
	vkvg_line_cap_t		lineCap;
	vkvg_line_join_t	lineJoint;
	vkvg_fill_rule_t	curFillRule;

	long				selectedCharSize; /* Font size*/
	char*				selectedFontName;
	_vkvg_font_identity_t		 selectedFont;	   //hold current face and size before cache addition
	_vkvg_font_identity_t*		 currentFont;	   //font ready for lookup
	vkvg_direction_t	textDirection;
	push_constants		pushConsts;
	VkvgPattern			pattern;

}vkvg_context_save_t;

typedef struct _vkvg_context_t {
	VkvgContext			pPrev;		//double linked list of contexts
	VkvgContext			pNext;
	uint32_t			references; //reference count

	VkvgSurface			pSurf;		//surface bound to context, set on creation of ctx
	VkFence				flushFence; //context fence
	VkhImage			source;		//source of painting operation

	VkCommandPool		cmdPool;	//local pools ensure thread safety
	VkCommandBuffer		cmdBuffers[2];//double cmd buff for context operations
	VkCommandBuffer		cmd;		//current recording buffer
	bool				cmdStarted; //prevent flushing empty renderpass
	bool				pushCstDirty;//prevent pushing to gpu if not requested
	VkDescriptorPool	descriptorPool;//one pool per thread
	VkDescriptorSet		dsFont;		//fonts glyphs texture atlas descriptor (local for thread safety)
	VkDescriptorSet		dsSrc;		//source ds
	VkDescriptorSet		dsGrad;		//gradient uniform buffer

	VkRect2D			bounds;

	uint32_t			curColor;

	/*float xMin;
	float xMax;
	float yMin;
	float yMax;*/

	vkvg_buff	uboGrad;		//uniform buff obj holdings gradient infos

	//vk buffers, holds data until flush
	vkvg_buff	indices;		//index buffer with persistent map memory
	uint32_t	sizeIBO;		//size of vk ibo
	uint32_t	sizeIndices;	//reserved size
	uint32_t	indCount;		//current indice count

	uint32_t	curIndStart;	//last index recorded in cmd buff
	uint32_t	curVertOffset;	//vertex offset in draw indexed command

	vkvg_buff	vertices;		//vertex buffer with persistent mapped memory
	uint32_t	sizeVBO;		//size of vk vbo size
	uint32_t	sizeVertices;	//reserved size
	uint32_t	vertCount;		//effective vertices count

	Vertex*		vertexCache;
	VKVG_IBO_INDEX_TYPE* indexCache;

	//pathes, exists until stroke of fill
	vec2*		points;			//points array
	uint32_t	sizePoints;		//reserved size
	uint32_t	pointCount;		//effective points count

	//pathes array is a list of point count per segment
	uint32_t	pathPtr;		//pointer in the path array
	uint32_t*	pathes;
	uint32_t	sizePathes;

	uint32_t	segmentPtr;		//current segment count in current path having curves

	float		lineWidth;
	uint32_t	dashCount;		//value count in dash array, 0 if dash not set.
	float		dashOffset;		//an offset for dash
	float*		dashes;			//an array of alternate lengths of on and off stroke.

	vkvg_operator_t		curOperator;
	vkvg_line_cap_t		lineCap;
	vkvg_line_join_t	lineJoin;
	vkvg_fill_rule_t	curFillRule;

	long				selectedCharSize; /* Font size*/
	char*				selectedFontName;
	//_vkvg_font_t		  selectedFont;		//hold current face and size before cache addition
	_vkvg_font_identity_t*		 currentFont;		//font pointing to cached fonts identity
	_vkvg_font_t*		currentFontSize;	//font structure by size ready for lookup
	vkvg_direction_t	textDirection;

	push_constants		pushConsts;
	VkvgPattern			pattern;
	vkvg_status_t		status;

	vkvg_context_save_t* pSavedCtxs;		//last ctx saved ptr
	uint8_t				curSavBit;			//current stencil bit used to save context, 6 bits used by stencil for save/restore
	VkhImage*			savedStencils;		//additional image for saving contexes once more than 6 save/restore are reached

	VkClearRect			clearRect;
	VkRenderPassBeginInfo renderPassBeginInfo;
}vkvg_context;

typedef struct _ear_clip_point{
	vec2 pos;
	VKVG_IBO_INDEX_TYPE idx;
	struct _ear_clip_point* next;
}ear_clip_point;

typedef struct {
	bool	dashOn;
	uint32_t curDash;	//current dash index
	float	curDashOffset;	//cur dash offset between defined path point and last dash segment(on/off) start
	float	totDashLength;	//total length of dashes
	vec2	normal;
}dash_context_t;

typedef struct {
	uint32_t iL;
	uint32_t iR;
	uint32_t cp;//current point
	VKVG_IBO_INDEX_TYPE firstIdx;//save first point idx for closed path
}stroke_context_t;

void _check_vertex_cache_size(VkvgContext ctx);
void _resize_vertex_cache	(VkvgContext ctx, uint32_t newSize);
void _check_index_cache_size(VkvgContext ctx);
bool _check_pathes_array	(VkvgContext ctx);

bool _current_path_is_empty (VkvgContext ctx);
void _finish_path			(VkvgContext ctx);
void _clear_path			(VkvgContext ctx);
void _remove_last_point		(VkvgContext ctx);
bool _path_is_closed		(VkvgContext ctx, uint32_t ptrPath);
void _set_curve_start		(VkvgContext ctx);
void _set_curve_end			(VkvgContext ctx);
bool _path_has_curves		(VkvgContext ctx, uint32_t ptrPath);

float _normalizeAngle		(float a);
float _get_arc_step			(VkvgContext ctx, float radius);

vec2 _get_current_position	(VkvgContext ctx);
void _add_point				(VkvgContext ctx, float x, float y);

void _resetMinMax			(VkvgContext ctx);
void _vkvg_path_extents		(VkvgContext ctx, bool transformed, float *x1, float *y1, float *x2, float *y2);
void _draw_stoke_cap		(VkvgContext ctx, float hw, vec2 p0, vec2 n, bool isStart);
void _draw_segment			(VkvgContext ctx, float hw, stroke_context_t* str, dash_context_t* dc, bool isCurve);
float _draw_dashed_segment	(VkvgContext ctx, float hw, stroke_context_t *str, dash_context_t* dc, bool isCurve);

void _poly_fill				(VkvgContext ctx);
void _fill_ec				(VkvgContext ctx);//earclipping fill
void _draw_full_screen_quad (VkvgContext ctx, bool useScissor);

void _create_gradient_buff	(VkvgContext ctx);
void _create_vertices_buff	(VkvgContext ctx);
void _add_vertex			(VkvgContext ctx, Vertex v);
void _add_vertexf			(VkvgContext ctx, float x, float y);
void _set_vertex			(VkvgContext ctx, uint32_t idx, Vertex v);
void _add_triangle_indices	(VkvgContext ctx, VKVG_IBO_INDEX_TYPE i0, VKVG_IBO_INDEX_TYPE i1, VKVG_IBO_INDEX_TYPE i2);
void _add_tri_indices_for_rect	(VkvgContext ctx, VKVG_IBO_INDEX_TYPE i);
bool _build_vb_step		(vkvg_context* ctx, float hw, stroke_context_t *str, bool isCurve);

void _vao_add_rectangle		(VkvgContext ctx, float x, float y, float width, float height);

void _bind_draw_pipeline	(VkvgContext ctx);
void _create_cmd_buff		(VkvgContext ctx);
void _check_vao_size		(VkvgContext ctx);
void _ensure_renderpass_is_started	(VkvgContext ctx);
void _flush_cmd_buff		(VkvgContext ctx);
void _emit_draw_cmd_undrawn_vertices(VkvgContext ctx);
void _flush_cmd_until_vx_base (VkvgContext ctx);
bool _wait_flush_fence		(VkvgContext ctx);
void _reset_flush_fence		(VkvgContext ctx);
bool _wait_and_submit_cmd	(VkvgContext ctx);
void _update_push_constants (VkvgContext ctx);
void _update_cur_pattern	(VkvgContext ctx, VkvgPattern pat);
void _set_mat_inv_and_vkCmdPush (VkvgContext ctx);
void _start_cmd_for_render_pass (VkvgContext ctx);

void _createDescriptorPool	(VkvgContext ctx);
void _init_descriptor_sets	(VkvgContext ctx);
void _update_descriptor_set (VkvgContext ctx, VkhImage img, VkDescriptorSet ds);
void _update_gradient_desc_set(VkvgContext ctx);
void _free_ctx_save			(vkvg_context_save_t* sav);

static inline float vec2_zcross (vec2 v1, vec2 v2){
	return v1.x*v2.y-v1.y*v2.x;
}
static inline float ecp_zcross (ear_clip_point* p0, ear_clip_point* p1, ear_clip_point* p2){
	return vec2_zcross (vec2_sub (p1->pos, p0->pos), vec2_sub (p2->pos, p0->pos));
}
void _recursive_bezier(VkvgContext ctx, float distanceTolerance,
					   float x1, float y1, float x2, float y2,
					   float x3, float y3, float x4, float y4,
					   unsigned level);
void _bezier (VkvgContext ctx,
			  float x1, float y1, float x2, float y2,
			  float x3, float y3, float x4, float y4);
#endif

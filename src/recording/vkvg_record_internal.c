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
#include "vkvg.h"
#include "vkvg_record_internal.h"
#include "vkvg_context_internal.h"

#define VKVG_RECORDING_INIT_BUFFER_SIZE_TRESHOLD	64
#define VKVG_RECORDING_INIT_BUFFER_SIZE				1024
#define VKVG_RECORDING_INIT_COMMANDS_COUNT			64

vkvg_recording_t* _new_recording () {

	vkvg_recording_t* rec = (vkvg_recording_t*)calloc(1,sizeof (vkvg_recording_t));

	rec->commandsReservedCount	= VKVG_RECORDING_INIT_COMMANDS_COUNT;
	rec->bufferReservedSize		= VKVG_RECORDING_INIT_BUFFER_SIZE;
	rec->commands = (vkvg_record_t*)malloc(rec->commandsReservedCount * sizeof (vkvg_record_t));
	rec->buffer = malloc (rec->bufferReservedSize);

	return rec;
}
void _destroy_recording (vkvg_recording_t* rec) {
	free(rec->commands);
	free(rec->buffer);
	free(rec);
}
void _start_recording (VkvgContext ctx) {
	if (ctx->recording)
		_destroy_recording(ctx->recording);
	ctx->recording = _new_recording();
}
vkvg_recording_t* _stop_recording (VkvgContext ctx) {
	vkvg_recording_t* rec = ctx->recording;
	if (!rec)
		return NULL;
	if (!rec->commandsCount) {
		_destroy_recording(rec);
		ctx->recording = NULL;
		return NULL;
	}
	/*rec->buffer = realloc(rec->buffer, rec->bufferSize);
	rec->commands = (vkvg_record_t*)realloc(rec->commands, rec->commandsCount * sizeof (vkvg_record_t));*/
	ctx->recording = NULL;
	return rec;
}
void* _ensure_recording_buffer (vkvg_recording_t* rec, size_t size) {
	if (rec->bufferReservedSize >= rec->bufferSize - VKVG_RECORDING_INIT_BUFFER_SIZE_TRESHOLD - size) {
		rec->bufferReservedSize += VKVG_RECORDING_INIT_BUFFER_SIZE;
		rec->buffer = realloc(rec->buffer, rec->bufferReservedSize);
	}
	return rec->buffer + rec->bufferSize;
}
void* _advance_recording_buffer_unchecked (vkvg_recording_t* rec, size_t size) {
	rec->bufferSize += size;
	return rec->buffer + rec->bufferSize;
}

#define STORE_FLOATS(floatcount)										\
	for (i=0; i<floatcount; i++) {										\
		buff = _ensure_recording_buffer (rec, sizeof(float));			\
		*(float*)buff = (float)va_arg(args, double);					\
		buff = _advance_recording_buffer_unchecked (rec, sizeof(float));\
	}
#define STORE_BOOLS(floatcount)											\
	for (i=0; i<floatcount; i++) {										\
		buff = _ensure_recording_buffer (rec, sizeof(bool));			\
		*(bool*)buff = (bool)va_arg(args, int);							\
		buff = _advance_recording_buffer_unchecked (rec, sizeof(bool));	\
	}


void _record (vkvg_recording_t* rec,...) {
	va_list args;
	va_start(args, rec);

	uint32_t cmd = va_arg(args, uint32_t);

	if (rec->commandsCount == rec->commandsReservedCount) {
		rec->commandsReservedCount += VKVG_RECORDING_INIT_COMMANDS_COUNT;
		rec->commands = (vkvg_record_t*)realloc(rec->commands, rec->commandsReservedCount * sizeof (vkvg_record_t));
	}
	vkvg_record_t* r = &rec->commands[rec->commandsCount++];
	r->cmd = cmd;
	r->dataOffset = rec->bufferSize;

	void* buff;


	int i = 0;
	switch (cmd) {
	case VKVG_CMD_MOVE_TO:
	case VKVG_CMD_LINE_TO:
	case VKVG_CMD_REL_MOVE_TO:
	case VKVG_CMD_REL_LINE_TO:
		STORE_FLOATS(2);
		break;
	case VKVG_CMD_SAVE:
	case VKVG_CMD_RESTORE:
	case VKVG_CMD_NEW_PATH:
	case VKVG_CMD_NEW_SUB_PATH:
	case VKVG_CMD_CLOSE_PATH:
	case VKVG_CMD_PAINT:
	case VKVG_CMD_FILL:
	case VKVG_CMD_STROKE:
	case VKVG_CMD_CLIP:
	case VKVG_CMD_CLEAR:
	case VKVG_CMD_FILL_PRESERVE:
	case VKVG_CMD_STROKE_PRESERVE:
	case VKVG_CMD_CLIP_PRESERVE:
		break;
	case VKVG_CMD_RECTANGLE:
	//case VKVG_CMD_ELLIPSE:
	case VKVG_CMD_SET_SOURCE_RGBA:
	case VKVG_CMD_QUADRATIC_TO:
	case VKVG_CMD_REL_QUADRATIC_TO:
		STORE_FLOATS(4);
		break;
	case VKVG_CMD_ARC:
	case VKVG_CMD_ARC_NEG:
		STORE_FLOATS(5);
		break;
	case VKVG_CMD_CURVE_TO:
	case VKVG_CMD_REL_CURVE_TO:
		STORE_FLOATS(6);
		break;
	case VKVG_CMD_ELLIPTICAL_ARC_TO:
		STORE_FLOATS(5);
		STORE_BOOLS(2);
		break;
	case VKVG_CMD_SET_SOURCE_RGB:
		STORE_FLOATS(3);
		break;
	case VKVG_CMD_REL_ELLIPTICAL_ARC_TO:
		STORE_FLOATS(5);
		STORE_BOOLS(2);
		break;
	case VKVG_CMD_SET_SOURCE:
		buff = _ensure_recording_buffer (rec, sizeof(VkvgPattern));
		*(VkvgPattern*)buff = va_arg(args, VkvgPattern);
		_advance_recording_buffer_unchecked (rec, sizeof(VkvgPattern));
		break;
	case VKVG_CMD_SET_SOURCE_SURFACE:
		STORE_FLOATS(2);
		buff = _ensure_recording_buffer (rec, sizeof(VkvgSurface));
		*(VkvgSurface*)buff = va_arg(args, VkvgSurface);
		_advance_recording_buffer_unchecked (rec, sizeof(VkvgSurface));
		break;
	}
	va_end(args);
}
void _replay_command (VkvgContext ctx, VkvgRecording rec, uint32_t index) {
	vkvg_record_t* r = &rec->commands[index];
	float* floats = (float*)(rec->buffer + r->dataOffset);
	switch (r->cmd) {
	case VKVG_CMD_SAVE:
		vkvg_save (ctx);
		break;
	case VKVG_CMD_RESTORE:
		vkvg_restore (ctx);
		break;
	case VKVG_CMD_NEW_PATH:
		vkvg_new_path (ctx);
		break;
	case VKVG_CMD_NEW_SUB_PATH:
		vkvg_new_sub_path (ctx);
		break;
	case VKVG_CMD_CLOSE_PATH:
		vkvg_close_path (ctx);
		break;
	case VKVG_CMD_PAINT:
		vkvg_paint (ctx);
		break;
	case VKVG_CMD_FILL:
		vkvg_fill (ctx);
		break;
	case VKVG_CMD_STROKE:
		vkvg_stroke (ctx);
		break;
	case VKVG_CMD_CLIP:
		vkvg_clip (ctx);
		break;
	case VKVG_CMD_CLEAR:
		vkvg_clear (ctx);
		break;
	case VKVG_CMD_FILL_PRESERVE:
		vkvg_fill_preserve (ctx);
		break;
	case VKVG_CMD_STROKE_PRESERVE:
		vkvg_stroke_preserve (ctx);
		break;
	case VKVG_CMD_CLIP_PRESERVE:
		vkvg_clip_preserve (ctx);
		break;
	case VKVG_CMD_RECTANGLE:
		vkvg_rectangle (ctx, floats[0], floats[1], floats[2], floats[3]);
		break;
	case VKVG_CMD_ARC:
		vkvg_arc (ctx, floats[0], floats[1], floats[2], floats[3], floats[4]);
		break;
	case VKVG_CMD_ARC_NEG:
		vkvg_arc (ctx, floats[0], floats[1], floats[2], floats[3], floats[4]);
		break;
	/*case VKVG_CMD_ELLIPSE:
		vkvg_ellipse (ctx, floats[0], floats[1], floats[2], floats[3], floats[4]);
		break;*/
	case VKVG_CMD_MOVE_TO:
		vkvg_move_to(ctx, floats[0], floats[1]);
		break;
	case VKVG_CMD_LINE_TO:
		vkvg_line_to(ctx, floats[0], floats[1]);
		break;
	case VKVG_CMD_CURVE_TO:
		vkvg_curve_to (ctx, floats[0], floats[1], floats[2], floats[3], floats[4], floats[5]);
		break;
	case VKVG_CMD_ELLIPTICAL_ARC_TO:
		{
			bool* flags = (bool*)&floats[5];
			vkvg_elliptic_arc_to (ctx, floats[0], floats[1], flags[0], flags[1], floats[2], floats[3], floats[4]);
		}
		break;
	case VKVG_CMD_QUADRATIC_TO:
		vkvg_quadratic_to (ctx, floats[0], floats[1], floats[2], floats[3]);
		break;

	case VKVG_CMD_REL_MOVE_TO:
		vkvg_rel_move_to(ctx, floats[0], floats[1]);
		break;
	case VKVG_CMD_REL_LINE_TO:
		vkvg_rel_line_to(ctx, floats[0], floats[1]);
		break;
	case VKVG_CMD_REL_CURVE_TO:
		vkvg_rel_curve_to (ctx, floats[0], floats[1], floats[2], floats[3], floats[4], floats[5]);
		break;
	case VKVG_CMD_REL_QUADRATIC_TO:
		vkvg_rel_quadratic_to (ctx, floats[0], floats[1], floats[2], floats[3]);
		break;
	case VKVG_CMD_REL_ELLIPTICAL_ARC_TO:
		{
			bool* flags = (bool*)&floats[5];
			vkvg_rel_elliptic_arc_to (ctx, floats[0], floats[1], flags[0], flags[1], floats[2], floats[3], floats[4]);
		}
		break;
	case VKVG_CMD_SET_SOURCE_RGB:
		vkvg_set_source_rgb (ctx, floats[0], floats[1], floats[2]);
		break;
	case VKVG_CMD_SET_SOURCE_RGBA:
		vkvg_set_source_rgba (ctx, floats[0], floats[1], floats[2], floats[3]);
		break;
	case VKVG_CMD_SET_SOURCE:
		{
			VkvgPattern pat = (VkvgPattern)(rec->buffer + r->dataOffset);
			vkvg_set_source (ctx, pat);
		}
		break;
	case VKVG_CMD_SET_SOURCE_SURFACE:
		{
			VkvgSurface surf = (VkvgSurface)&floats[2];
			vkvg_set_source_surface (ctx, surf, floats[0], floats[1]);
		}
		break;
	}
}


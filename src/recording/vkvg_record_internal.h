﻿/*
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
#ifndef VKVG_RECORD_INTERNAL_H
#define VKVG_RECORD_INTERNAL_H

#include "vkvg.h"
#include "vkvg_internal.h"

#define VKVG_CMD_SAVE               0x0001
#define VKVG_CMD_RESTORE            0x0002

#define VKVG_CMD_PATH_COMMANDS      0x0100
#define VKVG_CMD_DRAW_COMMANDS      0x0200
#define VKVG_CMD_RELATIVE_COMMANDS  (0x0400 | VKVG_CMD_PATH_COMMANDS)
#define VKVG_CMD_PATHPROPS_COMMANDS (0x1000 | VKVG_CMD_PATH_COMMANDS)
#define VKVG_CMD_PRESERVE_COMMANDS  (0x0400 | VKVG_CMD_DRAW_COMMANDS)
#define VKVG_CMD_PATTERN_COMMANDS   0x0800
#define VKVG_CMD_TRANSFORM_COMMANDS 0x2000
#define VKVG_CMD_TEXT_COMMANDS      0x4000

#define VKVG_CMD_NEW_PATH           (0x0001 | VKVG_CMD_PATH_COMMANDS)
#define VKVG_CMD_NEW_SUB_PATH       (0x0002 | VKVG_CMD_PATH_COMMANDS)
#define VKVG_CMD_CLOSE_PATH         (0x0003 | VKVG_CMD_PATH_COMMANDS)
#define VKVG_CMD_MOVE_TO            (0x0004 | VKVG_CMD_PATH_COMMANDS)
#define VKVG_CMD_LINE_TO            (0x0005 | VKVG_CMD_PATH_COMMANDS)
#define VKVG_CMD_RECTANGLE          (0x0006 | VKVG_CMD_PATH_COMMANDS)
#define VKVG_CMD_ARC                (0x0007 | VKVG_CMD_PATH_COMMANDS)
#define VKVG_CMD_ARC_NEG            (0x0008 | VKVG_CMD_PATH_COMMANDS)
// #define VKVG_CMD_ELLIPSE			(0x0009|VKVG_CMD_PATH_COMMANDS)
#define VKVG_CMD_CURVE_TO              (0x000A | VKVG_CMD_PATH_COMMANDS)
#define VKVG_CMD_QUADRATIC_TO          (0x000B | VKVG_CMD_PATH_COMMANDS)
#define VKVG_CMD_ELLIPTICAL_ARC_TO     (0x000C | VKVG_CMD_PATH_COMMANDS)

#define VKVG_CMD_SET_LINE_WIDTH        (0x0001 | VKVG_CMD_PATHPROPS_COMMANDS)
#define VKVG_CMD_SET_MITER_LIMIT       (0x0002 | VKVG_CMD_PATHPROPS_COMMANDS)
#define VKVG_CMD_SET_LINE_JOIN         (0x0003 | VKVG_CMD_PATHPROPS_COMMANDS)
#define VKVG_CMD_SET_LINE_CAP          (0x0004 | VKVG_CMD_PATHPROPS_COMMANDS)
#define VKVG_CMD_SET_OPERATOR          (0x0005 | VKVG_CMD_PATHPROPS_COMMANDS)
#define VKVG_CMD_SET_FILL_RULE         (0x0006 | VKVG_CMD_PATHPROPS_COMMANDS)
#define VKVG_CMD_SET_DASH              (0x0007 | VKVG_CMD_PATHPROPS_COMMANDS)

#define VKVG_CMD_TRANSLATE             (0x0001 | VKVG_CMD_TRANSFORM_COMMANDS)
#define VKVG_CMD_ROTATE                (0x0002 | VKVG_CMD_TRANSFORM_COMMANDS)
#define VKVG_CMD_SCALE                 (0x0003 | VKVG_CMD_TRANSFORM_COMMANDS)
#define VKVG_CMD_TRANSFORM             (0x0004 | VKVG_CMD_TRANSFORM_COMMANDS)
#define VKVG_CMD_IDENTITY_MATRIX       (0x0005 | VKVG_CMD_TRANSFORM_COMMANDS)

#define VKVG_CMD_SET_MATRIX            (0x0006 | VKVG_CMD_TRANSFORM_COMMANDS)

#define VKVG_CMD_SET_FONT_SIZE         (0x0001 | VKVG_CMD_TEXT_COMMANDS)
#define VKVG_CMD_SET_FONT_FACE         (0x0002 | VKVG_CMD_TEXT_COMMANDS)
#define VKVG_CMD_SET_FONT_PATH         (0x0003 | VKVG_CMD_TEXT_COMMANDS)
#define VKVG_CMD_SHOW_TEXT             (0x0004 | VKVG_CMD_TEXT_COMMANDS)

#define VKVG_CMD_REL_MOVE_TO           (VKVG_CMD_MOVE_TO | VKVG_CMD_RELATIVE_COMMANDS)
#define VKVG_CMD_REL_LINE_TO           (VKVG_CMD_LINE_TO | VKVG_CMD_RELATIVE_COMMANDS)
#define VKVG_CMD_REL_CURVE_TO          (VKVG_CMD_CURVE_TO | VKVG_CMD_RELATIVE_COMMANDS)
#define VKVG_CMD_REL_QUADRATIC_TO      (VKVG_CMD_QUADRATIC_TO | VKVG_CMD_RELATIVE_COMMANDS)
#define VKVG_CMD_REL_ELLIPTICAL_ARC_TO (VKVG_CMD_ELLIPTICAL_ARC_TO | VKVG_CMD_RELATIVE_COMMANDS)

#define VKVG_CMD_PAINT                 (0x0001 | VKVG_CMD_DRAW_COMMANDS)
#define VKVG_CMD_FILL                  (0x0002 | VKVG_CMD_DRAW_COMMANDS)
#define VKVG_CMD_STROKE                (0x0003 | VKVG_CMD_DRAW_COMMANDS)
#define VKVG_CMD_CLIP                  (0x0004 | VKVG_CMD_DRAW_COMMANDS)
#define VKVG_CMD_RESET_CLIP            (0x0005 | VKVG_CMD_DRAW_COMMANDS)
#define VKVG_CMD_CLEAR                 (0x0006 | VKVG_CMD_DRAW_COMMANDS)

#define VKVG_CMD_FILL_PRESERVE         (VKVG_CMD_FILL | VKVG_CMD_PRESERVE_COMMANDS)
#define VKVG_CMD_STROKE_PRESERVE       (VKVG_CMD_STROKE | VKVG_CMD_PRESERVE_COMMANDS)
#define VKVG_CMD_CLIP_PRESERVE         (VKVG_CMD_CLIP | VKVG_CMD_PRESERVE_COMMANDS)

#define VKVG_CMD_SET_SOURCE_RGB        (0x0001 | VKVG_CMD_PATTERN_COMMANDS)
#define VKVG_CMD_SET_SOURCE_RGBA       (0x0002 | VKVG_CMD_PATTERN_COMMANDS)
#define VKVG_CMD_SET_SOURCE_COLOR      (0x0003 | VKVG_CMD_PATTERN_COMMANDS)
#define VKVG_CMD_SET_SOURCE            (0x0004 | VKVG_CMD_PATTERN_COMMANDS)
#define VKVG_CMD_SET_SOURCE_SURFACE    (0x0005 | VKVG_CMD_PATTERN_COMMANDS)

typedef struct _vkvg_record_t {
    uint16_t cmd;
    size_t   dataOffset;
} vkvg_record_t;

typedef struct _vkvg_recording_t {
    vkvg_record_t *commands;
    uint32_t       commandsCount;
    uint32_t       commandsReservedCount;
    size_t         bufferSize;
    size_t         bufferReservedSize;
    char          *buffer;
} vkvg_recording_t;

void              _start_recording(VkvgContext ctx);
vkvg_recording_t *_stop_recording(VkvgContext ctx);
void              _destroy_recording(vkvg_recording_t *rec);
void              _replay_command(VkvgContext ctx, VkvgRecording rec, uint32_t index);
void              _record(vkvg_recording_t *rec, ...);

#define RECORD(ctx, ...)                                                                                               \
    {                                                                                                                  \
        if (ctx->recording) {                                                                                          \
            _record(ctx->recording, __VA_ARGS__);                                                                      \
            return;                                                                                                    \
        }                                                                                                              \
    }
#define RECORD2(ctx, ...)                                                                                              \
    {                                                                                                                  \
        if (ctx->recording) {                                                                                          \
            _record(ctx->recording, __VA_ARGS__);                                                                      \
            return 0;                                                                                                  \
        }                                                                                                              \
    }

#endif

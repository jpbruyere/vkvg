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
#include "vkvg_context_internal.h"
#include "vkvg_record_internal.h"

#ifdef VKVG_RECORDING
void vkvg_start_recording (VkvgContext ctx) {
	if (ctx->status)
		return;
	_start_recording(ctx);
}
VkvgRecording vkvg_stop_recording (VkvgContext ctx) {
	if (ctx->status)
		return NULL;
	return _stop_recording (ctx);
}
uint32_t vkvg_recording_get_count (VkvgRecording rec) {
	if (!rec)
		return 0;
	return rec->commandsCount;
}
void vkvg_replay (VkvgContext ctx, VkvgRecording rec) {
	if (!rec)
		return;
	for (uint32_t i=0; i<rec->commandsCount; i++)
		_replay_command(ctx, rec, i);
}
void vkvg_recording_destroy (VkvgRecording rec) {
	if (!rec)
		return;
	_destroy_recording(rec);
}
#endif

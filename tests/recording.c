#include "test.h"

#if VKVG_RECORDING
void test(){
	VkvgContext ctx = _initCtx(surf);

	vkvg_scale(ctx,0.5,0.5);
	vkvg_start_recording(ctx);

	vkvg_set_source_rgba(ctx,0.1f,0.9f,0.1f,1.0f);
	vkvg_move_to(ctx,100,100);
	vkvg_rel_line_to(ctx,50,200);
	vkvg_rel_line_to(ctx,150,-100);
	vkvg_rel_line_to(ctx,100,200);
	vkvg_rel_line_to(ctx,-100,100);
	vkvg_rel_line_to(ctx,-10,-100);
	vkvg_rel_line_to(ctx,-190,-50);
	vkvg_rel_line_to(ctx,300,50);
	vkvg_rel_line_to(ctx,50,0);
	vkvg_rel_line_to(ctx,0, 50);
	vkvg_rel_line_to(ctx,50,0);
	vkvg_rel_line_to(ctx,0, 50);
	vkvg_rel_line_to(ctx,50,0);
	vkvg_rel_line_to(ctx,0, 50);
	vkvg_rel_line_to(ctx,50,0);
	vkvg_rel_line_to(ctx,0, 50);
	vkvg_close_path(ctx);

	vkvg_stroke_preserve(ctx);
	vkvg_fill(ctx);

	VkvgRecording rec = vkvg_stop_recording(ctx);

	vkvg_translate(ctx,400,0);
	vkvg_replay(ctx, rec);

	vkvg_destroy(ctx);
	vkvg_recording_destroy(rec);
}
#endif
int main(int argc, char *argv[]) {
	no_test_size = true;
#if VKVG_RECORDING
	PERFORM_TEST (test, argc, argv);
#endif
	return 0;
}


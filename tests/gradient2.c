#include "test.h"
#include "vectors.h"

vkvg_fill_rule_t fillrule = VKVG_FILL_RULE_NON_ZERO;
static VkSampleCountFlags samples = VK_SAMPLE_COUNT_8_BIT;
float lineWidth = 50.0f;
vkvg_line_join_t lineJoin = VKVG_LINE_JOIN_MITER;
vkvg_line_cap_t lineCap = VKVG_LINE_CAP_BUTT;
bool isClosed = false;

int ptsCount = 2;
int initPtsCount = 4;
vec2 pts[] = {
	{150,150},
	{250,150},
	{125,125},
	{145,125},
};
int hoverPt = -1;
double pointSize = 7;

static vkvg_pattern_type_t patternType = VKVG_PATTERN_TYPE_LINEAR;
static VkEngine e;

void draw (){

	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);

	VkvgPattern pat;

	switch (patternType) {
	case VKVG_PATTERN_TYPE_LINEAR:
		pat = vkvg_pattern_create_linear(pts[0].x,pts[0].y, pts[1].x,pts[1].y);
		break;
	case VKVG_PATTERN_TYPE_RADIAL:
		pat = vkvg_pattern_create_radial(
					pts[2].x,pts[2].y, vec2_length(vec2_sub(pts[3], pts[2])),
					pts[0].x,pts[0].y, vec2_length(vec2_sub(pts[1], pts[0]))
		);
		break;
	}

	/**/

	vkvg_pattern_add_color_stop(pat, 0.0, 1 ,0 ,0, 0.5);
	vkvg_pattern_add_color_stop(pat, 0.3, 0 ,1 ,0, 0.5);
	vkvg_pattern_add_color_stop(pat, 0.6, 0 ,0 ,1, 0.5);
	vkvg_pattern_add_color_stop(pat, 1.0, 0 ,0 ,0, 0.5);

	vkvg_set_source (ctx, pat);
	vkvg_paint (ctx);

	vkvg_set_dash(ctx, NULL, 0, 0);
	vkvg_set_line_width(ctx,1);
	for (int i=0; i<ptsCount; i++) {
		if (hoverPt == i)
			vkvg_set_source_rgba(ctx,0.5f,0.5f,1.0f,0.7f);
		else
			vkvg_set_source_rgba(ctx,1.0f,0.5f,0.5f,0.9f);
		vkvg_arc(ctx, pts[i].x, pts[i].y, pointSize, 0, M_PIF*2);
		vkvg_fill_preserve(ctx);
		vkvg_stroke(ctx);
	}


	vkvg_pattern_destroy	(pat);
	vkvg_destroy			(ctx);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS)
		return;
	switch (key) {
	case GLFW_KEY_ESCAPE :
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	case GLFW_KEY_P :
		patternType++;
		if (patternType > VKVG_PATTERN_TYPE_RADIAL)
			patternType = VKVG_PATTERN_TYPE_LINEAR;
		break;
	case GLFW_KEY_S :
		vkengine_wait_idle(e);
		vkvg_surface_write_to_png(surf, "/home/jp/test.png");
		break;
	case GLFW_KEY_KP_ADD :
		if (ptsCount < initPtsCount)
			ptsCount++;
		break;
	case GLFW_KEY_KP_SUBTRACT :
		if (ptsCount > 1)
			ptsCount--;
		break;
	}
}
static void mouse_move_callback(GLFWwindow* window, double x, double y){
	if (mouseDown) {
		if (hoverPt < 0)
			return;
		pts[hoverPt].x = x;
		pts[hoverPt].y = y;
	} else {
		for (int i=0; i<ptsCount; i++) {
			if (x > pts[i].x - pointSize &&
				x < pts[i].x + pointSize &&
				y > pts[i].y - pointSize &&
				y < pts[i].y + pointSize) {
				hoverPt = i;
				return;
			}
		}
		hoverPt = -1;
	}
}
static void scroll_callback(GLFWwindow* window, double x, double y){
	if (y<0.f)
		zoom *= 0.5f;
	else
		zoom *= 2.0f;
}
static void mouse_button_callback(GLFWwindow* window, int but, int state, int modif){
	if (but != GLFW_MOUSE_BUTTON_1)
		return;
	if (state == GLFW_TRUE)
		mouseDown = true;
	else
		mouseDown = false;
}



int main(int argc, char* argv[]) {

	_parse_args (argc, argv);

	e = vkengine_create (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_FIFO_KHR, test_width, test_height);

	VkhPresenter r = e->renderer;
	vkengine_set_key_callback (e, key_callback);
	vkengine_set_mouse_but_callback(e, mouse_button_callback);
	vkengine_set_cursor_pos_callback(e, mouse_move_callback);
	vkengine_set_scroll_callback(e, scroll_callback);

	bool deferredResolve = false;

	device = vkvg_device_create_from_vk_multisample(vkh_app_get_inst(e->app), r->dev->phy, r->dev->dev, r->qFam, 0, samples, deferredResolve);
	surf = vkvg_surface_create(device, test_width, test_height);

	vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), test_width, test_height);

	while (!vkengine_should_close (e)) {
		glfwPollEvents();

		draw ();

		if (!vkh_presenter_draw (r)){
			vkh_presenter_get_size (r, &test_width, &test_height);
			vkvg_surface_destroy (surf);
			surf = vkvg_surface_create(device, test_width, test_height);
			vkh_presenter_build_blit_cmd (r, vkvg_surface_get_vk_image(surf), test_width, test_height);
			vkDeviceWaitIdle(r->dev->dev);
			continue;
		}
	}
	vkDeviceWaitIdle(e->dev->dev);

	vkvg_surface_destroy    (surf);

	vkvg_device_destroy     (device);

	vkengine_destroy (e);

	return 0;
}

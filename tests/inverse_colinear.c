#include "test.h"
#include "vectors.h"

vkvg_fill_rule_t fillrule = VKVG_FILL_RULE_NON_ZERO;
static VkSampleCountFlags samples = VK_SAMPLE_COUNT_8_BIT;
float lineWidth = 50.0f;
vkvg_line_join_t lineJoin = VKVG_LINE_JOIN_MITER;
vkvg_line_cap_t lineCap = VKVG_LINE_CAP_BUTT;
bool isClosed = false;
bool startWithArc = false, endWithArc = false;

int ptsCount = 4;
int initPtsCount = 5;
vec2 pts[] = {
	{150,150},
	{200,300},
	{250,150},
	{280,350},
	{300,100},
};
/*vec2 pts[] = {
	{150,150},
	{250,150},
	{100,150},
	{150,200},
};*/
int hoverPt = -1;
double pointSize = 7;

float dash[] = {0, 60};
uint32_t dashCountInit = 2;
uint32_t dashCount = 0;
float miterLimit = 10.0f;



void draw (){

	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);
	if (dashCount > 0)
		vkvg_set_dash(ctx, dash, dashCount,0);
	vkvg_set_source_rgba	(ctx,1,0,0,0.3f);
	vkvg_set_line_width		(ctx,lineWidth);
	vkvg_set_line_join		(ctx, lineJoin);
	vkvg_set_line_cap		(ctx, lineCap);
	vkvg_set_miter_limit	(ctx, miterLimit);

	if (startWithArc)
		vkvg_arc_negative(ctx,pts[0].x,pts[0].y,200, M_PIF*1.5f, M_PIF);
	else
		vkvg_move_to(ctx,pts[0].x,pts[0].y);
	for (int i=1; i<ptsCount-1; i++)
		vkvg_line_to(ctx,pts[i].x,pts[i].y);
	if (endWithArc)
		vkvg_arc_negative(ctx,pts[ptsCount-1].x,pts[ptsCount-1].y,200, M_PIF*1.5f, M_PIF);
	else
		vkvg_line_to(ctx,pts[ptsCount-1].x,pts[ptsCount-1].y);

	if (isClosed)
		vkvg_close_path(ctx);

	if (hoverPt>=0) {
		vkvg_stroke_preserve(ctx);
		vkvg_set_dash(ctx, NULL, 0, 0);
		vkvg_set_line_width(ctx,2);
		vkvg_set_source_rgba(ctx,0,0,0,1);
		vkvg_stroke(ctx);
		vkvg_set_source_rgba(ctx,0.5f,0.5f,1.0f,0.7f);
		vkvg_arc (ctx, pts[hoverPt].x, pts[hoverPt].y, pointSize, 0, M_PIF*2);
		vkvg_fill_preserve(ctx);
		vkvg_stroke(ctx);
	} else
		vkvg_stroke(ctx);

	//draw_v(ctx, 200, 20, VKVG_LINE_JOIN_BEVEL);
	//draw_v(ctx, 300, 80, VKVG_LINE_JOIN_ROUND);
	vkvg_destroy(ctx);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_RELEASE)
		return;
	switch (key) {
	case GLFW_KEY_ESCAPE :
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	case GLFW_KEY_W :
		isClosed ^= true;
		break;
	case GLFW_KEY_Q :
		startWithArc ^= true;
		break;
	case GLFW_KEY_A :
		endWithArc ^= true;
		break;
	case GLFW_KEY_J :
		lineJoin++;
		if (lineJoin > 2)
			lineJoin = 0;
		break;
	case GLFW_KEY_C :
		lineCap++;
		if (lineCap > 2)
			lineCap = 0;
		break;
	case GLFW_KEY_D :
		if (dashCount == 0)
			dashCount = dashCountInit;
		else
			dashCount = 0;
		break;
	case GLFW_KEY_E :
		if (dash[0] > 0)
			dash[0] = 0;
		else
			dash[0] = 80;
		break;
	case GLFW_KEY_L :
		if (mods & GLFW_MOD_SHIFT)
			miterLimit /= 2.0f;
		else
			miterLimit *= 2.0f;
		break;
	case GLFW_KEY_P :
		if (mods & GLFW_MOD_SHIFT) {
			if (ptsCount > 1)
				ptsCount--;
		} else {
			if (ptsCount < initPtsCount)
				ptsCount++;
		}
		break;
	case GLFW_KEY_KP_ADD :
		lineWidth++;
		break;
	case GLFW_KEY_KP_SUBTRACT :
		if (lineWidth > 1)
			lineWidth--;
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
	VkEngine e;
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

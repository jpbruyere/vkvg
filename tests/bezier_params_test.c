#include "test.h"
#include "vectors.h"

vkvg_fill_rule_t fillrule = VKVG_FILL_RULE_NON_ZERO;
static VkSampleCountFlags samples = VK_SAMPLE_COUNT_8_BIT;
float lineWidth = 10.0f;
vkvg_line_join_t lineJoin = VKVG_LINE_JOIN_MITER;
vkvg_line_cap_t lineCap = VKVG_LINE_CAP_BUTT;
bool isClosed = false;

int ptsCount = 4;
int initPtsCount = 4;
vec2 pts[] = {
	{150,150},
	{500,340},
	{470,150},
	{160,350},
};
/*vec2 pts[] = {
	{150,150},
	{250,150},
	{100,150},
	{150,200},
};*/
int hoverPt = -1;
double	hoverPointSize = 7,
		pointSize = 5;

static float M_APPROXIMATION_SCALE	= 1.0f;
static float M_ANGLE_TOLERANCE		= 0.01f;
static float M_CUSP_LIMIT			= 0.01f;
static unsigned CURVE_RECURSION_LIMIT	= 100;
static float CURVE_COLLINEARITY_EPSILON =1.7f;
static float CURVE_ANGLE_TOLERANCE_EPSILON =0.001f;
static float CURVE_DISTANCE_TOLERANCE = 1.0f;

#ifdef VKVG_WIRED_DEBUG
vkvg_wired_debug_mode _wired_debug = vkvg_wired_debug_mode_normal;
#endif

void draw (){

	VkvgContext ctx = vkvg_create(surf);
	vkvg_clear(ctx);

	vkvg_set_source_rgba(ctx,1,1,1,1);
	float x = 10, y = 20, lineHeight = 15;

	char buff[256];

	vkvg_move_to(ctx, x, y);
	sprintf (buff, "1. approximation scale:     %6.1f", M_APPROXIMATION_SCALE);
	vkvg_show_text(ctx, buff);
	y+=lineHeight;
	vkvg_move_to(ctx, x, y);
	sprintf (buff, "2. angle tolerance:         %6.3f", M_ANGLE_TOLERANCE);
	vkvg_show_text(ctx, buff);
	y+=lineHeight;
	vkvg_move_to(ctx, x, y);
	sprintf (buff, "3. cusp limit:              %6.3f", M_CUSP_LIMIT);
	vkvg_show_text(ctx, buff);
	y+=lineHeight;
	vkvg_move_to(ctx, x, y);
	sprintf (buff, "4. recursion limit:         %6d", CURVE_RECURSION_LIMIT);
	vkvg_show_text(ctx, buff);
	y+=lineHeight;
	vkvg_move_to(ctx, x, y);
	sprintf (buff, "5. collinearity epsilon:    %6.3f", CURVE_COLLINEARITY_EPSILON);
	vkvg_show_text(ctx, buff);
	y+=lineHeight;
	vkvg_move_to(ctx, x, y);
	sprintf (buff, "6. angle tolerance epsilon  %6.4f", CURVE_ANGLE_TOLERANCE_EPSILON);
	vkvg_show_text(ctx, buff);
	y+=lineHeight;
	vkvg_move_to(ctx, x, y);
	sprintf (buff, "7. distance tolerance:      %6.3f", CURVE_DISTANCE_TOLERANCE);
	vkvg_show_text(ctx, buff);
	vkvg_flush (ctx);

#ifdef VKVG_WIRED_DEBUG
	vkvg_wired_debug = _wired_debug;
#endif

	vkvg_set_source_rgba(ctx,1,0,0,1);
	vkvg_set_line_width (ctx,lineWidth);
	vkvg_set_line_join	(ctx, lineJoin);
	vkvg_set_line_cap	(ctx, lineCap);

	vkvg_move_to(ctx,pts[0].x,pts[0].y);
	vkvg_curve_to(ctx, pts[1].x,pts[1].y, pts[2].x,pts[2].y, pts[3].x,pts[3].y);

	if (isClosed)
		vkvg_close_path(ctx);

	vkvg_stroke (ctx);
	vkvg_flush (ctx);

#ifdef VKVG_WIRED_DEBUG
	vkvg_wired_debug = vkvg_wired_debug_mode_normal;
#endif

	vkvg_set_line_width(ctx, 1);
	vkvg_set_source_rgba(ctx,0.5,0.5,0.5,0.6);
	for (int i=0; i<ptsCount; i++) {

		if (i == hoverPt) {
			vkvg_arc (ctx, pts[i].x, pts[i].y, hoverPointSize, 0, M_PIF*2);
			vkvg_set_source_rgba (ctx, 0.5, 0.5, 1.0, 0.6);
			vkvg_fill_preserve (ctx);
			vkvg_set_source_rgba (ctx, 0.5 ,0.5 ,0.5 ,0.6);
		} else {
			vkvg_arc (ctx, pts[i].x, pts[i].y, pointSize, 0, M_PIF*2);
			vkvg_fill_preserve (ctx);
		}
		vkvg_stroke (ctx);


	}

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
	case GLFW_KEY_1:
		if (mods & GLFW_MOD_SHIFT)
			M_APPROXIMATION_SCALE -= 0.1f;
		else
			M_APPROXIMATION_SCALE += 0.1f;
		break;
	case GLFW_KEY_2:
		if (mods & GLFW_MOD_SHIFT)
			M_ANGLE_TOLERANCE -= 0.01f;
		else
			M_ANGLE_TOLERANCE += 0.01f;
		break;
	case GLFW_KEY_3:
		if (mods & GLFW_MOD_SHIFT)
			M_CUSP_LIMIT -= 0.01f;
		else
			M_CUSP_LIMIT += 0.01f;
		break;
	case GLFW_KEY_4:
		if (mods & GLFW_MOD_SHIFT)
			CURVE_RECURSION_LIMIT -= 1;
		else
			CURVE_RECURSION_LIMIT += 1;
		break;
	case GLFW_KEY_5:
		if (mods & GLFW_MOD_SHIFT)
			CURVE_COLLINEARITY_EPSILON -= 0.01f;
		else
			CURVE_COLLINEARITY_EPSILON += 0.01f;
		break;
	case GLFW_KEY_6:
		if (mods & GLFW_MOD_SHIFT)
			CURVE_ANGLE_TOLERANCE_EPSILON -= 0.001f;
		else
			CURVE_ANGLE_TOLERANCE_EPSILON += 0.001f;
		break;
	case GLFW_KEY_7:
		if (mods & GLFW_MOD_SHIFT) {
			if (mods & GLFW_MOD_CONTROL) {
				if (CURVE_DISTANCE_TOLERANCE > 0.002f)
					CURVE_DISTANCE_TOLERANCE -= 0.001f;
			} else {
				if (CURVE_DISTANCE_TOLERANCE > 0.02f)
					CURVE_DISTANCE_TOLERANCE -= 0.01f;
			}
		} else {
			if (mods & GLFW_MOD_CONTROL)
				CURVE_DISTANCE_TOLERANCE += 0.001f;
			else
				CURVE_DISTANCE_TOLERANCE += 0.01f;
		}
		break;
	case GLFW_KEY_ESCAPE :
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	case GLFW_KEY_W :
		isClosed ^= true;
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
	case GLFW_KEY_KP_ADD :
		if (ptsCount < initPtsCount)
			ptsCount++;
		break;
	case GLFW_KEY_KP_SUBTRACT :
		if (ptsCount > 1)
			ptsCount--;
		break;
#ifdef VKVG_WIRED_DEBUG
	case GLFW_KEY_F1:
		_wired_debug ^= (1U << 0);
		break;
	case GLFW_KEY_F2:
		_wired_debug ^= (1U << 1);
		break;
	case GLFW_KEY_F3:
		_wired_debug ^= (1U << 2);
		break;
#endif
	}

#ifdef VKVG_DEBUG_CURVE_PARAMS
	vkvg_set_bezier_params(M_APPROXIMATION_SCALE, M_ANGLE_TOLERANCE, M_CUSP_LIMIT, CURVE_RECURSION_LIMIT,
						   CURVE_COLLINEARITY_EPSILON, CURVE_ANGLE_TOLERANCE_EPSILON, CURVE_DISTANCE_TOLERANCE);
#endif
}
static void mouse_move_callback(GLFWwindow* window, double x, double y){
	if (mouseDown) {
		if (hoverPt < 0)
			return;
		pts[hoverPt].x = x;
		pts[hoverPt].y = y;
	} else {
		for (int i=0; i<ptsCount; i++) {
			if (x > pts[i].x - hoverPointSize &&
				x < pts[i].x + hoverPointSize &&
				y > pts[i].y - hoverPointSize &&
				y < pts[i].y + hoverPointSize) {
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

#ifdef VKVG_DEBUG_CURVE_PARAMS
	vkvg_get_bezier_params(&M_APPROXIMATION_SCALE, &M_ANGLE_TOLERANCE, &M_CUSP_LIMIT, &CURVE_RECURSION_LIMIT,
						   &CURVE_COLLINEARITY_EPSILON, &CURVE_ANGLE_TOLERANCE_EPSILON, &CURVE_DISTANCE_TOLERANCE);
#endif

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

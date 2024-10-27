#include "test.h"
#include "vectors.h"

vkvg_fill_rule_t          fillrule  = VKVG_FILL_RULE_NON_ZERO;
static VkSampleCountFlags samples   = VK_SAMPLE_COUNT_8_BIT;
float                     lineWidth = 4.0f;
vkvg_line_join_t          lineJoin  = VKVG_LINE_JOIN_MITER;
vkvg_line_cap_t           lineCap   = VKVG_LINE_CAP_BUTT;
bool                      isClosed = false, refresh = true;
bool                      startWithArc = false, endWithArc = false;

int    ptsCount = 0;
vec2   pts[1024];
int    hoverPt   = -1;
double pointSize = 7;
float  _dashes[] = {5.0f, 3.0f};
int    _ndashes  = 2;

void emit_path(VkvgContext ctx) {
    vkvg_move_to(ctx, pts[0].x, pts[0].y);
    for (int i = 1; i < ptsCount; i++)
        vkvg_line_to(ctx, pts[i].x, pts[i].y);
    if (isClosed)
        vkvg_close_path(ctx);
}

void draw() {
    refresh = false;

    VkvgContext ctx = vkvg_create(surf);
    vkvg_clear(ctx);
    vkvg_set_source_rgba(ctx, 1, 0, 0, 1);
    vkvg_set_line_width(ctx, lineWidth);
    vkvg_set_line_join(ctx, lineJoin);
    vkvg_set_line_cap(ctx, lineCap);

    emit_path(ctx);
    float x1, y1, x2, y2;
    vkvg_path_extents(ctx, &x1, &y1, &x2, &y2);
    vkvg_set_source_rgba(ctx, 0.2, 0.2, 1, 0.5);
    if (hoverPt >= 0) {
        if (isClosed)
            vkvg_fill_preserve(ctx);
        vkvg_set_source_rgba(ctx, 1, 0.2, 0.2, 0.5);
        vkvg_stroke_preserve(ctx);
        vkvg_set_dash(ctx, NULL, 0, 0);
        vkvg_set_line_width(ctx, 1);
        vkvg_set_source_rgba(ctx, 0, 0, 0, 1);
        vkvg_stroke(ctx);
        vkvg_set_source_rgba(ctx, 0.5f, 0.5f, 1.0f, 0.7f);
        vkvg_arc(ctx, pts[hoverPt].x, pts[hoverPt].y, pointSize, 0, M_PIF * 2);
        vkvg_fill_preserve(ctx);
        vkvg_stroke(ctx);
    } else {
        if (isClosed)
            vkvg_fill_preserve(ctx);
        vkvg_set_source_rgba(ctx, 1, 0.2, 0.2, 0.5);
        vkvg_stroke(ctx);
    }

    vkvg_rectangle(ctx, x1, y1, x2 - x1, y2 - y1);
    vkvg_set_dash(ctx, _dashes, _ndashes, 0);
    vkvg_set_line_width(ctx, 1);
    vkvg_set_source_rgba(ctx, 0, 1, 0, 1);
    vkvg_stroke(ctx);

    vkvg_destroy(ctx);
}
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;
    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case GLFW_KEY_W:
        isClosed ^= true;
        break;
    case GLFW_KEY_DELETE:
        ptsCount = 0;
        break;
    case GLFW_KEY_J:
        lineJoin++;
        if (lineJoin > 2)
            lineJoin = 0;
        break;
    case GLFW_KEY_C:
        lineCap++;
        if (lineCap > 2)
            lineCap = 0;
        break;
    case GLFW_KEY_KP_SUBTRACT:
        if (ptsCount > 1)
            ptsCount--;
        break;
    }
    refresh = true;
}
static vec2 mousePos;
static void mouse_move_callback(GLFWwindow *window, double x, double y) {
    mousePos = (vec2){x, y};
    if (mouseDown) {
        if (hoverPt < 0)
            return;
        pts[hoverPt] = mousePos;
        refresh      = true;
    } else {
        for (int i = 0; i < ptsCount; i++) {
            if (x > pts[i].x - pointSize && x < pts[i].x + pointSize && y > pts[i].y - pointSize &&
                y < pts[i].y + pointSize) {
                hoverPt = i;
                refresh = true;
                return;
            }
        }
        refresh = true;
        hoverPt = -1;
    }
}
static void scroll_callback(GLFWwindow *window, double x, double y) {
    if (y < 0.f)
        zoom *= 0.5f;
    else
        zoom *= 2.0f;
}
static void mouse_button_callback(GLFWwindow *window, int but, int state, int modif) {
    if (but != GLFW_MOUSE_BUTTON_1)
        return;
    if (state == GLFW_TRUE)
        mouseDown = true;
    else
        mouseDown = false;
    if (state == GLFW_TRUE && hoverPt < 0)
        pts[ptsCount++] = mousePos;
    refresh = true;
}

int main(int argc, char *argv[]) {

    _parse_args(argc, argv);
    VkEngine e;
    e = vkengine_create(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PRESENT_MODE_FIFO_KHR, test_width, test_height);

    VkhPresenter r = e->renderer;
    vkengine_set_key_callback(e, key_callback);
    vkengine_set_mouse_but_callback(e, mouse_button_callback);
    vkengine_set_cursor_pos_callback(e, mouse_move_callback);
    vkengine_set_scroll_callback(e, scroll_callback);

    vkvg_device_create_info_t info = {samples, false, vkh_app_get_inst(e->app), r->dev->phy, r->dev->dev, r->qFam, 0};
    device                         = vkvg_device_create(&info);
    surf                           = vkvg_surface_create(device, test_width, test_height);

    vkh_presenter_build_blit_cmd(r, vkvg_surface_get_vk_image(surf), test_width, test_height);

    while (!vkengine_should_close(e)) {
        glfwPollEvents();

        if (refresh)
            draw();

        if (!vkh_presenter_draw(r)) {
            vkh_presenter_get_size(r, &test_width, &test_height);
            vkvg_surface_destroy(surf);
            surf = vkvg_surface_create(device, test_width, test_height);
            vkh_presenter_build_blit_cmd(r, vkvg_surface_get_vk_image(surf), test_width, test_height);
            vkDeviceWaitIdle(r->dev->dev);
            continue;
        }
    }
    vkDeviceWaitIdle(e->dev->dev);

    vkvg_surface_destroy(surf);

    vkvg_device_destroy(device);

    vkengine_destroy(e);

    return 0;
}

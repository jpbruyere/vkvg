#include "test.h"
#include "vectors.h"
#include "vkvg_context_internal.h"

vkvg_fill_rule_t          fillrule     = VKVG_FILL_RULE_NON_ZERO;
static VkSampleCountFlags samples      = VK_SAMPLE_COUNT_8_BIT;
float                     lineWidth    = 3.0f;
vkvg_line_join_t          lineJoin     = VKVG_LINE_JOIN_MITER;
vkvg_line_cap_t           lineCap      = VKVG_LINE_CAP_BUTT;
bool                      isClosed     = false;
bool                      startWithArc = false, endWithArc = false;
float                     angle = 0;
float                     r;
vec2                      mouse;

void draw() {
    angle += 0.001f;

    VkvgContext ctx = vkvg_create(surf);
    vkvg_clear(ctx);

    float r = fabsf(test_width / 2 - mouse.x) / zoom;

    vkvg_new_path(ctx);
    vkvg_set_source_rgba(ctx, 1, 0, 0, 1);
    vkvg_set_line_width(ctx, lineWidth / zoom);

    vkvg_translate(ctx, test_width / 2, test_height / 2);
    vkvg_scale(ctx, zoom, zoom);
    // vkvg_rotate (ctx, angle);

    vkvg_matrix_t mat;
    vkvg_get_matrix(ctx, &mat);
    float sx, sy;
    vkvg_matrix_get_scale(&mat, &sx, &sy);

    vkvg_arc(ctx, 0, 0, r, 0, M_PIF * 2);
    vkvg_fill(ctx);

    float steps = _get_arc_step(ctx, r);
    vkvg_identity_matrix(ctx);

    vkvg_set_source_rgba(ctx, 1, 1, 1, 1);
    char txt[100];
    sprintf(txt, "scale: %f, %f", sx, sy);
    vkvg_move_to(ctx, 10, 10);
    vkvg_show_text(ctx, txt);
    sprintf(txt, "angle: %f", angle);
    vkvg_move_to(ctx, 10, 25);
    vkvg_show_text(ctx, txt);
    sprintf(txt, "radius: %f", r);
    vkvg_move_to(ctx, 10, 40);
    vkvg_show_text(ctx, txt);
    sprintf(txt, "zoom: %f", zoom);
    vkvg_move_to(ctx, 10, 55);
    vkvg_show_text(ctx, txt);
    sprintf(txt, "arc step: %f", steps);
    vkvg_move_to(ctx, 10, 70);
    vkvg_show_text(ctx, txt);
    sprintf(txt, "steps: %d", (int)roundf(2.0f * M_PI / steps));
    vkvg_move_to(ctx, 10, 85);
    vkvg_show_text(ctx, txt);

    vkvg_destroy(ctx);
}
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;
    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case GLFW_KEY_KP_ADD:
        zoom *= 2.0f;
        break;
    case GLFW_KEY_KP_SUBTRACT:
        zoom *= 0.5f;
        break;
#ifdef VKVG_WIRED_DEBUG
    case GLFW_KEY_F1:
        vkvg_wired_debug ^= (1U << 0);
        break;
    case GLFW_KEY_F2:
        vkvg_wired_debug ^= (1U << 1);
        break;
    case GLFW_KEY_F3:
        vkvg_wired_debug ^= (1U << 2);
        break;
#endif
    }
}
static void mouse_move_callback(GLFWwindow *window, double x, double y) {
    if (mouseDown) {
        mouse = (vec2){x, y};
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

    bool deferredResolve = false;

    device = vkvg_device_create_from_vk_multisample(vkh_app_get_inst(e->app), r->dev->phy, r->dev->dev, r->qFam, 0,
                                                    samples, deferredResolve);
    surf   = vkvg_surface_create(device, test_width, test_height);

    vkh_presenter_build_blit_cmd(r, vkvg_surface_get_vk_image(surf), test_width, test_height);

    mouse = (vec2){test_width * 0.75, test_height * 0.5f};

    while (!vkengine_should_close(e)) {
        glfwPollEvents();

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

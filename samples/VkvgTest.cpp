#include "VkvgTest.hpp"


std::vector<VkvgTest*> VkvgTest::tests;

VkvgTest::VkvgTest(VkvgTestFunc testFunc, std::string _name) {
    this->testFunc = testFunc;
    this->name = _name;
    VkvgTest::tests.push_back(this);
}
void VkvgTest::initTest(SampleApp* app) {
    this->app = app;

    vkvg_device_create_info_t info = {
        app->samples, false,
        vkh_app_get_inst(app->app), vkh_device_get_phy(app->vkhDev), vkh_device_get_vkdev(app->vkhDev), app->presentQIndex, 0, false};

    device = vkvg_device_create(&info);
    surf = vkvg_surface_create(device, app->width, app->height);
    vkvg_device_set_dpy(device, 96, 96);

    vkh_presenter_build_blit_cmd(app->renderer, vkvg_surface_get_vk_image(surf), app->width, app->height);

    vkDeviceWaitIdle(vkh_device_get_vkdev(app->vkhDev));
}
void VkvgTest::performTest() {
    testFunc(this);
}
void VkvgTest::cleanTest() {
    vkDeviceWaitIdle(vkh_device_get_vkdev(app->vkhDev));
    vkvg_surface_destroy(surf);
    vkvg_device_destroy(device);
    vkDeviceWaitIdle(vkh_device_get_vkdev(app->vkhDev));
}

void draw_growing_circles(VkvgContext ctx, float y, int count) {
    float x = 2;
    for (int i = 1; i < count; i++) {
        x += 0.5f * i;
        vkvg_arc(ctx, x + 2, y, 0.5f * i, 0, M_PIF * 1.5f);
        x += 0.5f * i + 5;
    }
}
/* common context init for several tests */
vkvg_fill_rule_t fill_rule = VKVG_FILL_RULE_NON_ZERO;
vkvg_line_cap_t  line_cap  = VKVG_LINE_CAP_BUTT;
vkvg_line_join_t line_join = VKVG_LINE_JOIN_MITER;
float            dashes[]  = {10.0f, 6.0f};
// float		dashes[]	= {0.0f, 10.0f};
uint32_t dashes_count = 0;
float    dash_offset  = 0;
float    line_width   = 2.f;

/*VkvgContext _initCtx() {
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_line_width(ctx, line_width);
    vkvg_set_line_join(ctx, line_join);
    vkvg_set_line_cap(ctx, line_cap);
    vkvg_set_dash(ctx, dashes, dashes_count, dash_offset);
    vkvg_set_fill_rule(ctx, fill_rule);
    return ctx;
}*/

const int star_points[11][2] = {{0, 85},    {75, 75},   {100, 10}, {125, 75}, {200, 85}, {150, 125},
                                {160, 190}, {100, 150}, {40, 190}, {50, 125}, {0, 85}};
void randomize_color(VkvgContext ctx) { vkvg_set_source_rgba(ctx, rndf(), rndf(), rndf(), rndf()); }

void VkvgTest::draw_random_shape(VkvgContext ctx, shape_t shape, float sizeFact) {
    float w = (float)app->width;
    float h = (float)app->height;


    float x, y, z, v, r;

    randomize_color(ctx);

    switch (shape) {
    case SHAPE_LINE:
        x = rndf() * w;
        y = rndf() * h;
        z = rndf() * w;
        v = rndf() * h;

        vkvg_move_to(ctx, x, y);
        vkvg_line_to(ctx, z, v);
        vkvg_stroke(ctx);
        break;
    case SHAPE_RECTANGLE:
        z = truncf((sizeFact * w * rndf()) + 1.f);
        v = truncf((sizeFact * h * rndf()) + 1.f);
        x = truncf((w - z) * rndf());
        y = truncf((h - v) * rndf());

        vkvg_rectangle(ctx, x + 1, y + 1, z, v);
        break;
    case SHAPE_ROUNDED_RECTANGLE:
        z = truncf((sizeFact * w * rndf()) + 1.f);
        v = truncf((sizeFact * h * rndf()) + 1.f);
        x = truncf((w - z) * rndf());
        y = truncf((h - v) * rndf());
        r = truncf((0.2f * z * rndf()) + 1.f);

        if ((r > v / 2) || (r > z / 2))
            r = MIN(v / 2, z / 2);

        vkvg_move_to(ctx, x, y + r);
        vkvg_arc(ctx, x + r, y + r, r, (float)M_PI, (float)-M_PI_2);
        vkvg_line_to(ctx, x + z - r, y);
        vkvg_arc(ctx, x + z - r, y + r, r, (float)-M_PI_2, 0);
        vkvg_line_to(ctx, x + z, y + v - r);
        vkvg_arc(ctx, x + z - r, y + v - r, r, 0, (float)M_PI_2);
        vkvg_line_to(ctx, x + r, y + v);
        vkvg_arc(ctx, x + r, y + v - r, r, (float)M_PI_2, (float)M_PI);
        vkvg_line_to(ctx, x, y + r);
        vkvg_close_path(ctx);
        break;
    case SHAPE_CIRCLE:
        /*x = truncf((float)w * rnd()/RAND_MAX);
        y = truncf((float)h * rnd()/RAND_MAX);
        v = truncf((float)w * rnd()/RAND_MAX * 0.2f);*/
        x = rndf() * w;
        y = rndf() * h;

        r = truncf((sizeFact * MIN(w, h) * rndf()) + 1.f);

        /*float r = 0.5f*w*rand()/RAND_MAX;
        float x = truncf(0.5f * w*rand()/RAND_MAX + r);
        float y = truncf(0.5f * w*rand()/RAND_MAX + r);*/

        vkvg_arc(ctx, x, y, r, 0, (float)M_PI * 2.0f);
        break;
    case SHAPE_TRIANGLE:
    case SHAPE_STAR:
        x = rndf() * w;
        y = rndf() * h;
        z = rndf() * sizeFact + 0.15f; // scale

        vkvg_move_to(ctx, x + star_points[0][0] * z, y + star_points[0][1] * z);
        for (int s = 1; s < 11; s++)
            vkvg_line_to(ctx, x + star_points[s][0] * z, y + star_points[s][1] * z);
        vkvg_close_path(ctx);
        break;
    case SHAPE_RANDOM:
        draw_random_shape(ctx, (shape_t)(1 + (rndf() * 4)), sizeFact);
        break;
    }
}
void VkvgTest::draw_random_curve(VkvgContext ctx) {
    float w = (float)app->width;
    float h = (float)app->height;

    float x2    = w * rndf();
    float y2    = h * rndf();
    float cp_x1 = w * rndf();
    float cp_y1 = h * rndf();
    float cp_x2 = w * rndf();
    float cp_y2 = h * rndf();

    vkvg_curve_to(ctx, cp_x1, cp_y1, cp_x2, cp_y2, x2, y2);
}
void VkvgTest::draw_random_square(VkvgContext ctx, float s) {
    float w = (float)app->width;
    float h = (float)app->height;
    randomize_color(ctx);

    float x = truncf(w * rndf());
    float y = truncf(h * rndf());

    vkvg_rectangle(ctx, x, y, s, s);
}


#include "VkvgTest.hpp"

TEST(img_paint) {
    VkvgContext ctx = vkvg_create(test->surf);    
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("mirror.jpg"));

    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(img_paint_offset) {
    VkvgContext ctx = vkvg_create(test->surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("mirror.jpg"));
    vkvg_set_source_surface(ctx, imgSurf, 100, 100);
    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(img_paint_with_scale) {
    VkvgContext ctx = vkvg_create(test->surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("mirror.jpg"));

    vkvg_scale(ctx, 0.2f, 0.2f);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);

    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(img_translate) {
    VkvgContext ctx = vkvg_create(test->surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("mirror.jpg"));
    vkvg_translate(ctx, 150, 50);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);

    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(img_offset_and_scale) {
    VkvgContext ctx = vkvg_create(test->surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("mirror.jpg"));
    vkvg_scale(ctx, 0.2f, 0.2f);
    vkvg_set_source_surface(ctx, imgSurf, 100, 100);

    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}

static float angle = 0;

TEST(img_paint_with_rot) {
    VkvgContext ctx = vkvg_create(test->surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("mirror.jpg"));


    angle += 0.005;
    vkvg_clear(ctx);

    vkvg_rotate(ctx, angle);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);

    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(img_offset_and_rot) {
    VkvgContext ctx = vkvg_create(test->surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("mirror.jpg"));
    angle += 0.005;
    vkvg_clear(ctx);

    vkvg_rotate(ctx, angle);
    vkvg_set_source_surface(ctx, imgSurf, 100, 100);

    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(img_offset_and_rot_center) {
    VkvgContext ctx = vkvg_create(test->surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("mirror2.png"));
    angle += 0.005;
    vkvg_clear(ctx);

    vkvg_translate(ctx, 142,142);
    vkvg_rotate(ctx, angle);
    vkvg_translate(ctx, -142,-142);
    vkvg_set_source_surface(ctx, imgSurf, 100, 100);

    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(img_paint_pattern) {
    VkvgContext ctx = vkvg_create(test->surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("mirror2-64.png"));
    VkvgPattern pat     = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(img_paint_patt_repeat) {
    VkvgContext ctx = vkvg_create(test->surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("mirror.jpg"));
    VkvgPattern pat     = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_REPEAT);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(img_paint_patt_repeat_scalled) {
    VkvgContext ctx = vkvg_create(test->surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("mirror.jpg"));
    vkvg_scale(ctx, 0.2f, 0.2f);
    VkvgPattern pat     = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_REPEAT);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(img_paint_patt_pad) {
    VkvgContext ctx = vkvg_create(test->surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("mirror2-64.png"));
    VkvgPattern pat     = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_PAD);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
TEST(imgWithAlphaTest0) {
    VkvgContext ctx = vkvg_create(test->surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("filled.png"));

    vkvg_set_source_surface(ctx, imgSurf, 40, 40);
    vkvg_paint(ctx);

    vkvg_destroy(ctx);
    vkvg_surface_destroy(imgSurf);
}
TEST(imgWithAlphaTest1) {
    VkvgContext ctx = vkvg_create(test->surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(test->device, GET_PATH("mirror.jpg"));
    VkvgSurface imgSurf2 = vkvg_surface_create_from_image(test->device, GET_PATH("filled.png"));

    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);

    vkvg_flush(ctx);

    vkvg_set_source_surface(ctx, imgSurf2, 50, 50);
    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf2);
    vkvg_surface_destroy(imgSurf);

    vkvg_destroy(ctx);
}
/*
void test() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);

    vkvg_translate(ctx, 200, 200);
    // vkvg_rotate(ctx,M_PI_4);

    vkvg_set_line_width(ctx, 20.f);
    // vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_arc(ctx, 200, 200, 200, 0, 2.f * M_PIF);
    vkvg_new_sub_path(ctx);
    vkvg_arc(ctx, 200, 200, 100, 0, 2.f * M_PIF);

    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_fill_preserve(ctx);
    vkvg_set_source_rgba(ctx, 0.2f, 0.3f, 0.8f, 1);

    vkvg_stroke(ctx);

    vkvg_surface_destroy(imgSurf);

    vkvg_destroy(ctx);
}

void imgTest() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_line_width(ctx, 1.0); // 设置线宽
    vkvg_set_line_cap(ctx, VKVG_LINE_CAP_ROUND); // 设置线条端点样式
    vkvg_set_line_join(ctx, VKVG_LINE_JOIN_ROUND); // 设置线条连接样式

    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath3);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void imgTest2() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_line_width(ctx, 10.0); // 设置线宽
    vkvg_set_line_cap(ctx, VKVG_LINE_CAP_ROUND); // 设置线条端点样式
    vkvg_set_line_join(ctx, VKVG_LINE_JOIN_ROUND); // 设置线条连接样式

    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath3);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);

    float arcSize = 70.f;
    vkvg_translate(ctx, 20, 20);

    vkvg_arc(ctx, arcSize, arcSize, arcSize, 0, 2.f * M_PIF);
    vkvg_fill_preserve(ctx);
    vkvg_arc(ctx, arcSize, arcSize, arcSize, 0, 2.f * M_PIF);
    vkvg_set_source_rgba(ctx,0.4f,0.4f,0.9f,1);

    vkvg_stroke(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void imgTestClipped() {
    VkvgContext ctx = vkvg_create(surf);

    vkvg_set_line_width(ctx, 10.0); // 设置线宽
    vkvg_set_line_cap(ctx, VKVG_LINE_CAP_ROUND); // 设置线条端点样式
    vkvg_set_line_join(ctx, VKVG_LINE_JOIN_ROUND); // 设置线条连接样式
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD); // 设置线条连接样式

    vkvg_set_source_rgba(ctx,0,0,0,1);
    vkvg_paint(ctx);//black background, or png will be transparent


    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath3);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);

    float arcSize = 70.f;

    vkvg_arc(ctx, arcSize, arcSize, 71.f, 0, 2.f * M_PIF);
    vkvg_clip_preserve(ctx);

    vkvg_fill(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_reset_clip(ctx);
    vkvg_destroy(ctx);
}
void imgTest3() {
    VkvgContext ctx = vkvg_create(surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);
    VkvgSurface imgSurf2 = vkvg_surface_create_from_image(device, imgPath3);
    vkvg_set_operator(ctx, VKVG_OPERATOR_OVER);

    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);
    vkvg_set_operator(ctx, VKVG_OPERATOR_OVER);
    //vkvg_set_source_rgba(ctx, 1.0, 1.0, 1.0, 0.4);
    vkvg_set_source_surface(ctx, imgSurf2, 40, 40);
    vkvg_paint(ctx);

    vkvg_arc(ctx, 200, 200, 41.f, 0, M_PIF * 2);
    vkvg_fill_preserve(ctx);
    vkvg_destroy(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_surface_destroy(imgSurf2);

    //vkvg_surface_write_to_png(surface, "imgTest3.png");

    //vkvg_surface_destroy(surface);
}


void imgTest4() {
    VkvgContext ctx = vkvg_create(surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);
    VkvgSurface imgSurf2 = vkvg_surface_create_from_image(device, imgPath3);

    vkvg_set_operator(ctx, VKVG_OPERATOR_OVER);

    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);

    float arcSize = 70.f;

    //vkvg_set_operator(ctx, VKVG_OPERATOR_OVER);
    //vkvg_set_source_rgba(ctx, 1.0, 1.0, 1.0, 0.4);

    vkvg_set_source_surface(ctx, imgSurf2, 50, 50);
    //vkvg_arc(ctx, arcSize, arcSize, 60.f, 0, 2.f * M_PIF);
    vkvg_rectangle(ctx,50,50,120,120);
    vkvg_paint(ctx);

    vkvg_set_source_rgba(ctx, 1.0f, 0.0f, 0.0f, 0.9f);
    vkvg_arc(ctx, 200, 200, 21.f, 0, M_PIF * 2);
    vkvg_fill(ctx);



    //vkvg_set_line_width(ctx, 1);


    vkvg_surface_destroy(imgSurf2);
    vkvg_surface_destroy(imgSurf);

    vkvg_destroy(ctx);

    //vkvg_surface_write_to_png(surf, "imgTest4.png");
}*/


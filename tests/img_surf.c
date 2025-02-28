#include "test.h"

const char *imgPath = "data/miroir.jpg";
void        paint() {
    VkvgContext ctx     = vkvg_create(surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);

    vkvg_set_source_surface(ctx, imgSurf, 0, 0);
    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void paint_offset() {
    VkvgContext ctx     = vkvg_create(surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);

    vkvg_set_source_surface(ctx, imgSurf, 100, 100);
    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void paint_with_scale() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_scale(ctx, 0.2f, 0.2f);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);

    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void translate() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_translate(ctx, 150, 50);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);

    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void offset_and_scale() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_scale(ctx, 0.2f, 0.2f);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);
    vkvg_set_source_surface(ctx, imgSurf, 100, 100);

    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}

static float angle = 0;
void         paint_with_rot() {
    angle += 0.005;
    VkvgContext ctx = vkvg_create(surf);
    vkvg_clear(ctx);

    vkvg_rotate(ctx, angle);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);
    vkvg_set_source_surface(ctx, imgSurf, 0, 0);

    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void offset_and_rot() {
    angle += 0.005;
    VkvgContext ctx = vkvg_create(surf);
    vkvg_clear(ctx);

    vkvg_rotate(ctx, angle);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);
    vkvg_set_source_surface(ctx, imgSurf, 100, 100);

    vkvg_paint(ctx);

    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}

void paint_pattern() {
    VkvgContext ctx     = vkvg_create(surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);
    VkvgPattern pat     = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void paint_patt_repeat() {
    VkvgContext ctx     = vkvg_create(surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);
    VkvgPattern pat     = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_REPEAT);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void paint_patt_repeat_scalled() {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_scale(ctx, 0.2f, 0.2f);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);
    VkvgPattern pat     = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_REPEAT);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}
void paint_patt_pad() {
    VkvgContext ctx     = vkvg_create(surf);
    VkvgSurface imgSurf = vkvg_surface_create_from_image(device, imgPath);
    VkvgPattern pat     = vkvg_pattern_create_for_surface(imgSurf);
    vkvg_pattern_set_extend(pat, VKVG_EXTEND_PAD);
    vkvg_set_source(ctx, pat);
    vkvg_paint(ctx);
    vkvg_pattern_destroy(pat);
    vkvg_surface_destroy(imgSurf);
    vkvg_destroy(ctx);
}

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

const char* imgPath2 = "data/miroir.png";
const char* imgPath3 = "data/filled.png";

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
    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
    no_test_size = true;
    PERFORM_TEST(imgTestClipped, argc, argv);
    /*PERFORM_TEST(paint, argc, argv);
    PERFORM_TEST(paint_offset, argc, argv);
    PERFORM_TEST(paint_with_scale, argc, argv);
    PERFORM_TEST(offset_and_scale, argc, argv);
    PERFORM_TEST(translate, argc, argv);
    PERFORM_TEST(paint_with_rot, argc, argv);
    PERFORM_TEST(offset_and_rot, argc, argv);
    PERFORM_TEST(paint_pattern, argc, argv);
    PERFORM_TEST(paint_patt_repeat, argc, argv);
    PERFORM_TEST(paint_patt_repeat_scalled, argc, argv);
    PERFORM_TEST(paint_patt_pad, argc, argv);
    PERFORM_TEST(test, argc, argv);*/

    return 0;
}

#include "vkvg.h"
#include <gtest/gtest.h>

// The fixture for testing class Foo.
class ContextTest : public testing::Test {
  public:
    VkvgDevice  dev;
    VkvgSurface surf;

  protected:
    // You can remove any or all of the following functions if their bodies would
    // be empty.

    ContextTest() {
        vkvg_device_create_info_t info{};
        dev  = vkvg_device_create(&info);
        surf = vkvg_surface_create(dev, 512, 512);
    }

    ~ContextTest() override {
        vkvg_surface_destroy(surf);
        vkvg_device_destroy(dev);
    }

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(ContextTest, CtxCreate) {
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_status(NULL));

    VkvgContext ctx = vkvg_create(NULL);
    EXPECT_EQ(VKVG_STATUS_INVALID_SURFACE, vkvg_status(ctx));

    ctx = vkvg_create(surf);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_status(ctx));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));

    vkvg_destroy(ctx);
}

TEST_F(ContextTest, CtxReference) {
    EXPECT_EQ(0, vkvg_get_reference_count(NULL));

    VkvgContext ctx = vkvg_create(NULL);
    EXPECT_EQ(0, vkvg_get_reference_count(ctx));

    ctx = vkvg_create(surf);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_status(ctx));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));

    EXPECT_EQ(1, vkvg_get_reference_count(ctx));
    EXPECT_EQ(2, vkvg_surface_get_reference_count(surf));
    EXPECT_EQ(2, vkvg_device_get_reference_count(dev));

    vkvg_reference(ctx);
    EXPECT_EQ(2, vkvg_get_reference_count(ctx));

    vkvg_destroy(ctx);
    EXPECT_EQ(1, vkvg_get_reference_count(ctx));

    vkvg_destroy(ctx);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_device_status(dev));
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_surface_status(surf));
    EXPECT_EQ(0, vkvg_get_reference_count(ctx));
    EXPECT_EQ(1, vkvg_surface_get_reference_count(surf));
    EXPECT_EQ(2, vkvg_device_get_reference_count(dev));
}
TEST_F(ContextTest, CtxDrawBasicNullContext) {
    //test method with context in error
    VkvgContext ctx = vkvg_create(NULL);
    EXPECT_NO_FATAL_FAILURE(vkvg_new_path(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_close_path(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_new_sub_path(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_path_extents(ctx, NULL, NULL, NULL, NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_get_current_point(ctx, NULL, NULL));

    EXPECT_NO_FATAL_FAILURE(vkvg_line_to(ctx,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_rel_line_to(ctx,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_move_to(ctx,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_rel_move_to(ctx,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_arc(ctx,0,0,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_arc_negative(ctx,0,0,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_curve_to(ctx,0,0,0,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_rel_curve_to(ctx,0,0,0,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_quadratic_to(ctx,0,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_rel_quadratic_to(ctx,0,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_rectangle(ctx,0,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_rounded_rectangle(ctx,0,0,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_rounded_rectangle2(ctx,0,0,0,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_ellipse(ctx,0,0,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_elliptic_arc_to(ctx,0,0,0,0,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_rel_elliptic_arc_to(ctx,0,0,0,0,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_stroke(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_stroke_preserve(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_fill(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_fill_preserve(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_paint(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_clear(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_reset_clip(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_clip(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_clip_preserve(ctx));

    EXPECT_NO_FATAL_FAILURE(vkvg_set_opacity(ctx,0));

    EXPECT_EQ(0, vkvg_get_opacity(ctx));

    EXPECT_NO_FATAL_FAILURE(vkvg_set_source_color(ctx,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_set_source_rgb(ctx,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_set_source_rgba(ctx,0,0,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_set_source_surface(ctx,NULL,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_set_source(ctx,NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_set_line_width(ctx,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_set_miter_limit(ctx,0));

    EXPECT_EQ(0, vkvg_get_miter_limit(ctx));

    EXPECT_NO_FATAL_FAILURE(vkvg_set_line_cap(ctx, VKVG_LINE_CAP_ROUND));
    EXPECT_NO_FATAL_FAILURE(vkvg_set_line_join(ctx, VKVG_LINE_JOIN_MITER));
    EXPECT_NO_FATAL_FAILURE(vkvg_set_operator(ctx, VKVG_OPERATOR_OVER));
    EXPECT_NO_FATAL_FAILURE(vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD));
    EXPECT_NO_FATAL_FAILURE(vkvg_set_dash(ctx, NULL, 0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_get_dash(ctx, NULL, NULL,NULL));

    EXPECT_EQ(0, vkvg_get_line_width(ctx));

    EXPECT_EQ(VKVG_LINE_CAP_BUTT, vkvg_get_line_cap(ctx));
    EXPECT_EQ(VKVG_LINE_JOIN_MITER, vkvg_get_line_join(ctx));
    EXPECT_EQ(VKVG_OPERATOR_OVER, vkvg_get_operator(ctx));
    EXPECT_EQ(VKVG_FILL_RULE_NON_ZERO, vkvg_get_fill_rule(ctx));

    EXPECT_NO_FATAL_FAILURE(vkvg_get_source(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_get_target(ctx));

    EXPECT_EQ(false, vkvg_has_current_point(ctx));

    EXPECT_NO_FATAL_FAILURE(vkvg_save(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_restore(ctx));

    EXPECT_NO_FATAL_FAILURE(vkvg_translate(ctx,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_scale(ctx,0,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_rotate(ctx,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_transform(ctx,NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_set_matrix(ctx,NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_get_matrix(ctx,NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_identity_matrix(ctx));

    EXPECT_NO_FATAL_FAILURE(vkvg_select_font_face(ctx,NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_load_font_from_path(ctx,NULL,NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_load_font_from_memory(ctx,NULL,0,NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_set_font_size(ctx,0));
    EXPECT_NO_FATAL_FAILURE(vkvg_show_text(ctx,NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_text_extents(ctx,NULL,NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_font_extents(ctx,NULL));

    vkvg_text_run_create(ctx,NULL);
    vkvg_text_run_create_with_length(ctx,NULL,10);

    //vkvg_text_run_destroy(NULL);
    vkvg_show_text_run(ctx,NULL);
}
#define EXPECT_CP(x,y) { \
    vkvg_get_current_point(ctx, &a, &b);\
    EXPECT_FLOAT_EQ(x, a);\
    EXPECT_FLOAT_EQ(y, b);\
}
#define EXPECT_NO_CP() {\
    EXPECT_EQ(false, vkvg_has_current_point(ctx));\
    EXPECT_CP(0,0);\
}

TEST_F(ContextTest, CtxBasicPathCommands) {
    float a = 0.0,b = 0.0,c = 0.0,d = 0.0;
    VkvgContext ctx = vkvg_create(surf);
    EXPECT_NO_CP();

    EXPECT_NO_FATAL_FAILURE(vkvg_new_path(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_close_path(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_new_sub_path(ctx));
    EXPECT_NO_FATAL_FAILURE(vkvg_path_extents(ctx, &a, &b, &c, &d));
    EXPECT_FLOAT_EQ(0, a && b && c && d);

    EXPECT_NO_CP();

    vkvg_line_to(ctx, 50, 10);
    EXPECT_CP(50,10);

    vkvg_move_to(ctx, 10, 50);
    EXPECT_EQ(true, vkvg_has_current_point(ctx));
    EXPECT_CP(10,50);

    vkvg_line_to(ctx, 50, 10);
    EXPECT_CP(50,10);

    vkvg_rel_line_to(ctx, 10, 10);
    EXPECT_CP(60,20);

    vkvg_close_path(ctx);

    EXPECT_NO_CP();

    vkvg_line_to(ctx, 50, 10);
    vkvg_rel_line_to(ctx, 10, 10);
    vkvg_new_sub_path(ctx);

    EXPECT_NO_CP();

    vkvg_line_to(ctx, 50, 10);
    vkvg_rel_line_to(ctx, 10, 10);
    vkvg_new_path(ctx);

    EXPECT_NO_CP();

    vkvg_destroy(ctx);
}

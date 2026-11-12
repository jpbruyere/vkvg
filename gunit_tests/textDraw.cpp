#include "drawTestBase.h"
#include "unitTest.h"


class TextDrawTest : public DrawTestBase {

  protected:
    void SetUp() override {
        surf = vkvg_surface_create(dev, 512, 512);

    }
    void TearDown() override {
        DrawTestBase::TearDown();
    }
};
static const char* txt = "Galvanized by the quartz, the white rabbit just jumped on the red fox for the sake of the crows";

TEST_F(TextDrawTest, References) {
    VkvgFont font = vkvg_font_create(dev, "sans", 12);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_font_status(font));
    vkvg_font_destroy(font);

    font = vkvg_font_create(dev, "Sans", 12);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_font_status(font));
    vkvg_font_destroy(font);

    font = vkvg_font_create(dev, "noto sans jp", 12);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_font_status(font));
    vkvg_font_destroy(font);


    /*EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_font_status(NULL));
    EXPECT_EQ(0, vkvg_pattern_get_reference_count(NULL));
    EXPECT_NO_FATAL_FAILURE(vkvg_pattern_reference(NULL));

    VkvgPattern pat = vkvg_pattern_create_for_surface(NULL);
    EXPECT_EQ(VKVG_STATUS_NULL_POINTER, vkvg_pattern_status(pat));

    pat = vkvg_pattern_create_linear(0, 0, 0, 0);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_status(pat));
    EXPECT_EQ(1, vkvg_pattern_get_reference_count(pat));
    vkvg_pattern_reference(pat);
    EXPECT_EQ(2, vkvg_pattern_get_reference_count(pat));

    EXPECT_NO_FATAL_FAILURE(vkvg_pattern_destroy(NULL));
    vkvg_pattern_destroy(pat);
    EXPECT_EQ(1, vkvg_pattern_get_reference_count(pat));
    vkvg_pattern_destroy(pat);

    pat = vkvg_pattern_create_radial(0, 0, 0, 0, 0, 0);
    EXPECT_EQ(VKVG_STATUS_SUCCESS, vkvg_pattern_status(pat));
    EXPECT_EQ(1, vkvg_pattern_get_reference_count(pat));
    vkvg_pattern_destroy(pat);*/
}

TEST_F(TextDrawTest, ShowText) {

    VkvgContext ctx = vkvg_create(surf);
    vkvg_paint(ctx);
    VkvgFont font = vkvg_font_create(dev, "sans", 30);
    vkvg_set_font(ctx, font);
    vkvg_font_extents_t fe = {0};
    vkvg_font_extents(ctx, &fe);
    vkvg_set_source_rgb(ctx,1,1,1);
    float penY = fe.ascent;
    vkvg_move_to(ctx, 40, penY);
    vkvg_show_text(ctx, txt);
    penY += fe.height;

    vkvg_font_destroy(font);

    font = vkvg_font_create(dev, "noto sans jp", 30);
    vkvg_set_font(ctx, font);
    vkvg_move_to(ctx, 40, penY);
    vkvg_show_text(ctx, txt);
    vkvg_font_extents(ctx, &fe);
    penY += fe.height;

    vkvg_move_to(ctx, 40, penY);
    vkvg_show_text(ctx, "いろはにほへと ちりぬるを わかよたれそ つねならむ");
    vkvg_font_extents(ctx, &fe);
    penY += fe.height;

    vkvg_set_text_direction(ctx, VKVG_VERTICAL);
    vkvg_move_to(ctx, 20, 60);
    vkvg_show_text(ctx, "石英に刺激され、白ウサギはカラスたちのために、赤ギツネの上に飛び乗った。");

    vkvg_font_destroy(font);

    vkvg_set_text_direction(ctx, VKVG_HORIZONTAL);
    font = vkvg_font_create(dev, "Roboto:bold", 30);
    vkvg_set_font(ctx, font);
    vkvg_move_to(ctx, 40, penY);
    vkvg_show_text(ctx, txt);
    vkvg_font_extents(ctx, &fe);
    penY += fe.height;

    vkvg_destroy(ctx);

    compareWithRefImage();
}

TEST_F(TextDrawTest, scale_up) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_clear(ctx);

    vkvg_set_source_rgb(ctx, 0, 0, 0);
    vkvg_paint(ctx);
    vkvg_set_source_rgb(ctx, 1, 1, 1);

    VkvgFont font = vkvg_font_create(dev, "sans", 12);

    vkvg_set_font(ctx,font);
    vkvg_move_to(ctx, 2, 15);
    vkvg_scale(ctx, 10,10);
    vkvg_show_text(ctx, txt);

    vkvg_font_destroy(font);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(TextDrawTest, scale_down) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_clear(ctx);

    vkvg_set_source_rgb(ctx, 0, 0, 0);
    vkvg_paint(ctx);
    vkvg_set_source_rgb(ctx, 1, 1, 1);

    VkvgFont font = vkvg_font_create(dev, "sans", 60);

    vkvg_set_font(ctx,font);
    vkvg_move_to(ctx, 200, 200);
    vkvg_scale(ctx, 0.1f,0.1f);
    vkvg_show_text(ctx, txt);
    vkvg_font_destroy(font);
    vkvg_destroy(ctx);

    compareWithRefImage();
}
TEST_F(TextDrawTest, text_on_path) {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgb(ctx, 0, 0, 0);
    vkvg_paint(ctx);
    vkvg_set_source_rgb(ctx, 1, 1, 1);
    VkvgFont font = vkvg_font_create(dev, "sans", 20);

    vkvg_rotate(ctx, -0.2f);
    vkvg_scale(ctx, 1.5f,1.5f);
    vkvg_set_font(ctx,font);

    vkvg_move_to(ctx, 10, 250);
    vkvg_curve_to(ctx, 140, 0, 200, 450, 450, 250);

    VkvgText tr = vkvg_text_create(ctx, txt);
    vkvg_text_render_on_path(ctx, tr, 0, 0, true);
    vkvg_set_source_rgb(ctx, 1, 0, 0);
    vkvg_stroke(ctx);

    vkvg_text_destroy(tr);

    vkvg_font_destroy(font);
    vkvg_destroy(ctx);

    compareWithRefImage();
}

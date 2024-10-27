/*
 * Copyright (c) 2018-2022 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "test.h"

void _test_rounded_rect(VkvgContext cr) {
    /* a custom shape that could be wrapped in a function */
    float x0 = -100,                                                     /* parameters like vkvg_rectangle */
        y0 = -100, rect_width = 200, rect_height = 200, radius = 102.4f; /* and an approximate curvature radius */

    float x1, y1;

    x1 = x0 + rect_width;
    y1 = y0 + rect_height;
    if (!rect_width || !rect_height)
        return;
    if (rect_width / 2 < radius) {
        if (rect_height / 2 < radius) {
            vkvg_move_to(cr, x0, (y0 + y1) / 2);
            vkvg_curve_to(cr, x0, y0, x0, y0, (x0 + x1) / 2, y0);
            vkvg_curve_to(cr, x1, y0, x1, y0, x1, (y0 + y1) / 2);
            vkvg_curve_to(cr, x1, y1, x1, y1, (x1 + x0) / 2, y1);
            vkvg_curve_to(cr, x0, y1, x0, y1, x0, (y0 + y1) / 2);
        } else {
            vkvg_move_to(cr, x0, y0 + radius);
            vkvg_curve_to(cr, x0, y0, x0, y0, (x0 + x1) / 2, y0);
            vkvg_curve_to(cr, x1, y0, x1, y0, x1, y0 + radius);
            vkvg_line_to(cr, x1, y1 - radius);
            vkvg_curve_to(cr, x1, y1, x1, y1, (x1 + x0) / 2, y1);
            vkvg_curve_to(cr, x0, y1, x0, y1, x0, y1 - radius);
        }
    } else {
        if (rect_height / 2 < radius) {
            vkvg_move_to(cr, x0, (y0 + y1) / 2);
            vkvg_curve_to(cr, x0, y0, x0, y0, x0 + radius, y0);
            vkvg_line_to(cr, x1 - radius, y0);
            vkvg_curve_to(cr, x1, y0, x1, y0, x1, (y0 + y1) / 2);
            vkvg_curve_to(cr, x1, y1, x1, y1, x1 - radius, y1);
            vkvg_line_to(cr, x0 + radius, y1);
            vkvg_curve_to(cr, x0, y1, x0, y1, x0, (y0 + y1) / 2);
        } else {
            vkvg_move_to(cr, x0, y0 + radius);
            vkvg_curve_to(cr, x0, y0, x0, y0, x0 + radius, y0);
            vkvg_line_to(cr, x1 - radius, y0);
            vkvg_curve_to(cr, x1, y0, x1, y0, x1, y0 + radius);
            vkvg_line_to(cr, x1, y1 - radius);
            vkvg_curve_to(cr, x1, y1, x1, y1, x1 - radius, y1);
            vkvg_line_to(cr, x0 + radius, y1);
            vkvg_curve_to(cr, x0, y1, x0, y1, x0, y1 - radius);
        }
    }
    vkvg_close_path(cr);

    vkvg_set_source_rgb(cr, 0.5f, 0.5f, 1);
    vkvg_fill_preserve(cr);
    vkvg_set_source_rgba(cr, 0.5f, 0, 0, 0.5f);
    vkvg_set_line_width(cr, 10.0f);
    vkvg_stroke(cr);
}

static float rotation = 0.f;

void cairo_tests() {
    rotation += 0.002f;

    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
    vkvg_set_source_rgba(ctx, 1.0f, 1.0f, 1.0f, 1);
    vkvg_paint(ctx);

    vkvg_set_line_width(ctx, 10);

    vkvg_set_source_rgb(ctx, 0, 0, 1);

    vkvg_translate(ctx, test_width / 2, test_height / 2);
    vkvg_rotate(ctx, rotation);
    // vkvg_scale(ctx, 200,200);

    vkvg_arc(ctx, 0, 0, 150, 0, M_PI * 1.5);
    vkvg_stroke(ctx);

    _test_rounded_rect(ctx);

    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {
    no_test_size = true;
    PERFORM_TEST(cairo_tests, argc, argv);
    return 0;
}

/*
 * Copyright (c) 2018 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
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

#include "vkengine.h"
#include "vkvg.h"
#include "vkh_device.h"
#include "vkh_presenter.h"
#include <sys/time.h>
#include <stdlib.h>

#include "perf-test.h"



VkvgDevice device;
/*VkvgSurface surf = NULL;

void simple_paint () {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,1,0,0,1);
    vkvg_paint(ctx);
    vkvg_destroy(ctx);
}
void simple_rectangle_fill () {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,0,1,0,1);
    vkvg_rectangle(ctx,100,100,200,200);
    vkvg_fill(ctx);
    vkvg_destroy(ctx);
}
void simple_rectangle_stroke () {
    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_source_rgba(ctx,0,0,1,1);
    vkvg_set_line_width(ctx,10.f);
    vkvg_rectangle(ctx,100,100,200,200);
    vkvg_stroke(ctx);
    vkvg_destroy(ctx);
}*/

vkvg_color_t getRadomColor () {
    vkvg_color_t c = {
        (float)rand()/RAND_MAX,
        (float)rand()/RAND_MAX,
        (float)rand()/RAND_MAX,
        (float)rand()/RAND_MAX,
    };
}

void drawRandomRectangle (options_t* opt, VkvgContext ctx) {
    for (int i=0; i<opt->count; i++) {
        vkvg_color_t c = getRadomColor();
        vkvg_set_source_rgba(ctx, c.r, c.g, c.b, c.a);

        float x = (float)rand()/RAND_MAX * opt->width - 10;
        float y = (float)rand()/RAND_MAX * opt->height - 10;
        float w = (float)rand()/RAND_MAX * 200 + 10;
        float h = (float)rand()/RAND_MAX * 200 + 10;

        vkvg_rectangle(ctx, x, y, w, h);
    }
    if (opt->drawMode == DM_BOTH)
        vkvg_fill_preserve(ctx);
    else if (opt->drawMode == DM_FILL)
        vkvg_fill (ctx);
    if (opt->drawMode & DM_STROKE)
        vkvg_stroke (ctx);
}

results_t performTest (options_t* opt) {
    results_t res = initResults();

    double run_time_values[opt->iterations];
    double start_time, stop_time, run_time, run_total = 0;

    VkvgSurface surf = vkvg_surface_create(device, opt->width, opt->height);
    void* ctx = vkvg_create(surf);

    for (int i=0; i<opt->iterations; i++) {

        start_time = get_tick();

        drawRandomRectangle (opt, ctx);

        stop_time = get_tick();

        run_time = stop_time - start_time;
        run_time_values[i] = run_time;

        if (run_time < res.run_min)
            res.run_min = run_time;
        if (run_time > res.run_max)
            res.run_max = run_time;
        run_total += run_time;
    }

    vkvg_destroy (ctx);
    vkvg_surface_destroy (surf);

    res.avg_time = run_total / (double)opt->iterations;
    res.median_time = median_run_time(run_time_values, opt->iterations);
    res.std_deriv = standard_deviation(run_time_values,opt->iterations, res.avg_time);

    return res;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    options_t opt = initOptions(argc, argv);
    opt.test_name = "vkvg rectangles and";

    vk_engine_t* e = vkengine_create (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, 1024, 800);
    VkhPresenter r = e->renderer;

    device  = vkvg_device_create (r->dev->phy, r->dev->dev, r->qFam, 0);

    results_t res = performTest (&opt);

    //outputResults(&opt, &res);
    outputResultsHeadRow(&opt);
    outputResultsOnOneLine(&opt, &res);

    vkvg_device_destroy     (device);

    vkengine_destroy (e);

    return 0;
}

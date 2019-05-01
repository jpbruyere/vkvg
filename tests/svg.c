#include "test.h"

#include "string.h" //for nanosvg
#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "nanosvg.h"

void svg_set_color (VkvgContext ctx, uint32_t c, float alpha) {
    float a = (c >> 24 & 255) / 255.f;
    float b = (c >> 16 & 255) / 255.f;
    float g = (c >> 8 & 255) / 255.f;
    float r = (c & 255) / 255.f;
    vkvg_set_source_rgba(ctx,r,g,b,a*alpha);
}

static float rotation = 0.f;

void test_svg () {
    rotation+=0.01f;

    vkvg_matrix_t mat;
    vkvg_matrix_init_translate (&mat, 512,400);
    vkvg_matrix_rotate(&mat,rotation);
    vkvg_matrix_translate(&mat,-512,-400);

    VkvgContext ctx = vkvg_create(surf);
    vkvg_set_fill_rule(ctx, VKVG_FILL_RULE_EVEN_ODD);
    vkvg_set_source_rgba(ctx,1.0,1.0,1.0,1);
    vkvg_paint(ctx);

    vkvg_set_matrix(ctx,&mat);

    NSVGimage* svg;
    NSVGshape* shape;
    NSVGpath* path;
    //svg = nsvgParseFromFile("/mnt/data/images/svg/tux.svg", "px", 96);
    //svg = nsvgParseFromFile("/mnt/data/images/svg/world.svg", "px", 96);
    svg = nsvgParseFromFile("data/tiger.svg", "px", 96);
    //svg = nsvgParseFromFile("/mnt/data/images/svg/koch_curve.svg", "px", 96);
    //svg = nsvgParseFromFile("/mnt/data/images/svg/diamond1.svg", "px", 96);
    //svg = nsvgParseFromFile("/mnt/data/images/svg/diamond2.svg", "px", 96);
    //svg = nsvgParseFromFile("/home/jp/yahweh-protosinaitic.svg", "px", 96);
    //svg = nsvgParseFromFile("/mnt/data/images/svg/WMD-biological.svg", "px", 96);
    //svg = nsvgParseFromFile("/mnt/data/images/svg/Skull_and_crossbones.svg", "px", 96);
    //svg = nsvgParseFromFile("/mnt/data/images/svg/IconAlerte.svg", "px", 96);
    //svg = nsvgParseFromFile("/mnt/data/images/svg/Svg_example4.svg", "px", 96);

    //vkvg_scale(ctx, 3,3);
    vkvg_set_source_rgba(ctx,0.0,0.0,0.0,1);

    for (shape = svg->shapes; shape != NULL; shape = shape->next) {

        vkvg_new_path(ctx);

        float o = shape->opacity;

        vkvg_set_line_width(ctx, shape->strokeWidth);

        for (path = shape->paths; path != NULL; path = path->next) {
            float* p = path->pts;
            vkvg_move_to(ctx, p[0],p[1]);
            for (int i = 1; i < path->npts-2; i += 3) {
                p = &path->pts[i*2];
                vkvg_curve_to(ctx, p[0],p[1], p[2],p[3], p[4],p[5]);
            }
            if (path->closed)
                vkvg_close_path(ctx);
        }

        if (shape->fill.type == NSVG_PAINT_COLOR)
            svg_set_color(ctx, shape->fill.color, o);
        else if (shape->fill.type == NSVG_PAINT_LINEAR_GRADIENT){
            NSVGgradient* g = shape->fill.gradient;
            svg_set_color(ctx, g->stops[0].color, o);
        }

        if (shape->fill.type != NSVG_PAINT_NONE){
            if (shape->stroke.type == NSVG_PAINT_NONE){
                vkvg_fill(ctx);
                continue;
            }
            vkvg_fill_preserve (ctx);
        }

        if (shape->stroke.type == NSVG_PAINT_COLOR)
            svg_set_color(ctx, shape->stroke.color, o);
        else if (shape->stroke.type == NSVG_PAINT_LINEAR_GRADIENT){
            NSVGgradient* g = shape->stroke.gradient;
            svg_set_color(ctx, g->stops[0].color, o);
        }

        vkvg_stroke(ctx);
    }

    nsvgDelete(svg);


    vkvg_destroy(ctx);
}

int main(int argc, char *argv[]) {

    perform_test (test_svg, 1024, 768);
    return 0;
}

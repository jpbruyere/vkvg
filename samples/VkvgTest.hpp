#pragma once

#include "rnd.h"

#include <string>
#include <vector>
#include <filesystem>

#include "SampleApp.hpp"

#define M_PIF        3.14159265359f /* float pi */
#define M_PIF_MULT_2 6.28318530718f
#ifndef M_PI
#define M_PI 3.14159265358979323846 /* pi */
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923 /* pi/2 */
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

namespace fs = std::filesystem;
#define GET_PATH(fileName) (char*)(fs::path(SAMPLES_DATA_ROOT) / fileName).c_str()

typedef enum _shape_t {
    SHAPE_LINE,
    SHAPE_RECTANGLE,
    SHAPE_ROUNDED_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE,
    SHAPE_STAR,
    SHAPE_RANDOM,
} shape_t;

class VkvgTest;
typedef void (*VkvgTestFunc)(VkvgTest* test);

class VkvgTest {
    VkvgTestFunc testFunc;
    double totTime;
    double avg_run_time;
    double med_run_time;
    double standard_dev;
    double avg_frames_per_second;

  public:
    static std::vector<VkvgTest*> tests;

    SampleApp*  app;
    std::string name;
    int         index;

    VkvgTest(VkvgTestFunc testFunc, std::string _name);

    VkvgDevice  device;
    VkvgSurface surf;


    void initTest(SampleApp* app);
    int performTest(int inst);
    void cleanTest(double _totTime, double *run_time_values);

    void draw_random_shape(VkvgContext ctx, shape_t shape, float sizeFact);
    void draw_random_curve(VkvgContext ctx);
    void draw_random_square(VkvgContext ctx, float s);
};
#define TEST(name)                                                                                                     \
    static void     name(VkvgTest* test);                                                                              \
    static VkvgTest _##name(name, #name);                                                                              \
    static void     name(VkvgTest* test)

void draw_growing_circles(VkvgContext ctx, float y, int count);
void randomize_color(VkvgContext ctx);

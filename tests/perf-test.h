#ifndef PERFTEST_H
#define PERFTEST_H

#include "vkvg.h"
#include <stdlib.h>
#include <float.h>

enum DrawMode {
    DM_FILL = 0x1,
    DM_STROKE = 0x2,
    DM_BOTH = 0x3
};

typedef struct _results {
    double run_min;
    double run_max;
    double avg_time;
    double median_time;
    double std_deriv;
} results_t;

results_t initResults () {
    results_t res = {};
    res.run_min = DBL_MAX;
    res.run_max = DBL_MIN;
    return res;
}

typedef struct _options {
    char*       test_name;
    int         iterations;
    int         count;
    int         width;
    int         height;
    enum DrawMode    drawMode;
} options_t;

options_t initOptions (int argc, char *argv[]) {
    options_t opt = {};
    opt.iterations = 100;
    opt.count = 1000;
    opt.width = 1024;
    opt.height = 800;
    opt.drawMode = DM_FILL;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
            case 'w':
                sscanf (argv[++i], "%d", &opt.width);
                break;
            case 'h':
                sscanf (argv[++i], "%d", &opt.height);
                break;
            case 'i':
                sscanf (argv[++i], "%d", &opt.iterations);
                break;
            case 'c':
                sscanf (argv[++i], "%d", &opt.count);
                break;
            case 'f':
                opt.drawMode = DM_FILL;
                break;
            case 's':
                opt.drawMode = DM_STROKE;
                break;
            case 'b':
                opt.drawMode = DM_BOTH;
                break;
            default:
                break;
            }
        }
    }
    return opt;
}


void printHelp () {
    printf ("\t-w x : Set test surface width.\n");
    printf ("\t-h x : Set test surface height.\n");
    printf ("\t-i x : Set iterations count.\n");
    printf ("\t-c x : Set shape occurence count.\n");
    printf ("\t-f :   Set shape draw mode to fill.\n");
    printf ("\t-s :   Set shape draw mode to stroke.\n");
    printf ("\t-b :   Set shape draw mode to fill and stroke.\n");
}
void outputResultsHeadRow (options_t* opt) {
    printf ("__________________________________________________________________________________________________________\n");
    printf ("| Test Name       | Iter | Cpt  | Resolution  |DM |   Min    |   Max    |  Average |  Median  | Std Deriv|\n");
    printf ("|-----------------|------|------|-------------|---|----------|----------|----------|----------|----------|\n");
}
void outputResultsOnOneLine (options_t* opt, results_t* res) {
    printf ("| %.15s | %4d | %4d | %4d x %4d | ",
            opt->test_name, opt->iterations, opt->count, opt->width, opt->height);
    switch (opt->drawMode) {
    case DM_BOTH:
        printf ("B | ");
        break;
    case DM_FILL:
        printf ("F | ");
        break;
    case DM_STROKE:
        printf ("S | ");
        break;
    }
    printf ("%.6f | %.6f | %.6f | %.6f | %.6f |",
            res->run_min, res->run_max, res->avg_time, res->median_time, res->std_deriv);
}

void outputResults (options_t* opt, results_t* res) {
    printf ("Test name: %s\n", opt->test_name);
    printf ("\nOptions\n");
    printf ("=======\n");
    printf ("Iterations  = %d\n", opt->iterations);
    printf ("Shape count = %d\n", opt->count);
    printf ("Canva size  = %d x %d\n", opt->width, opt->height);
    switch (opt->drawMode) {
    case DM_BOTH:
        printf ("Draw Mode   = FILL and STROKE\n");
        break;
    case DM_FILL:
        printf ("Draw Mode   = FILL\n");
        break;
    case DM_STROKE:
        printf ("Draw Mode   = STROKE\n");
        break;
    }
    printf ("\nResults\n");
    printf ("=======\n");
    printf ("Minimum = %f (sec)\n", res->run_min);
    printf ("Maximum = %f (sec)\n", res->run_max);
    printf ("Average = %f (sec)\n", res->avg_time);
    printf ("Median  = %f (sec)\n", res->median_time);
    printf ("Std reriv = %f \n", res->std_deriv);
}

double get_tick (void)
{
    struct timeval now;
    gettimeofday (&now, NULL);
    return (double)now.tv_sec + (double)now.tv_usec / 1000000.0;
}
double median_run_time (double data[], int n)
{
    double temp;
    int i, j;
    for (i = 0; i < n; i++)
        for (j = i+1; j < n; j++)
        {
            if (data[i] > data[j])
            {
                temp = data[j];
                data[j] = data[i];
                data[i] = temp;
            }
        }
    if (n % 2 == 0)
        return (data[n/2] + data[n/2-1])/2;
    else
        return data[n/2];
}
double standard_deviation (const double data[], int n, double mean)
{
    double sum_deviation = 0.0;
    int i;
    for (i = 0; i < n; ++i)
    sum_deviation += (data[i]-mean) * (data[i]-mean);
    return sqrt (sum_deviation / n);
}

#endif // PERFTEST_H

#include "vkengine.h"
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

#include "vkvg.h"

#include "vkh_device.h"
#include "vkh_presenter.h"

#define M_PIF               3.14159265359f /* float pi */
#define M_PIF_MULT_2        6.28318530718f

#ifndef MIN
# define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
# define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#define PERFORM_TEST(testName) perform_test(testName, #testName, 1024, 768);
#if defined(_WIN32) || defined(_WIN64)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h> // Windows.h -> WinDef.h defines min() max()

    /*
        typedef uint16_t WORD ;
        typedef uint32_t DWORD;

        typedef struct _FILETIME {
            DWORD dwLowDateTime;
            DWORD dwHighDateTime;
        } FILETIME;

        typedef struct _SYSTEMTIME {
              WORD wYear;
              WORD wMonth;
              WORD wDayOfWeek;
              WORD wDay;
              WORD wHour;
              WORD wMinute;
              WORD wSecond;
              WORD wMilliseconds;
        } SYSTEMTIME, *PSYSTEMTIME;
    */

    // *sigh* Microsoft has this in winsock2.h because they are too lazy to put it in the standard location ... !?!?
    typedef struct timeval {
        long tv_sec;
        long tv_usec;
    } timeval;

    // *sigh* no gettimeofday on Win32/Win64
    int gettimeofday(struct timeval * tp, void * tzp);
#else
    #include <sys/time.h>
#endif

extern uint32_t test_size;
extern int iterations;

extern float panX;
extern float panY;
extern float lastX;
extern float lastY;
extern float zoom;
extern bool mouseDown;

extern VkvgDevice device;
extern VkvgSurface surf;

//run test in one step
void perform_test (void(*testfunc)(), const char* testName, uint32_t width, uint32_t height);
void randomize_color (VkvgContext ctx);

//run test in 3 step: init, run, clear.
void init_test (uint32_t width, uint32_t height);
void run_test_func (void(*testfunc)(), uint32_t width, uint32_t height);
void clear_test ();

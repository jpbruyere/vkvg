#include "vkengine.h"
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

#include "vkvg.h"

#include "vkh_device.h"
#include "vkh_presenter.h"
#include "vkh_phyinfo.h"

#define M_PIF               3.14159265359f /* float pi */
#define M_PIF_MULT_2        6.28318530718f

#ifndef MIN
# define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
# define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifdef VKVG_TEST_OFFSCREEN
#define PERFORM_TEST(testName, argc, argv) perform_test_offscreen(testName, #testName, argc, argv);
#else
#define PERFORM_TEST(testName, argc, argv) perform_test(testName, #testName, argc, argv);
#endif
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

typedef enum _shape_t {
	SHAPE_LINE,
	SHAPE_RECTANGLE,
	SHAPE_ROUNDED_RECTANGLE,
	SHAPE_CIRCLE,
	SHAPE_TRIANGLE,
	SHAPE_STAR,
	SHAPE_RANDOM,
} shape_t;

extern uint32_t test_size;
extern uint32_t iterations;
extern uint32_t test_width;
extern uint32_t test_height;

extern float panX;
extern float panY;
extern float lastX;
extern float lastY;
extern float zoom;
extern bool mouseDown;

extern VkvgDevice device;
extern VkvgSurface surf;

//run test in one step
void perform_test (void(*testfunc)(), const char* testName, int argc, char *argv[]);
void perform_test_offscreen (void(*testfunc)(void), const char *testName, int argc, char* argv[]);

void randomize_color	(VkvgContext ctx);
void draw_random_shape	(VkvgContext ctx, shape_t shape, float sizeFact);

//run test in 3 step: init, run, clear.
void init_test (uint32_t width, uint32_t height);
void clear_test ();

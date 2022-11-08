#include "vkengine.h"
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

#include "rnd.h"
#include "vkvg.h"

#include "vkh_device.h"
#include "vkh_presenter.h"
#include "vkh_phyinfo.h"

#define M_PIF               3.14159265359f /* float pi */
#define M_PIF_MULT_2        6.28318530718f
#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif
#ifndef M_PI_2
# define M_PI_2		1.57079632679489661923	/* pi/2 */
#endif

#ifndef MIN
# define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
# define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#define PERFORM_TEST(testName, argc, argv) perform_test(testName, #testName, argc, argv);

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
extern bool no_test_size;

extern float panX;
extern float panY;
extern float lastX;
extern float lastY;
extern float zoom;
extern bool mouseDown;

extern VkvgDevice device;
extern VkvgSurface surf;

/* common context init for several tests */
extern float dash_offset;
extern float line_width;
extern vkvg_fill_rule_t fill_rule;
extern vkvg_line_cap_t line_cap;
extern vkvg_line_join_t line_join;
extern float dashes[];
extern uint32_t dashes_count;

VkvgContext _initCtx();
void _parse_args (int argc, char* argv[]);
/*******************************/

//run test in one step
void perform_test (void(*testfunc)(), const char* testName, int argc, char *argv[]);
void perform_test_onscreen (void(*testfunc)(void), const char *testName, int argc, char* argv[]);
void perform_test_offscreen (void(*testfunc)(void), const char *testName, int argc, char* argv[]);

void randomize_color	(VkvgContext ctx);
void draw_random_shape	(VkvgContext ctx, shape_t shape, float sizeFact);
void draw_random_curve (VkvgContext ctx);

//run test in 3 step: init, run, clear.
void init_test (uint32_t width, uint32_t height);
void clear_test ();

#include "vkengine.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "vkvg.h"

#include "vkh_device.h"
#include "vkh_presenter.h"

#define M_PIF               3.14159265359f /* float pi */
#define M_PIF_MULT_2        6.28318530718f

#ifdef _WIN32 // MSC_VER
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
    int gettimeofday(struct timeval * tp, struct timezone * tzp)
    {
        // FILETIME Jan 1 1970 00:00:00
        // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
        static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

        SYSTEMTIME  nSystemTime;
        FILETIME    nFileTime;
        uint64_t    nTime;

        GetSystemTime( &nSystemTime );
        SystemTimeToFileTime( &nSystemTime, &nFileTime );
        nTime =  ((uint64_t)nFileTime.dwLowDateTime )      ;
        nTime += ((uint64_t)nFileTime.dwHighDateTime) << 32;

        tp->tv_sec  = (long) ((nTime - EPOCH) / 10000000L);
        tp->tv_usec = (long) (nSystemTime.wMilliseconds * 1000);
        return 0;
    }
#else
    #include <sys/time.h>
#endif // _WIN32

extern uint iterations;
extern uint runs;

extern float panX;
extern float panY;
extern float lastX;
extern float lastY;
extern float zoom;
extern bool mouseDown;

extern VkvgDevice device;
extern VkvgSurface surf;

//run test in one step
void perform_test (void(*testfunc)(void),uint width, uint height);
void randomize_color (VkvgContext ctx);

//run test in 3 step: init, run, clear.
void init_test (uint width, uint height);
void run_test_func (void(*testfunc)(void),uint width, uint height);
void clear_test ();

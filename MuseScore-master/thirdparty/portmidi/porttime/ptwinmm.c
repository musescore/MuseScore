/* ptwinmm.c -- portable timer implementation for win32 */


#include "porttime.h"
#include "windows.h"
#include "time.h"


TIMECAPS caps;

static long time_offset = 0;
static int time_started_flag = FALSE;
static long time_resolution;
static MMRESULT timer_id;
static PtCallback *time_callback;

void CALLBACK winmm_time_callback(UINT uID, UINT uMsg, DWORD dwUser, 
                                  DWORD dw1, DWORD dw2)
{
    (*time_callback)(Pt_Time(), (void *) dwUser);
}
 

PtError Pt_Start(int resolution, PtCallback *callback, void *userData)
{
    if (time_started_flag) return ptAlreadyStarted;
    timeBeginPeriod(resolution);
    time_resolution = resolution;
    time_offset = timeGetTime();
    time_started_flag = TRUE;
    time_callback = callback;
    if (callback) {
        timer_id = timeSetEvent(resolution, 1, winmm_time_callback, 
            (DWORD) userData, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
        if (!timer_id) return ptHostError;
    }
    return ptNoError;
}


PtError Pt_Stop()
{
    if (!time_started_flag) return ptAlreadyStopped;
    if (time_callback && timer_id) {
        timeKillEvent(timer_id);
        time_callback = NULL;
        timer_id = 0;
    }
    time_started_flag = FALSE;
    timeEndPeriod(time_resolution);
    return ptNoError;
}


int Pt_Started()
{
    return time_started_flag;
}


PtTimestamp Pt_Time()
{
    return timeGetTime() - time_offset;
}


void Pt_Sleep(long duration)
{
    Sleep(duration);
}

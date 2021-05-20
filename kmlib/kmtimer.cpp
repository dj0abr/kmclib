/*
* Linux multithreaded timer
* =========================
* Author: DJ0ABR
*
*   (c) DJ0ABR
*   www.dj0abr.de

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*
* Repeatable Timer for various needs
* ==================================
*
*/

#include "kmtimer.h"

/*
usage:

// this function is called every time_in_ms milliseconds
void timerhandler_function(void)
{
	// called every time_in_ms
}

// start a new timer
int timernum = start_timer(time_in_ms, &timerhandler_function);

stop_timer(timernum);

*/


typedef struct _TIMERDEF_ {
	int to_ms;								// timeout value in ms
	timer_t timerid;
	void(*timer_func_handler_pntr)(void);	// this user supplied function is called on timeout
    struct itimerspec in, out;
} TIMERDEF;

#define MAXTIMERS 20
TIMERDEF timers[MAXTIMERS];
int timernum = 0;

// callback function. The timer number is in val.sival_int
void sighler (union sigval val)
{
	int tnum = val.sival_int;
	timers[tnum].timer_func_handler_pntr();
}

int start_timer(int mSec, void(*timer_func_handler)(void))
{
	timers[timernum].timer_func_handler_pntr = timer_func_handler;
	timers[timernum].to_ms = mSec;

	pthread_attr_t attr;
    pthread_attr_init( &attr );

    struct sched_param parm;
    parm.sched_priority = 255;
    pthread_attr_setschedparam(&attr, &parm);

	struct sigevent sig;
    sig.sigev_notify = SIGEV_THREAD;
    sig.sigev_notify_function = sighler;
    sig.sigev_value.sival_int = timernum;
    sig.sigev_notify_attributes = &attr;

    
    int ret = timer_create(CLOCK_REALTIME, &sig, &(timers[timernum].timerid));
    if (ret == 0)
    {
        timers[timernum].in.it_value.tv_sec = mSec / 1000;
        timers[timernum].in.it_value.tv_nsec = (mSec % 1000) * 1000000;
        timers[timernum].in.it_interval.tv_sec = mSec / 1000;
        timers[timernum].in.it_interval.tv_nsec = (mSec % 1000) * 1000000;

        ret = timer_settime(timers[timernum].timerid, 0, &timers[timernum].in, &timers[timernum].out);
        if(ret == -1)
		{
            printf("timer_settime() failed with %d\n", errno);
			return -1;
		}
    }
    else
	{
    	printf("timer_create() failed with %d\n", errno);
		return -1;
	}

	int r = timernum;
	timernum++;

	return r;
}


void stop_timer(int timer)
{
	timer_delete(timers[timer].timerid);
}

// ============= MISC timing functions ==============================

void sleep_ms(int ms)
{
    for(int i=0; i<1000; i++)
        usleep(ms);
}

char *get_utctime()
{
    static char tim[20];
    struct tm *gtime;
    time_t now;
    time(&now);
    gtime = gmtime(&now);

    sprintf(tim,"%02d%02d%02d",gtime->tm_hour % 24, gtime->tm_min, gtime->tm_sec);
    //printf ("UTC-time: %s\n", tim);
    return tim;
}

char *get_utcdate()
{
    static char dat[20];
    struct tm *gtime;
    time_t now;
    time(&now);
    gtime = gmtime(&now);

    sprintf(dat,"%02d%02d%02d",gtime->tm_mday, gtime->tm_mon,gtime->tm_year+1900-2000);

    //printf ("UTC: %s\n%s\n", tim,dat);
    return dat;
}

void measure_samplerate(int id, int samp, int prt)
{
static unsigned long lastus[10];
static unsigned long samples[10];
static int f[10];
static int cnt[10];
static int first=1;

    if(first)
    {
        first = 0;
        for(int i=0; i<10; i++)
        {
            f[i] = 1;
            cnt[i] = 0;
        }
    }

    // measure sample rate
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    if(f[id])
    {
        f[id] = 0;
        lastus[id] = tv.tv_sec * 1000000 + tv.tv_usec;
        samples[id] = samp;
    }
    samples[id] += samp;
    unsigned long tnow = tv.tv_sec * 1000000 + tv.tv_usec;
    
    if(++(cnt[id]) >= prt)
    {
        cnt[id] = 0;
        printf("%d: %.3f\n",id,((samples[id]*1e6)/(tnow - lastus[id]))/1e6);
    }
}

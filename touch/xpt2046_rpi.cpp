/*
* Raspberry PI / Zero AddOn Board specially for Ham Radio Applications
* ====================================================================
* Author: DJ0ABR
*
*   (c) DJ0ABR
*   www.dj0abr.de
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
* 

reads the XPT2046 touch screen

usage:
------
1) initialize the touch thread: touch_init()
2) read scanned positions: touch_read(..)

some code bases on this work:
https://github.com/mwilliams03/Pi-Touchscreen-basic
*
*/

#include "../kmclib.h"
#include "xpt2046_rpi.h"
#include "../kmlib/km_helper.h"
#include "../kmlib/kmfifo.h"

void getTouchScreenDetails(int *screenXmin,int *screenXmax,int *screenYmin,int *screenYmax);
void getTouchSample(int *rawX, int *rawY, int *rawPressure);

int touch_active = 0;
int tfd;

pthread_mutex_t     touch_crit_sec;
#define LOCK	pthread_mutex_lock(&touch_crit_sec)
#define UNLOCK	pthread_mutex_unlock(&touch_crit_sec)

pthread_t touch_tid;
void *touchproc(void *pdata);

int display_w = 480;
int display_h = 320;

int touch_fifoid = -1;

void touch_init()
{
    if ((tfd = open("/dev/input/event0", O_RDONLY)) < 0) 
    {
        printf("cannot open touch screen\n");
        return;
    }

    int pret = pthread_create(&touch_tid,NULL,touchproc, NULL);
    if(pret)
    {
        printf("touch process NOT started\n");
        return;
    }
    usleep(100000);

    touch_active = 1;
}

int touch_read(TOUCHINFO *ptp)
{
    if(touch_active == 0) return -1;
    if(touch_fifoid == -1) return -1;

    int len = read_fifo(touch_fifoid,(uint8_t *)ptp,sizeof(TOUCHINFO));
    if(len == 0)
        return -1;

    return 0;
}

void *touchproc(void *pdata)
{
int screenXmax, screenXmin, screenYmax, screenYmin;
int rawX;
int rawY;
int rawPressure;
TOUCHINFO tinf;

    pthread_detach(pthread_self());

    printf("start touch thread\n");

    getTouchScreenDetails(&screenXmin,&screenXmax,&screenYmin,&screenYmax);
    //printf("touch range: %d %d %d %d\n",screenXmin,screenXmax,screenYmin,screenYmax);
    float scaleXvalue = ((float)screenXmax-screenXmin) / display_w;
    float scaleYvalue =  (((float)screenYmax-screenYmin) / display_h);

    touch_fifoid = create_fifo(50, sizeof(TOUCHINFO));

    fcntl(tfd, F_SETFL, O_NONBLOCK);
    while(keeprunning)
    {
        getTouchSample(&rawX, &rawY, &rawPressure);
        tinf.scaledX = rawY/scaleXvalue;
        tinf.scaledY = display_h - rawX/scaleYvalue;
        //printf("% 4d % 4d\n",tinf.scaledX,tinf.scaledY);
        write_fifo(touch_fifoid,(uint8_t *)(&tinf),sizeof(TOUCHINFO));
        usleep(1000);
    }

  
    printf("exit touch thread\n");
    pthread_exit(NULL);
}

// ========== touch handler ===============

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)

void getTouchScreenDetails(int *screenXmin,int *screenXmax,int *screenYmin,int *screenYmax)
{
    unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
    char name[256] = "Unknown";
    int abs[6] = {0};

    ioctl(tfd, EVIOCGNAME(sizeof(name)), name);

    memset(bit, 0, sizeof(bit));
    ioctl(tfd, EVIOCGBIT(0, EV_MAX), bit[0]);

    for (int i = 0; i < EV_MAX; i++)
    {
        if (test_bit(i, bit[0])) 
        {
            if (!i) continue;
            ioctl(tfd, EVIOCGBIT(i, KEY_MAX), bit[i]);
            for (int j = 0; j < KEY_MAX; j++)
            {
                if (test_bit(j, bit[i]) && i == EV_ABS) 
                {
                    ioctl(tfd, EVIOCGABS(j), abs);
                    for (int k = 0; k < 5; k++)
                    {
                        if ((k < 3) || abs[k])
                        {
                            if (j == 0)
                            {
                                if (k == 1) *screenXmin =  abs[k];
                                if (k == 2) *screenXmax =  abs[k];
                            }
                            if (j == 1)
                            {
                                if (k == 1) *screenYmin =  abs[k];
                                if (k == 2) *screenYmax =  abs[k];
                            }
                        }
                    }
                }
            }
        }
    }
}

// returns the last position if no new value within 100ms
void getTouchSample(int *rawX, int *rawY, int *rawPressure)
{
    ssize_t rb;
    struct input_event ev[64];

    int touch_to = -1;
    while(keeprunning)
    {
        rb = read(tfd, ev, sizeof(struct input_event)*64);
        if(rb != -1)
        {
            for (size_t i = 0;  i <  (rb / sizeof(struct input_event)); i++)
            {
                
                if (ev[i].type == EV_ABS && ev[i].value > 0)
                {
                    
                    switch (ev[i].code)
                    {
                        case 0: *rawX = ev[i].value; break;
                        case 1: *rawY = ev[i].value; break;
                        case 24: *rawPressure = ev[i].value; break;
                    }
                    touch_to = 0;
                }
            }
        }
        if(touch_to >= 0) touch_to++;
        if(touch_to >= 100) break;
        usleep(1000);
    }
}

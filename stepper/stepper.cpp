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
stepper.cpp
===========
handling stepper motors

Usage:
======

1. at program start: create a process for a steper motor: create_stepper(..)
2. move stepper relative to current position: move_stepper(..)
3. move stepper to absolute position: gopos_stepper(..)
4. move to a reference switch (if available): ref_stepper(..)
5. move until user supplied condition is met: move_cond_stepper(..)
   ATTENTION: the user supplied callback function MUST be thread safe !
6. read the current position: getpos_stepper(..)
7. disable all stepper PAs: disable_stepper()

*/

#include "../kmclib.h"
#include "../i2c_rpi/i2c_rpi.h"
#include "../i2c_rpi/mcp23017.h"
#include "../i2c_rpi/max11615.h"
#include "../i2c_rpi/gpio.h"
#include "stepper.h"
#include "../kmlib/km_helper.h"

pthread_mutex_t     stp_crit_sec;
#define LOCK	pthread_mutex_lock(&stp_crit_sec)
#define UNLOCK	pthread_mutex_unlock(&stp_crit_sec)

void *stepperproc(void *pdata);
void wait_stepjob(int ID, int wait);

typedef struct _STEPPER_ {
    int ID;                 // stepper handler ID, beginning with 0, then incrementing by 1
    int step_port;          // GPIO used for step signal (see gpio.h)
    int dir_port;           // GPIO used for direction signal (see gpio.h)
    int enable_port;        // GPIO used for enable signal (see gpio.h) (-1 if not used)
    int polarity;           // polarity of step, dir, enable ports: 0=source 1=sink
    int reference_port;     // GPIO used for reference switch (see gpio.h) (-1 if not used)
    int reference_polarity; // polarity of reference switch
    int speed;              // speed per step in 100us
    int steps;              // number of steps in mode 0
    int direction;          // direction of the current job
    int actpos;             // actual position (in steps), set to 0 in mode 1
    int mode;               // -1=idle, 0=move requested steps, 1=move to reference, 2=move until condition
    int (*steppercondition_callback)(int,int); // user supplied callback function for stop condition in mode 2
    pthread_t stepper_tid;  // thread reference
} STEPPER;

#define STEPPERNUM  5   // max. number of supported stepper motors (increase if needed)
STEPPER stepper[STEPPERNUM];
int steppernum = 0;     // actual number of stepper motors in use

/*
create a new process handling a stepper motor
sp ... step port (see gpio.h for valid values)
dp ... direction port
en ... enable port (-1 if not used)
pol .. polarity (out is sink=1 or source=0)
spd ... speed (pulse frequency in 50us steps)
refp .. reference input port (see gpio.h for valid values) or -1 if not used
refdir ... polarity of signal on ref input port  or -1 if not used

returns: -1=error, other values are the reference ID of the created stepper driver
*/
int create_stepper(int sp, int dp, int ep, int pol, int spd, int refp, int refdir)
{
    if(steppernum >= STEPPERNUM)
    {
        printf("too many steps, increase STEPPERNUM\n");
        return -1;
    }
    stepper[steppernum].ID = steppernum;
    stepper[steppernum].step_port = sp;
    stepper[steppernum].dir_port = dp;
    stepper[steppernum].enable_port = ep;
    stepper[steppernum].reference_port = refp;
    stepper[steppernum].reference_polarity = refdir;
    stepper[steppernum].polarity = pol;
    stepper[steppernum].speed = spd;
    stepper[steppernum].steps = 0;
    stepper[steppernum].direction = 0;
    stepper[steppernum].actpos= 0;
    stepper[steppernum].mode = -1;

    int pret = pthread_create(&stepper[steppernum].stepper_tid,NULL,stepperproc, &(stepper[steppernum]));
    if(pret)
    {
        printf("stepper process %d NOT started\n",steppernum);
        return -1;
    }
    usleep(100000);

    int ret = steppernum;

    steppernum++;
    return ret;
}

/*
move stepper by "steps" into direction "dir"
ID ... stepper id returned by create_stepper
steps ... number of steps to move
dir ... direction 0 or 1
wait ... 0=return immediately 1=wait until position is reached
*/
void move_stepper(int ID, int steps, int dir, int wait)
{
    //printf("%d steps into dir %d\n",steps,dir);
    LOCK;
    stepper[ID].direction = dir;
    stepper[ID].steps =steps;
    stepper[ID].mode = 0;
    UNLOCK;

    wait_stepjob(ID, wait);
}

/*
move stepper into direction "dir" until condition comes TRUE
ID ... stepper id returned by create_stepper
steps ... number of steps to move (or -1 to move infinite)
dir ... direction 0 or 1
wait ... 0=return immediately 1=wait until position is reached
stepcond ... user supplied callback function which must return 1 to stop movement
*/
void move_cond_stepper(int ID, int steps, int dir , int wait, int (*stepcond)(int step, int dir))
{
    //printf("steps to condition\n");
    LOCK;
    stepper[ID].steppercondition_callback = stepcond;
    stepper[ID].direction = dir;
    stepper[ID].steps =steps;
    stepper[ID].mode = 2;
    UNLOCK;

    wait_stepjob(ID, wait);
}

void wait_stepjob(int ID, int wait)
{
    if(wait == 0) return;

    while(1)
    {
        LOCK;
        int mode = stepper[ID].mode;
        UNLOCK;
        if(mode == -1) break;
        usleep(1000);
    }
}

/*
step to specified position, makes only sense when a reference switch is used
ID ... stepper id returned by create_stepper
pos ... step-position to go to
wait ... 0=return immediately 1=wait until position is reached
*/
void gopos_stepper(int ID, int pos, int wait)
{
    LOCK;
    int actpos = stepper[ID].actpos;
    UNLOCK;
    if(pos == actpos) return;

    int dir = 0;
    int steps = actpos - pos;
    if(pos > actpos) 
    {
        dir = 1;
        steps = pos - actpos;
    }

    move_stepper(ID, steps, dir, wait);
}

/*
move to reference switch position
ID ... stepper id returned by create_stepper
waits until refpos is reached
*/
void ref_stepper(int ID)
{
    if(stepper[ID].reference_port != -1 && stepper[ID].reference_polarity != -1)
    {
        printf("%d move to reference\n",ID);
        stepper[ID].mode = 1;
    }
    
    wait_stepjob(ID, 1);
}

/*
read current step position
*/
int getpos_stepper(int ID)
{
    int pos = 0;
    LOCK;
    pos = stepper[ID].actpos;
    UNLOCK;

    return pos;
}

void disable_stepper()
{
    LOCK;
    for(int i=0; i<steppernum; i++)
        stepper[i].mode = 3;
    UNLOCK;
}

void *stepperproc(void *pdata)
{
    pthread_detach(pthread_self());
    
    STEPPER *pst = (STEPPER *)pdata;

    LOCK;
    int step_port = pst->step_port;
    int dir_port = pst->dir_port;
    int enable_port = pst->enable_port;
    int polarity = pst->polarity;
    int refport = pst->reference_port;
    int refpol = pst->reference_polarity;
    int speed = pst->speed;
    UNLOCK;

    printf("stepper process started\n");
    
    while(keeprunning)
    {
        LOCK;
        int mode = pst->mode;
        UNLOCK;
        if(mode == 0)
        {
            LOCK;
            int steps = pst->steps;
            int dir = pst->direction;
            speed = pst->speed;
            UNLOCK;
            // normale stepping: move requested steps
            if(steps > 0)
            {
                // move for pst->steps steps into pst->direction
                // set direction
                if(dir) dir = 1;
                if(polarity) dir = 1-dir;
                setPort(dir_port,dir);

                // enable
                if(enable_port != -1) setPort(enable_port,polarity);

                // calc speed
                int delay = (speed * 100) / 2;   // pst->speed is in 100us steps

                // move the steps
                while(steps--)
                {
                    LOCK;
                    int md = pst->mode;
                    int ap = pst->actpos;
                    UNLOCK;

                    if(md == 1) break;
                    if(ap <= 0 && dir == 0) break;

                    if(refport != -1)
                    {
                        // check ref switch
                        int p = getPort(refport);
                        if(p == refpol && dir == 0) break;
                    }

                    setPort(step_port,1-polarity);
                    usleep(delay);
                    setPort(step_port,polarity);
                    usleep(delay);

                    LOCK;
                    pst->actpos += (dir?1:-1);
                    UNLOCK;
                    printf("actpos: %d\n",pst->actpos);
                }

                pst->mode = -1;
            }
        }

        if(mode == 1)
        {
            // move to reference switch, then return to mode=0
            printf("ID:%d move to reference\n",pst->ID);

            // enable
            if(enable_port != -1) setPort(enable_port,polarity);

            // calc speed
            int delay = (speed * 100) / 2;   // pst->speed is in 100us steps

            // check if already on reference switch
            int p = getPort(refport);
            if(p == refpol) 
            {
                printf("on ref ID:%d move away\n",pst->ID);
                // is already on ref switch, move away
                int dir = 1;
                if(polarity) dir = 1;
                setPort(dir_port,dir);

                while(1)
                {
                    setPort(step_port,1-polarity);
                    usleep(delay);
                    setPort(step_port,polarity);
                    usleep(delay);

                    if(getPort(refport) != refpol) break;
                }       

                // move some additional steps     
                for(int st=0; st<100; st++)
                {
                    setPort(step_port,1-polarity);
                    usleep(delay);
                    setPort(step_port,polarity);
                    usleep(delay);
                }
            }

            // set direction
            int dir = 0;
            if(polarity) dir = 1;
            setPort(dir_port,dir);

            while(1) 
            {
                int p = getPort(refport);
                if(p == refpol) break;

                setPort(step_port,1-polarity);
                usleep(delay);
                setPort(step_port,polarity);
                usleep(delay);
            }
            LOCK;
            pst->actpos = 0;
            pst->mode = -1;
            UNLOCK;

            printf("ID:%d reference found\n",pst->ID);
        }

        if(mode == 2)
        {
            LOCK;
            int steps = pst->steps;
            int dir = pst->direction;
            speed = pst->speed;
            UNLOCK;            

            // step until condition comes TRUE or no of steps reached
            // set direction
            if(dir) dir = 1;
            if(polarity) dir = 1-dir;
            setPort(dir_port,dir);

            // enable
            if(enable_port != -1) setPort(enable_port,polarity);

            // calc speed
            int delay = (speed * 100) / 2;   // pst->speed is in 100us steps

            while(1)
            {
                LOCK;
                int md = pst->mode;
                int ap = pst->actpos;
                UNLOCK;                

                // stop if:
                // 1. mode changes
                if(md != 2) break;

                // 2. all steps done
                if(steps != -1)
                {
                    if(steps-- == 0) 
                    {
                        break;
                    }
                }

                // 3.ref position met
                if(ap <= 0 && dir == 0) break;

                // 4. ref switch met (if ref sw is in use)
                if(refport != -1)
                {
                    
                    int p = getPort(refport);
                    if(p == refpol && dir == 0) break;
                }
                // 4. if user supplied condition is met
                if(pst->steppercondition_callback(pst->actpos, dir)) break;

                setPort(step_port,1-polarity);
                usleep(delay);
                setPort(step_port,polarity);
                usleep(delay);

                //printf("actpos: %d\n",pst->actpos);

                LOCK;
                pst->actpos += (dir?1:-1);
                UNLOCK;
            }

            LOCK;
            pst->mode = -1;
            UNLOCK;
        }

        if(mode == 3)
        {
            if(enable_port != -1) setPort(enable_port,1-polarity);
            LOCK;
            pst->mode = -1;
            UNLOCK;
        }

        if(mode == -1)
        {
            usleep(10000);
        }
    }

    if(enable_port != -1) 
    {
        printf("disable stepper: %d\n",pst->ID);
        setPort(enable_port,1-polarity);
    }
    printf("exit stepper thread\n");
    pthread_exit(NULL);
}

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
motor.cpp
===========
handling motor drivers

Usage:
======
1) for each motor: init_motor(..): init motor driver and assign GPIOs
   ATTENTION: the user supplied callback function MUST be thread safe !

turnleft_motor(int ID) ... enable left turning
turnright_motor(int ID) .. enable right turning
stop_motor(int ID) ....... stop immediately

Module port requiring a GPIO:
=============================
GND/Vcc ... connect to 3,3v supply
LEN+REN ... connect together to one GPIO (HIGH=enable, LOW=disable)
LPWM ... turn left if High GPIO
RPWM ... turn right if High GPIO

*/

#include "../ctlbrd.h"

pthread_mutex_t     motor_crit_sec;
#define LOCK	pthread_mutex_lock(&motor_crit_sec)
#define UNLOCK	pthread_mutex_unlock(&motor_crit_sec)

void *motorproc(void *pdata);
void wait_motorjob(int ID, int wait);

typedef struct _MOTOR_ {
    int ID;                 // motor handler ID, beginning with 0, then incrementing by 1
    int enable_port;        // GPIO used for enable signal (see gpio.h) (on LEN+REN)
    int turn_left_port;     // GPIO used to turn motor left (see gpio.h)
    int turn_right_port;    // GPIO used to turn motor left (see gpio.h)
    int speed;              // PWM speed in % 1-99
    int maxruntime;         // max seconds to turn motor
    int mode;               // -1=idle, 0=move left until condition, 1=move right until condition
    int (*motorcondition_callback)(); // user supplied callback function for stop condition in mode 0 and 1
    pthread_t motor_tid;    // thread reference
} MOTOR;

#define MOTORNUM  5   // max. number of supported motors (increase if needed)
MOTOR motor[MOTORNUM];
int motornum = 0;     // actual number of motors in use

/*
create a new process handling a linear motor
en ... enable port
lft ... draw left port
rgt ... draw right port
maxtm ... shut off motor after maxtm seconds
stopcond ... user supplied stop funktion, must return 1 to stop movement

returns: -1=error, other values are the reference ID of the created motor driver
*/
int create_motor(int en, int lft, int rgt, int spd, int maxtm, int (*stopcond)())
{
    if(motornum >= MOTORNUM)
    {
        printf("too many motors, increase MOTORNUM\n");
        return -1;
    }

    motor[motornum].ID = motornum;
    motor[motornum].enable_port = en;
    motor[motornum].turn_left_port = lft;
    motor[motornum].turn_right_port = rgt;
    motor[motornum].speed = spd;
    motor[motornum].maxruntime = maxtm;
    motor[motornum].mode = -1;
    motor[motornum].motorcondition_callback = stopcond;

    int pret = pthread_create(&motor[motornum].motor_tid,NULL,motorproc, &(motor[motornum]));
    if(pret)
    {
        printf("motor process %d NOT started\n",motornum);
        return -1;
    }
    usleep(100000);

    int ret = motornum;

    motornum++;
    return ret;
}

void wait_motorjob(int ID)
{
    while(1)
    {
        LOCK;
        int mode = motor[ID].mode;
        UNLOCK;
        if(mode == -1) break;
        usleep(1000);
    }
}

void turnleft_motor(int ID)
{
    printf("turn left\n");
    usleep(100000);
    LOCK;
    motor[ID].mode = 0;
    UNLOCK;
    wait_motorjob(ID);
    printf("turn left FIN\n");
}

void turnright_motor(int ID)
{
    printf("turn right\n");
    usleep(100000);
    LOCK;
    motor[ID].mode = 1;
    UNLOCK;
    wait_motorjob(ID);
    printf("turn right FIN\n");
}

void stop_motor(int ID)
{
    LOCK;
    motor[ID].mode = -1;
    UNLOCK;
}

void *motorproc(void *pdata)
{
    pthread_detach(pthread_self());
    
    MOTOR *pmot = (MOTOR *)pdata;

    LOCK;
    int ID = pmot->ID;
    int enport = pmot->enable_port;
    int leftport = pmot->turn_left_port;
    int rightport = pmot->turn_right_port;
    UNLOCK;

    printf("motor process started for motor: %d\n",ID);
    
    while(keeprunning)
    {
        LOCK;
        int mode = pmot->mode;
        int to = pmot->maxruntime;
        int speed = pmot->speed;
        UNLOCK;

        if(mode == 0 || mode == 1)
        {
            // make sure motors are not driven
            setPort(leftport,0);
            setPort(rightport,0);
            usleep(1000);

            // set direction
            if(mode == 0)
                setPort(leftport,1);
            else
                setPort(rightport,1);

            int ontime = speed * 100;
            int offtime = (100-speed) * 100;
            if(ontime <= 0 || offtime <= 0)
            {
                printf("wrong speed value\n");
            }
            else 
            {
                // to is the max. runtime in seconds
                // one loop delay is ontime+offtime
                int actto = (to * 1000000) / (ontime + offtime);

                //printf("turn motor on:%d us, off:%d us, maxrun:%d\n",ontime,offtime,actto);
                while(pmot->motorcondition_callback() == 0) 
                {
                    // enable motor driver, motor starts turning
                    setPort(enport,0);
                    // let motor ON for speed*100 us (max 10 ms)
                    usleep(ontime);

                    // motor off
                    setPort(enport,1);  
                    // let motor OFF for speed*100 us (max 10 ms)
                    usleep(offtime);

                    if(--actto <= 0) break;    // security timeout
                    LOCK;
                    int m = pmot->mode;
                    UNLOCK;
                    if(m == -1) break;
                }
            }

            // disable motor driver
            setPort(enport,1);
            setPort(leftport,0);
            setPort(rightport,0);

            LOCK;
            pmot->mode = -1;
            UNLOCK;
        }

        if(mode == -1)
        {
            usleep(10000);
        }
    }

    printf("exit motor thread\n");
    pthread_exit(NULL);
}

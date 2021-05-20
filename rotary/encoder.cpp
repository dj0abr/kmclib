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
encoder.cpp
===========

function to handle rotary encoder

Usage:
======
initialize and assign ports to the encoders
-------------------------------------------
up to 3 encoders are supported, If not used, then assign port nunmber -1
init_rotencoder(int portA1, int portB1, int portA2, int portB2, int portA3, int portB3)
see gpio.h _INPUTS_ for valid port numbers

read encoder:
-------------
int getEncSteps(int idx)
idx ... encoder number 0,1 or 2
returns the number of steps since last read

encoder speed:
the maximum detectable encoder speed is about 1ms per pulse or a little less.
This depends on the I2C bus load by other devices

*
*/

#include "../kmclib.h"
#include "../i2c_rpi/i2c_rpi.h"
#include "../i2c_rpi/mcp23017.h"
#include "../i2c_rpi/max11615.h"
#include "../i2c_rpi/gpio.h"
#include "encoder.h"
#include "../kmlib/km_helper.h"

#define NUM_OF_ENCODERS 3

pthread_mutex_t     rotenc_crit_sec;
#define LOCK	    pthread_mutex_lock(&(rotenc_crit_sec))
#define UNLOCK	    pthread_mutex_unlock(&(rotenc_crit_sec))

void *rotencproc(void *);    

int portA[NUM_OF_ENCODERS];
int portB[NUM_OF_ENCODERS];

int encsteps[NUM_OF_ENCODERS];

pthread_t rotenc_tid;

// port numbers see gpio.h _INPORTS_
// -1 if not used
void init_rotencoder(int portA1, int portB1, int portA2, int portB2, int portA3, int portB3)
{
    portA[0] = portA1;
    portA[1] = portA2;
    portA[2] = portA3;
    portB[0] = portB1;
    portB[1] = portB2;
    portB[2] = portB3;

    memset(&encsteps, 0, sizeof(int)*NUM_OF_ENCODERS);

    int pret = pthread_create(&rotenc_tid,NULL,rotencproc, NULL);
    if(pret)
    {
        printf("rotenc process NOT started\n");
        exit(0);
    }
}

int getEncSteps(int idx)
{
    int v;
    LOCK;
    v = encsteps[idx];
    encsteps[idx] = 0;
    UNLOCK;
    return v;
}

void *rotencproc(void *pdata)
{
    pthread_detach(pthread_self());
    
    printf("rotenc process started\n");

    int aval[NUM_OF_ENCODERS];
    int bval[NUM_OF_ENCODERS];
    int oldaval[NUM_OF_ENCODERS];
    while(keeprunning)
    {
        for(int i=0; i<NUM_OF_ENCODERS; i++)
        {
            if(portA[i] == -1 || portB[i] == -1) continue;
            aval[i] = getPort(portA[i]);
            bval[i] = getPort(portB[i]);

            if(aval[i]==1 && oldaval[i]==0)
            {
                // falling edge of port A
                int dir = -1;
                if(bval[i] == 1) dir = 1;
                LOCK;
                encsteps[i] += dir;
                UNLOCK;
            }
            oldaval[i] = aval[i];
        }

        usleep(100);
    }

    printf("exit rotenc thread\n");
    pthread_exit(NULL);
}

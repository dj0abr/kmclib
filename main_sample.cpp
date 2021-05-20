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
main.cpp
===========

this is an exaple of the main file calling all other functions
do all initialisation here

*
*/

#include "ctlbrd.h"

void exit_program();

int main(int argc, char *argv[])
{
    printf("Raspi Ham Radio Control Board starting ...\n");

    if(isRunning("ctlbrd") == 1)
        exit(0);    // do not allow to runs this program twice

    install_signal_handler(exit_program); // mainly used to catch Ctrl-C

    // open display (if available)
    //display_init();

    // these initialisations are required if the library is used on the hamcontrol PCB
    // open i2c bus, which also activates the in/outputs
    //if(i2c_init() == -1) exit_program();
    //init_adc();

    //app_setup();

    while(keeprunning)
    {
        //app_loop();
        usleep(100);    // make as long as possible for the application to reduce CPU load
    }
    
    exit_program();
    return 0;
}

void exit_program()
{
    printf("close program\n");
    // do cleanup
    //keeprunning = 0;
    //sleep(1);   // give threads a chance to exit
    //destroy_fifos();
    printf("exit program\n");
    exit(0);
}

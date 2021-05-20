/*
* Autmatic Antenna Tuner
* ======================
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

Application: Automatic Antenna Tuner

*/

#include "../ctlbrd.h"

int stepid;

void autotuner_setup()
{
    setup_touchbuttons();
    setup_bargraphs();
    act_menunum = 0;

    // weise dem Rotary-Encoder zwei Eingangsports zu
    init_rotencoder(KEY0, KEY1, -1,-1,-1,-1);
    
    // weise der Schrittmotor-PA Ports für Step und Richtung zu
    stepid = create_stepper(OUT0, OUT1, OUT2, 0, 10, -1,-1);
    ref_stepper(stepid);
}

int microsteps = 4;

int stepcallback(int step, int dir)
{
    printf("Position step: %d\n",step);
    if(step >= (12300*microsteps) && dir == 1) return 1;
    return 0;
}

void autotuner_loop()
{
    static int lastmenu = -1;

    if(act_menunum != lastmenu)
    {
        show_menu(act_menunum);
        lastmenu = act_menunum;
    }

    show_values(lastmenu,stepid);

    int steps = getEncSteps(0);     // Lese Encoder
                                    // falls der Encoder betätigt wurde
                                    // gebe die Schrittzahl an die Schrittmotor PA aus
    if(steps != 0)
    {
        int rstps = steps;
        rstps = rstps * rstps * rstps;
        printf("Step: %d -> %d\n",steps,rstps);
        move_cond_stepper(stepid,abs(rstps),(steps>0)?1:0,0,stepcallback);
    }

    usleep(100000);
}


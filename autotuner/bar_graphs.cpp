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

setup and handle all bar graphs

*
*/

#include "../ctlbrd.h"

void stepts_uH(char *s, int maxlen, double val);

extern int microsteps;

void setup_bargraphs()
{
    t_bargraph tb;

    tb.minx = 10;
    tb.width = 350;
    tb.miny = 90;
    tb.height = 30;
    tb.titlefont = VERDANA_BOLD;
    strcpy(tb.titletext,"KONDENSATOR");
    tb.titlex = 10;
    tb.titley = -36;
    tb.titlesize = 20;
    strcpy(tb.labeltext,"   C [pF]");
	tb.labelfont = VERDANA;
    tb.labelsize = 15;
    tb.labelypos = 16;
    strcpy(tb.barlabel[0].text,"160");
    tb.barlabel[0].posx = 70;
    strcpy(tb.barlabel[1].text,"320");
    tb.barlabel[1].posx = 140;
    strcpy(tb.barlabel[2].text,"400");
    tb.barlabel[2].posx = 210;
    strcpy(tb.barlabel[3].text,"560");
    tb.barlabel[3].posx = 280;
    strcpy(tb.barlabel[4].text,"800");
    tb.barlabel[4].posx = 350-20;
    tb.labelanz = 5;
    tb.showvalue = 1;
	tb.valuefont = COURIER_BOLD;
    tb.valuefontsize = 20;
	tb.valuex = tb.width+10;
	tb.valuey = -18;
    tb.barvaluefunc = NULL;
    tb.bgcolor = BLACK;
    tb.bgx = tb.valuex;
    tb.bgw = 100;
    tb.bgy = 5;
    tb.bgh = 20;
    tb.minval = 0;
    tb.maxval = 1000;

    create_bargraph(&tb); 

    tb.miny += tb.height * 2 + 20;
    strcpy(tb.titletext,"ROLLSPULE");
    strcpy(tb.labeltext,"L [uH]");
    strcpy(tb.barlabel[0].text,"0.1");
    tb.barlabel[0].posx = 60;
    strcpy(tb.barlabel[1].text,"1");
    tb.barlabel[1].posx = 120;
    strcpy(tb.barlabel[2].text,"10");
    tb.barlabel[2].posx = 180;
    strcpy(tb.barlabel[3].text,"50");
    tb.barlabel[3].posx = 240;
    tb.labelanz = 4;
    tb.barvaluefunc = stepts_uH;
    tb.minval = 0;
    tb.maxval = 12400*microsteps;

    create_bargraph(&tb);
}

// bargraph callback: calculate uH from steps
// and make string to be printed as value

typedef struct _nH_ {
    int steps;
    double nH;
} nH;

nH spulenvals[] = {
{	0	    ,	0.14	},
{	1000	,	0.3	    },
{	2000	,	0.55	},
{	3000	,	0.74	},
{	4000	,	1	    },
{	5000	,	1.33	},
{	6000	,	1.7	    },
{	7000	,	2.1	    },
{	8000	,	2.5	    },
{	10000	,	3.5	    },
{	15000	,	6.2	    },
{	22000	,	10.6	},
{	30000	,	16.2	},
{	40000	,	23.4	},
{	49000	,	30.3	},
{-1,-1}
};


void stepts_uH(char *s, int maxlen, double val)
{
    double L = 31;  // max value

    // seek range
    int i=0;
    while(1)
    {
        if((int)val < spulenvals[i+1].steps) break;
        i++;
        if(spulenvals[i+1].steps == -1) break;
    }

    if(spulenvals[i+1].steps != -1)
    {
        // range found, between i and i+1
        double si = spulenvals[i].steps;
        double se = spulenvals[i+1].steps;
        double li = spulenvals[i].nH;
        double le = spulenvals[i+1].nH;
        L = li + (val-si)*(le-li)/(se-si);
    }
    sprintf(s,"%.2f uH",L);
}

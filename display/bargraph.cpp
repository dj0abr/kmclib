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
bargraph.cpp
===========
handles bar graphs to display various values

usage:
------
* create a bargraph with various options: create_bargraph(..)
* set a bargraph by putting a value: putvalue_bargraph(..)

*
*/

#include "../kmclib.h"

t_bargraph bargraph[MAXBARGRAPHS];
int bargraphanz = 0;

// internal values
typedef struct _BGINTERN_ {
    int lastval;
    int ytop;
    int ybottom;
} BGINTERN;

BGINTERN bgint[MAXBARGRAPHS];

// returns ID of the created bargraph
// use this ID for setting a value
int create_bargraph(t_bargraph *settings)
{
    if(bargraphanz >= MAXBARGRAPHS) return -1;

    memcpy(&(bargraph[bargraphanz]), settings, sizeof(t_bargraph));

    int id = bargraphanz;
    bargraphanz++;

    putvalue_bargraph(id, 0, 2);

    return id;
}

// drawmode: 0=draw everything, 1=draw value only, 2=just set values, no drawing
void putvalue_bargraph(int id, double value, int drawmode)
{
    t_bargraph *pb = &(bargraph[id]);

    // calc positions
    double v = value - pb->minval;
    v /= pb->maxval;
    v *= (double)pb->width - 0.5;
    int xs = pb->minx+1;
    int xe = xs + (int)v;

    if(drawmode == 0)
    {
        // clear screen under bar
        TFT_Rectangle_filled(pb->minx, pb->miny, pb->minx+pb->width, pb->miny+pb->height, BLACK);

        // draw white margin
        TFT_Rectangle(pb->minx, pb->miny, pb->minx+pb->width, pb->miny+pb->height, RED, 1);

        // draw title
        draw_font(pb->titletext, pb->minx + pb->titlex, pb->miny + pb->titley, pb->titlefont, WHITE, pb->titlesize);

        // X-Axis Title
        draw_font(pb->labeltext, pb->minx + pb->width, pb->miny + pb->labelypos, pb->labelfont, WHITE, pb->labelsize);

        // X-Labels
        for(int i=0; i<pb->labelanz; i++)
            draw_font(pb->barlabel[i].text, pb->minx + pb->barlabel[i].posx, pb->miny + pb->labelypos, pb->labelfont, WHITE, pb->labelsize);

        bgint[id].ytop = pb->miny+1;
        bgint[id].ybottom = pb->miny+pb->height-1;
        bgint[id].lastval = xe;
    }

    if(drawmode == 0 || drawmode == 1)
    {
        if(xe != bgint[id].lastval)
        {
            // adjust bargraph
            if(xe > bgint[id].lastval)
                TFT_Rectangle_filled(bgint[id].lastval, bgint[id].ytop, xe, bgint[id].ybottom, YELLOW);
            else
                TFT_Rectangle_filled(xe, bgint[id].ytop, bgint[id].lastval, bgint[id].ybottom, BLACK);
        }

        if(pb->showvalue)
        {
            // clear background
            TFT_Rectangle_filled(pb->minx+pb->bgx,pb->miny+pb->bgy,pb->minx+pb->bgx+pb->bgw,pb->miny+pb->bgy+pb->bgh,pb->bgcolor);
            //printf("%d %d %d %d %d\n",pb->minx+pb->bgx,pb->miny+pb->bgy,pb->minx+pb->bgx+pb->bgw,pb->miny+pb->bgy+pb->bgh,pb->bgcolor);

            // show value as text
            char s[50];

            if(pb->barvaluefunc != NULL)
            {
                // print user supplied text
                pb->barvaluefunc(s, 50, value);
            }
            else
            {
                // print value
                // check if number has decimals or not
                if((int)(value*1000) == ((int)value)*1000)
                    sprintf(s,"%d ",(int)value);
                else
                    sprintf(s,"%.2f ",value);
            }
            draw_font(s, pb->minx + pb->valuex, pb->miny + pb->valuey, pb->valuefont, WHITE, pb->valuefontsize);
        }
    }

    bgint[id].lastval = xe;
}

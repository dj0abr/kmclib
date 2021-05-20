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

showvals.cpp ... show values of the various menus

*/

#include "../ctlbrd.h"

void show_manuell(int stepperid);
void show_calib();

int calib_mode = 0;

void show_values(int menunum, int inf)
{
    switch(menunum)
    {
        case MENU_MANUELL:
            show_manuell(inf);
            break;

        case MENU_CALIB:
            show_calib();
            break;
    }
}

void show_calib()
{
static int oldcalmode = -1;
char s[100];
int linespace = 40;
int titlespace = 18;
int draw_statics = 0;

    if(calib_mode != oldcalmode)
    {
        // clear screen
        TFT_Rectangle_filled(0,50,disp_width,228,BLACK);
        oldcalmode = calib_mode;
        draw_statics = 1;
    }

    if(calib_mode == 0)
    {
        int y = 50;
        if(draw_statics == 1)
        {
            for(int i=0; i<8; i++)
            {
                sprintf(s,"%d",i);
                draw_font(s,i*42+101,y-titlespace,COURIER_BOLD,WHITE,13);
            }
        }

        strcpy(s,"OUT:   ");
        for(int i=0; i<8; i++)
        {
            sprintf(s+strlen(s),"%1d",getOutPort(i));
            if(i < (8-1)) strcat(s,"  ");
        }
        draw_font(s,0,y,COURIER_BOLD,WHITE,24);

        y+=linespace;

        if(draw_statics == 1)
        {
            for(int i=0; i<8; i++)
            {
                sprintf(s,"%d",i+8);
                draw_font(s,i*42+101,y-titlespace,COURIER_BOLD,WHITE,13);
            }
        }

        strcpy(s,"       ");
        for(int i=8; i<16; i++)
        {
            sprintf(s+strlen(s),"%1d",getOutPort(i));
            if(i < (16-1)) strcat(s,"  ");
        }
        draw_font(s,0,y,COURIER_BOLD,WHITE,24);

        y+=linespace;

        if(draw_statics == 1)
        {
            for(int i=0; i<8; i++)
            {
                sprintf(s,"%d",i);
                draw_font(s,i*42+101,y-titlespace,COURIER_BOLD,WHITE,13);
            }
        }

        strcpy(s,"IN :   ");
        for(int i=0; i<8; i++)
        {
            sprintf(s+strlen(s),"%1d",getPort(i));
            if(i < (8-1)) strcat(s,"  ");
        }
        draw_font(s,0,y,COURIER_BOLD,WHITE,24);

        y+=linespace;

        if(draw_statics == 1)
        {
            for(int i=0; i<6; i++)
            {
                sprintf(s,"%d",i+8);
                draw_font(s,i*42+101,y-titlespace,COURIER_BOLD,WHITE,13);
            }
        }

        strcpy(s,"       ");
        for(int i=8; i<14; i++)
        {
            sprintf(s+strlen(s),"%1d",getPort(i));
            if(i < (16-1)) strcat(s,"  ");
        }
        draw_font(s,0,y,COURIER_BOLD,WHITE,24);
    }

    if(calib_mode == 1)
    {
        int y = 50;
        int linespc = 38;
        if(draw_statics == 1)
        {
            for(int i=0; i<4; i++)
            {
                sprintf(s,"ADC-%d:       V",i);
                draw_font(s,10,y+i*linespc,COURIER_BOLD,WHITE,24);

                sprintf(s,"ADC-%d:       V",i+4);
                draw_font(s,240,y+i*linespc,COURIER_BOLD,WHITE,24);
            }
        }
        int voff = 100;
        for(int i=0; i<4; i++)
        {
            sprintf(s,"%5.2f",getADCvoltage(i));
            draw_font(s,10+voff,y+i*linespc,COURIER_BOLD,WHITE,24);

            sprintf(s,"%5.2f",getADCvoltage(i+4));
            draw_font(s,240+voff,y+i*linespc,COURIER_BOLD,WHITE,24);
        }
    }   

    if(calib_mode == 2)
    {
        int y = 50;
        int linespc = 38;
        if(draw_statics == 1)
        {
            for(int i=0; i<4; i++)
            {
                switch(i)
                {
                    case 0: sprintf(s,"FWD  :"); break;
                    case 1: sprintf(s,"REV  :"); break;
                    case 2: sprintf(s,"Z    :"); break;
                    case 3: sprintf(s,"PHI  :"); break;
                }
                draw_font(s,10,y+i*linespc,COURIER_BOLD,WHITE,24);

                switch(i)
                {
                    case 0: sprintf(s,"TEMP1:"); break;
                    case 1: sprintf(s,"TEMP2:"); break;
                    case 2: sprintf(s,"CURR :"); break;
                    case 3: sprintf(s,"VOLT :"); break;
                }
                draw_font(s,240,y+i*linespc,COURIER_BOLD,WHITE,24);
            }
        }
        int voff = 100;
        for(int i=0; i<4; i++)
        {
            sprintf(s,"%5.2f",getADC(i));
            draw_font(s,10+voff,y+i*linespc,COURIER_BOLD,WHITE,24);

            sprintf(s,"%5.2f",getADC(i+4));
            draw_font(s,240+voff,y+i*linespc,COURIER_BOLD,WHITE,24);
        }
    }   
}

void show_manuell(int stepperid)
{
    putvalue_bargraph(1, getpos_stepper(stepperid), 1);
}

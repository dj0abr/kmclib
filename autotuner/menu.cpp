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

menu.cpp ... creates and displays screen menus

*/

#include "../ctlbrd.h"

void draw_statusline();
void draw_title(char *text, int x);

int act_menunum = 0;

// ONLY call from main program loop, never from threads or callbacks !
void show_menu(int menunum)
{
    deactivate_allbuttons();
    TFT_Rectangle_filled(0,0,disp_width,disp_height,BLACK);

    switch (menunum)
    {
    case MENU_MAIN:
        // Tuner Main Menu
        TFT_Rectangle_filled(0,0,disp_width,50,DARKGREY);
        draw_title("KW AUTO Tuner", 10);
        draw_button(BT_MANUELL);
        draw_button(BT_AUTO);
        draw_button(BT_SETUP);
        draw_button(BT_CALIB);
        draw_statusline();
        break;

    case MENU_MANUELL:
        // Manuell Menu
        TFT_Rectangle_filled(0,0,disp_width,50,DARKGREY);
        draw_title("L / C manuell", 40);
        putvalue_bargraph(0, 0, 0);
        putvalue_bargraph(1, 0, 0);
        draw_button(BT__RETURN);
        draw_font("SWR:", 5, disp_height - 80, VERDANA_BOLD, WHITE, 40); 
        draw_statusline();
        break;

    case MENU_CALIB:
        // show values for calibration
        TFT_Rectangle_filled(0,0,disp_width,50,DARKGREY);
        draw_title("Messwerte", 70);
        draw_button(BT__RETURN);
        draw_button(BT_CALIB_VAL);
        draw_button(BT_CALIB_ADC);
        draw_button(BT_CALIB_DIG);
        draw_button(BT_CALIB_RETURN);
        draw_statusline();
        break;

    default:
        draw_font("NOT IMPLEMENTED", 20, 10, VERDANA_BOLD_ITALIC, WHITE, 40);
        break;
    }

    act_menunum = menunum;
}

void draw_statusline()
{
   draw_font("Kilowatt Antennentuner DL1EV, DJ0ABR Version 1.0", 10, disp_height - 40, VERDANA, WHITE, 18);
}

void draw_title(char *text, int x)
{
    draw_font(text, x, 10, VERDANA_BOLD_ITALIC, WHITE, 50);
}

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

setup and handle all touch buttons

*
*/

#include "../ctlbrd.h"

void button_callback_main(int id);
void button_callback_to_main_menu(int id);
void button_callback_calib(int id);

void setup_touchbuttons()
{
    TOUCHBUTTON tb;

    // Main menu Buttons
    tb.menu = MENU_MAIN;
    tb.x = 0;
    tb.y = 50;
    tb.width = disp_width/2 - 4;
    tb.height = 120 - 4;
    tb.button_color = YELLOW;
    tb.text_color = GREY;
    strcpy(tb.text, "MANUELL");
    tb.font = VERDANA_BOLD;
    tb.fontsize = 30;
    tb.text_xoff = 40;
    tb.text_yoff = 35;
    tb.touchbutfunc = button_callback_main;
    create_touchbutton(&tb);

    tb.x = disp_width/2 + 2;
    strcpy(tb.text, "AUTO");
    tb.text_xoff = 70;
    create_touchbutton(&tb);

    tb.x = 0;
    tb.y += tb.height + 6;
    strcpy(tb.text, "SETUP");
    create_touchbutton(&tb);

    tb.x = disp_width/2 + 2;
    tb.width = disp_width/2 - 4;
    strcpy(tb.text, "CALIB");
    create_touchbutton(&tb);

    // Menu Manuell: Return Button
    tb.menu = MENU_MANUELL;
    tb.x = disp_width*3/4;
    tb.y = 228;
    tb.width = disp_width - tb.x - 3;
    tb.height = 60;
    tb.button_color = YELLOW;
    tb.text_color = GREY;
    strcpy(tb.text, "RETURN");
    tb.font = VERDANA_BOLD;
    tb.fontsize = 20;
    tb.text_xoff = 15;
    tb.text_yoff = 5;
    tb.touchbutfunc = button_callback_to_main_menu;
    create_touchbutton(&tb);

    // Menu Calib: digital
    tb.menu = MENU_CALIB;
    tb.x = disp_width*0/4;
    tb.y = 228;
    strcpy(tb.text, "DIG");
    tb.text_xoff = 40;
    tb.touchbutfunc = button_callback_calib;
    create_touchbutton(&tb);

    // Menu Calib: analog
    tb.x = disp_width*1/4;
    strcpy(tb.text, "ADC");
    tb.touchbutfunc = button_callback_calib;
    create_touchbutton(&tb);

    // Menu Calib: Values
    tb.x = disp_width*2/4;
    strcpy(tb.text, "VAL");
    tb.touchbutfunc = button_callback_calib;
    create_touchbutton(&tb);

    // Menu Calib: Return
    tb.x = disp_width*3/4;
    strcpy(tb.text, "RETURN");
    tb.text_xoff = 15;
    tb.touchbutfunc = button_callback_to_main_menu;
    create_touchbutton(&tb);
}

void button_callback_main(int id)
{
    act_menunum = id+1;
}

void button_callback_to_main_menu(int id)
{
    act_menunum = 0;
}

int getActMenu()
{
    return act_menunum;
}

extern int calib_mode;
void button_callback_calib(int id)
{
    if(id == BT_CALIB_RETURN) button_callback_to_main_menu(id);
    if(id == BT_CALIB_DIG) calib_mode = 0;
    if(id == BT_CALIB_ADC) calib_mode = 1;
    if(id == BT_CALIB_VAL) calib_mode = 2;
}
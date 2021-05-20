/*
* I2C Driver for Raspberry PI
* ========================
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

    driver for the mcp23017 port expander
* 
*/

#include "../ctlbrd.h"

int init_mcp23017()
{
    // U1: all outputs
    // Bank mode 0, seq. operation and all input is default already
    // switch to all output, except pins with layout error
    if(i2c_write_register(mcp23017_U1, IODIR, IODIRA_IVAL_U1 ) == -1) return -1;
    if(i2c_write_register(mcp23017_U1, IODIR+1, IODIRB_IVAL_U1 ) == -1) return -1;
    // invert outputs (because the hardware drivers are also inverting)
    if(i2c_write_register(mcp23017_U1, IPOL, IPOL_IVAL_U1 ) == -1) return -1;
    if(i2c_write_register(mcp23017_U1, IPOL+1, IPOL_IVAL_U1 ) == -1) return -1;

    // U2: all inputs
    // Bank mode 0, seq. operation and all input is default already
    // just need to activate pullups
    if(i2c_write_register(mcp23017_U2, GPPU, GPPU_IVAL_U2 ) == -1) return -1;
    if(i2c_write_register(mcp23017_U2, GPPU+1, GPPU_IVAL_U2 ) == -1) return -1;

    return 0;
}


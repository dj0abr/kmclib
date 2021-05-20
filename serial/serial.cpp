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
serial.cpp
============
handles the serial interface and the CIV interface

Raspi supporting WLAN (RPI3, 4 and Zero-W) need remapping of serial port
========================================================================
1. Switch-off serial login shell
    sudo raspi-config
    InterfaceOptions - Serial Port:
    login shell: NO
    hardware enabled: YES
2. map primary serial port to connector pins
    sudo nano /boot/config.txt and add at the end:
    # Switch serial ports
    dtoverlay=miniuart-bt
3. Reboot and check if ok:
    ls -l /dev/serial*
    serial0 must map to ttyAMA0

*/

#include "../ctlbrd.h"

// functions for the primary UART on the Raspi Pins 8 and 10
// =========================================================

int serid = -1;

// exit program on error. Check for above remapping or hardware error
// speed ... use definition Bxxx
void open_serial(int speed)
{
    serid = init_serial_interface("/dev/ttyAMA0","", speed);
    if(serid == -1)
    {
        printf("connot open primary serial interface\n");
        exit(0);
    }
}

// returns: -1...write error (write pipe overflow, in this case write slower)
int write_serial(int data)
{
    int ret = write_serpipe(serid, data, 'w');
    return ret;
}

// returns: -1=write buffer full, 0=ok
int write_serial_free()
{
    return check_pipe(serid, 'w');
}

// returns: -1...no data
int read_serial()
{
    return read_serpipe(serid,'r');
}

// Functions for the USB-serial converter located on the board
// ===========================================================

int civid = -1;

// speed ... use definition Bxxx
void open_civ(int speed)
{
    civid = init_serial_interface("AB0KOTJ8","0403", speed);
    if(civid == -1)
    {
        printf("connot open USB serial interface\n");
        exit(0);
    }
}

int read_civ()
{
    return read_serpipe(civid,'r');
}

int write_civ(int data)
{
    int ret = write_serpipe(civid, data, 'w');
    return ret;
}

int write_civ_free()
{
    return check_pipe(civid, 'w');    
}

// Functions for other USB-serial converters
// =========================================

#define MAXUSBSER   10
int serUSBid[MAXUSBSER];
int serUSBnum = 0;

// speed ... use definition Bxxx
int open_serialUSB(int speed, char *idserial, char *idVendor)
{
    serUSBid[serUSBnum] = init_serial_interface(idserial,idVendor, speed);
    if(serUSBid[serUSBnum] == -1)
    {
        printf("connot open USB serial interface\n");
        exit(0);
    }
    int ret = serUSBnum;
    serUSBnum++;
    return ret;
}

int read_serialUSB(int id)
{
    return read_serpipe(serUSBid[id],'r');
}

int write_serialUSB(int id, int data)
{
    int ret = write_serpipe(serUSBid[id], data, 'w');
    return ret;
}

int write_serialUSB_free(int id)
{
    return check_pipe(serUSBid[id], 'w');    
}


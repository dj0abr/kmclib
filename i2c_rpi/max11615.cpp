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
max11615.cpp
============
functions for the ADC

Setup-Byte:
-----------
Reg: 1          ... this is a setup byte
Sel 2-1-0: 010  ... ref=ext (2.5v)
CLK: 0          ... internal clock
BIP/-UNI: 0     ... unipolar
-RST: 1         ... no reset
X: 0            ... not used
Summary: 1010 0010 = 0xa2 (value for the setup byte)

Configuration-Byte:
-------------------
Reg: 0          ... this is a configuration byte
SCAN 1-0: 01    ... scan selected channel 8 times
CS 3-2-1-0: 0xxx... channel
SGL/-DIF: 1     ... single-ended
Summary: 0010 xxx1 = 0x21 (value for the configuration byte)

Speed:
------
needs abt. 3ms to read all ADC (at I2C clock = 100kHz)

*/

#include "../ctlbrd.h"

int init_adc()
{
    //setup + configuration byte
    uint8_t d[2] = {0xa2 , 0x21};
    if(i2c_write(MAX11615,d,2) == -1) return -1;

    return 0;
}

uint16_t read_adc16(int channel)
{
    // select channel
    uint8_t d = 0x21 + (channel<<1);
    if(i2c_write(MAX11615,&d,1) == -1) return -1;

    // read all 8 values
    uint8_t fifo[16];
    i2c_read(MAX11615,fifo,16);

    // convert to 16 bit
    uint16_t dvals[8];
    for(int i=0; i<8; i++)
        dvals[i] = ((fifo[i*2] & 0x0f)<<8) + fifo[i*2+1];

    // build mean value
    int mval = 0;
    for(int i=0; i<8; i++)
        mval += dvals[i];
    mval /= 8;

    return (uint16_t)mval;
}

// digital ADC value -> real voltage
float calcVoltage(uint16_t dv)
{
	return dv * VREF / (1<<12);   // ref voltage = 2,048v, 12bit=4096
}


#define RV		8200.0    // R to Vcc
 
//R[ntc] = Umess * RV / (UV - Umess)

// Conrad Sensor 500526
float temptab[] = {
	32650 , //    Ohm at 0 degrees
	25390 , //    5
	19900 , //    10
	15710 , //    15
	12490 , //    20
	10000 , //    25
	8057  , //    30
	6531  , //    35
	5327  , //    40
	4369  , //    45
	3603  , //    50
	2986  , //    55
	2488  , //    60
	2083  , //    65
	1752  , //    70
	1481  , //    75
	1258  , //    80
	1072  , //    85
	917.7   , //    90
	788.5   , //    95
	680.0   , //    100
	588.6   , //    105
	511.2   , //    110
	445.4   , //    115
	389.3   , //    120
	341.7   , //    125
	300.9   , //    130
	265.4   , //    135
	234.8   , //    140
	208.3   , //    145
	185.3   , //    150
	-1
};

// measured real voltage -> temperature
float cf_calc_temp(float Umess)
{
	float Rntc;
	int i;
	float x;

	// Umess ist die Spannung am ADC Eingang
	// jetzt berechne daraus den Widerstand des NTCs
	Rntc = Umess * RV / (VREF - Umess);

	// suche den Bereich in der Tabelle
	i=0;
	while(temptab[i]!=-1)
	{
		if(temptab[i] <= Rntc) break;
		i++;
	}

	if(i==0)
	{
		return -10; // kleiner als kleinster Wert
	}

	if(temptab[i]!=-1)
	{
		// Widerstandsbereich gefunden, interpoliere
		x = i - (Rntc - temptab[i])/(temptab[i-1] - temptab[i]);

		// x ist jetzt der interpolierte Tabellenindex
		// rechne ihn in die Temperatur um
		return  x*5.0;
	}

	return 160; // größer als größter Wert
}

// convert measured voltage on ADC6 to
// to real voltage on input ST11-2
float calc_voltage(float f)
{
    // Calculate Voltage
	// Divider 390k : 10k
	f = f *(390.0+10.0) / 10.0;

    return f;
}

// convert measured voltage on ADC7 to
// to real current on ST11 according R_SHUNT
float calc_current(float f)
{
	f = f / (20.0*R_SHUNT);
    return f;
}

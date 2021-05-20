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
sunposition.cpp
============
calculates actual azimuth and elevation of the sun

the original sun calculation routine was made by "Tim Taylor".
The code was found here:
https://www.mikrocontroller.net/attachment/43830/sonnenposition.c

*/

#include "../ctlbrd.h"
#include "sunposition.h"

JDatum Datum2JDatum(Zeit Jetzt)
{
double Stunden;
int8_t A, Jahrhunderte;
uint16_t C;
uint32_t B;
JDatum Jdatum;

    if (Jetzt.Monat<=2)
    {
        Jetzt.Jahr-=1;
        Jetzt.Monat+=12;
    } 

    Stunden=(double) Jetzt.Stunden/24 + (double) Jetzt.Minuten/1440;
    Jahrhunderte=(Jetzt.Jahr/100);
    A=2-Jahrhunderte+Jahrhunderte/4;
    B=365.25*(Jetzt.Jahr+4716);
    C=(30.6001*(Jetzt.Monat+1));
    Jdatum.jd0=(A + B + C + Jetzt.Tag - 1524.5);
    Jdatum.jd=(A + B + C + Jetzt.Tag + Stunden - 1524.5);
    Jdatum.T0=(Jdatum.jd0-2451545.0)/36525.0;
    Jdatum.T=Stunden*24;
    return Jdatum;
}

Koordinaten Positionsbestimmung(JDatum Jdatum, Standort Ort)
{
	double n,L,g,A;
	double Deklination, Ekliptik, Nenner, Nenner2, Rektaszension, Refraktion; // refraktion hinzugefügt
	double StundenwinkelFP, GStundenwinkelFP, OStundenwinkelFP, OStundenwinkel;
	Koordinaten Position;

	n=Jdatum.jd-2451545.0;
	L=fmod(280.46+0.9856474*n,360);
	g=fmod(357.528+0.9856003*n,360);
	A=L+(e*360/M_PI)*sin(g*M_PI/180)+(225*e*e/M_PI)*sin(2*g*M_PI/180);
	Ekliptik=23.439-0.0000004*n; //verändert
	Nenner=cos(A*M_PI/180);
	Rektaszension=atan((cos(Ekliptik*M_PI/180)*sin(A*M_PI/180))/Nenner)*180/M_PI;
	if (Nenner<0) Rektaszension+=180;
	Deklination=asin(sin(Ekliptik*M_PI/180)*sin(A*M_PI/180))*180/M_PI;
	StundenwinkelFP=fmod(6.697376+(2400.05134*Jdatum.T0)+(1.002738*Jdatum.T),24);
	GStundenwinkelFP=StundenwinkelFP*15;
	OStundenwinkelFP=GStundenwinkelFP+Ort.Laenge; // korrektur
	OStundenwinkel=OStundenwinkelFP-Rektaszension;
	Nenner2=cos(OStundenwinkel*M_PI/180)*sin(Ort.Breite*M_PI/180)-tan(Deklination*M_PI/180)*cos(Ort.Breite*M_PI/180); // korrektur
	Position.Azimut=atan(sin(OStundenwinkel*M_PI/180)/Nenner2)*180/M_PI;
	if (Nenner2<0) Position.Azimut+=180;
	// Position.Azimut=360-Position.Azimut;
	Position.Azimut+=180; // korrektur
	Position.Azimut=fmod(Position.Azimut, 360); // korrektur


	Position.Hoehenwinkel=asin(cos(Deklination*M_PI/180)*cos(OStundenwinkel*M_PI/180)*cos(Ort.Breite*M_PI/180)+sin(Deklination*M_PI/180)*sin(Ort.Breite*M_PI/180))*180/M_PI; // korrektur
	Refraktion=1.02/tan((Position.Hoehenwinkel+(10.3/(Position.Hoehenwinkel+5.11)))*M_PI/180); //ergänzung
	Position.Hoehenwinkel+=Refraktion/60; // ergänzung

	return Position;
}

int getInteger(char *s)
{
    char t[3];
    t[2] = 0;
    t[0] = s[0];
    t[1] = s[1];

    return atoi(t);
}

Koordinaten k;

void calcSonnenpos(char *z, char *d, float flat, float flon)
{
    Zeit now;

    now.Sekunden = getInteger(z+4);
    now.Minuten = getInteger(z+2);
    now.Stunden = getInteger(z);
    now.Tag = getInteger(d);
    now.Monat = getInteger(d+2);
    now.Jahr = 2000 + getInteger(d+4);

    // die Formel rechnet mit MEZ (Winterzeit), stelle die GPS Zeit von UTC auf MEZ
    /*if(now.Stunden == 0)
        now.Stunden = 23;
    else
        now.Stunden--;*/
    
    /*show("-----");
    show(z);
    show("-----");
    showi(now.Tag);
    show(".");
    showi(now.Monat);
    show(".");
    showi(now.Jahr);
    show(".");
    showi(now.Stunden);
    show(".");
    showi(now.Minuten);
    show(".");
    showi(now.Sekunden);
    show("-----");*/

    Standort qth;

    qth.Laenge= flon;
    qth.Breite= flat;
    
    JDatum jd = Datum2JDatum(now);

    k = Positionsbestimmung(jd, qth);

}

int getAz()
{
    double v = k.Azimut;
    return (uint32_t)v;
}

int getEl()
{
    double v = k.Hoehenwinkel;
    return (uint32_t)v;
}

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
gps.cpp
===========
reads serial data from a GPS mouse

usage:
======

Open a GPS process which opens a serial interface: gps_open(..)

use these functions to get data:
char *getTime();
char *getDate();
float getLatitude();
float getLongitude();
char *getQTHloc();
void getSunPos(double *pazimuth, double *pelevation);

*/
#include "../kmclib.h"
#include "gps.h"
#include "../kmlib/kmtimer.h"
#include "../kmlib/km_helper.h"
#include "../serial/serial.h"

typedef struct _GPSINFO_ {
    int serialID;
    char latitude[20];
    char longitude[20];
    char zeit[20];
    char datum[20];
} GPSINFO;

void gps_decoder(int d, GPSINFO *pgps);
void evalGPS(char *s, int len, GPSINFO *pgps);
char *CalculateGridSquare(float flon, float flat, char londir, char latdir);

pthread_mutex_t     gps_crit_sec;
#define LOCK	pthread_mutex_lock(&gps_crit_sec)
#define UNLOCK	pthread_mutex_unlock(&gps_crit_sec)

void *gpsproc(void *pdata);
pthread_t gps_tid;
GPSINFO gpsinfo;

/*
open GPS receiver using a serial IF
idserial ... serial ID of a USB-serial converter, NULL...use onboard serial IF
idVendor ... vendor ID of a USB-serial converter, NULL...use onboard serial IF
speed ... serial speed of GPS mouse
*/
int gps_open(char *idserial, char *idVendor, int speed)
{
    // open serial interface
    if(idserial == NULL || idVendor == NULL)
    {
        printf("open onboard serial IF for GPS\n");
        open_serial(speed);
        gpsinfo.serialID = -1;
    }
    else
    {
        printf("open USB/serial IF for GPS\n");
        gpsinfo.serialID = open_serialUSB(speed, idserial, idVendor);
    }

    int pret = pthread_create(&gps_tid,NULL,gpsproc, &gpsinfo);
    if(pret)
    {
        printf("GPS process NOT started\n");
        return -1;
    }
    usleep(100000);
    return 0;
}

void *gpsproc(void *pdata)
{
    pthread_detach(pthread_self());
    GPSINFO *pgps = (GPSINFO *)pdata;

    *pgps->zeit = 0;
    *pgps->datum = 0;
    *pgps->latitude = 0;
    *pgps->longitude = 0;

    printf("GPS process started\n");
    while(keeprunning)
    {
        int c;
        // read serial data
        if(pgps->serialID == -1)
            c = read_serial();
        else
            c = read_serialUSB(pgps->serialID);
        if(c == -1)
        {
            // no data
            usleep(10000);
            continue;
        }
        gps_decoder(c, pgps);
    }

    printf("exit GPS thread\n");
    pthread_exit(NULL);
}

void gps_decoder(int d, GPSINFO *pgps)
{
static int idx = 0;
static int receiving = 0;
static char s[GPSBUFLEN] = {0};

    // wenn es ein $ ist, geht ein neuer GPS Datensatz los
    if(d=='$')
    {
        idx = 0;
        receiving = 1;
    }

    if(!receiving) return;  // warten bis zum nächsten Datensatz, also bis zum $

    // Empfang des Datensatzes bis zum Ende oder bis buffer voll ist
    // trage empfangenes Byte in Buffer ein
    s[idx++] = d;

    if(idx >= GPSBUFLEN)
    {
        // buffer ist voll, Fehler, verwerfe Daten
        printf("Buffer overflow error");
        idx = 0;
        receiving = 0;
        return;
    }

    if(d=='*')
    {
        // Datensatz zu ende, lese noch Prüfsumme
        receiving = 2;
        return;
    }

    if(receiving == 2)
    {
        // erstes Byte der Prüfsumme
        receiving = 3;
        return;
    }

    if(receiving == 3)
    {
        // zweites Byte der Prüfsumme
        // alles fertig gelesen, verarbeite GPS Daten
        s[idx] = 0;
        evalGPS(s,idx,pgps);

        // und gehe wieder an den Anfang für den nächsten Datensatz
        receiving = 0;
        idx = 0;
    }
}

void evalGPS(char *s, int len,GPSINFO *pgps)
{
    //printf("sentence: <%s>\n",s);
    // checke die Prüfsumme
    unsigned char chksum = 0;
    char chktxt[10];
    for(int i=1; i<(len-3); i++)
    {
        chksum ^= s[i];
    }

    sprintf(chktxt,"%02X",chksum);

    if(!strcmp(chktxt,s+(len-2)))
    {
        //printf("Checksum OK\n");
    }
    else
    {
        printf(" Checksum ERROR ");
        printf("chksum calc: %s chksum rxed: %s\n",chktxt,s+(len-2));
        return;
    }

    // the GPS sentence is correctly received
    // now split it into fields
    char gpsfields[30][50];
    int numfields = 0;
    char *start = s;
    while(1)
    {
        char *hpk = strchr(start,',');
        if(hpk)
        {
            *hpk = 0;
            strcpy(gpsfields[numfields++],start);
            start = hpk+1;
        }
        else
        {
            // last part
            strcpy(gpsfields[numfields++],start);
            break;
        }
    }

    /*
    printf("%d fields\n",numfields);
    for(int i=0; i<numfields; i++) printf("%s\n",gpsfields[i]);
    */
    
    if(numfields < 10) return; // no data

    // filtere auf RMC Datensatz, da hier Datum und Uhrzeit enthalten sind
    if(strcmp(gpsfields[0],"$GPRMC") == 0)
    {
        LOCK;
        strcpy(pgps->zeit,gpsfields[1]);
        strcpy(pgps->latitude,gpsfields[3]);
        strcat(pgps->latitude,gpsfields[4]);
        strcpy(pgps->longitude,gpsfields[5]);
        strcat(pgps->longitude,gpsfields[6]);
        strcpy(pgps->datum,gpsfields[9]);
        UNLOCK;
    }
}

// =========== read GPS info by other programs ================

char *getTime()
{
    static char s[20];
    LOCK;
    strcpy(s,gpsinfo.zeit);
    UNLOCK;
    if(*s==0)
    {
        // no GPS info
        strcpy(s,get_utctime());
    }
    char *hp = strchr(s,'.');   // remove unused fraction of seconds, if there
    if(hp) *hp=0;
    return s;
}

char *getDate()
{
    static char s[20];
    LOCK;
    strcpy(s,gpsinfo.datum);
    UNLOCK;
    if(*s==0)
    {
        // no GPS info
        strcpy(s,get_utcdate());
    }
    return s;
}

float getLatitude()
{
    float f = 0;
    char s[20];

    LOCK;
    if(gpsinfo.latitude[0] == 0)
    {
        UNLOCK;
        #ifdef DEFAULT_LATITUDE
        return DEFAULT_LATITUDE;
        #else
        return 0;
        #endif
    }
    strcpy(s,gpsinfo.latitude);
    UNLOCK;

    float fm = atof(s+2);
    s[2] = 0;
    float fd = atof(s);

    fm += fd * 60;  // total minutes
    f = fm / 60;    // total deg
    
    return f;
}

float getLongitude()
{
    float f = 0;
    char s[20];

    LOCK;
    if(gpsinfo.longitude[0] == 0)
    {
        UNLOCK;
        #ifdef DEFAULT_LONGITUDE
        return DEFAULT_LONGITUDE;
        #else
        return 0;
        #endif
    }    
    strcpy(s,gpsinfo.longitude);
    UNLOCK;

    float fm = atof(s+3);
    s[3] = 0;
    float fd = atof(s);

    fm += fd * 60;  // total minutes
    f = fm / 60;    // total deg
    
    return f;
}

char *getQTHloc()
{
    static char s[20];
    char a,o;

    if(strlen(gpsinfo.longitude) < 5) return " ";
    if(strlen(gpsinfo.latitude) < 5) return " ";

    LOCK;
    o = gpsinfo.longitude[strlen(gpsinfo.longitude)-1];
    a = gpsinfo.latitude[strlen(gpsinfo.latitude)-1];
    UNLOCK;

    char *p = CalculateGridSquare(getLongitude(),getLatitude(),o,a);
    strcpy(s,p);
    return s;
}

void getSunPos(int *pazimuth, int *pelevation)
{
    calcSonnenpos(getTime(), getDate(), getLatitude(), getLongitude());
    *pazimuth = getAz();
    *pelevation = getEl();
}

/* calc qthloc from GPS_lat and GPS_lon
 * 
 * JO12AB
 * 
 * J ... longitude A..R ... 180W to 180E
 * 1 ... longitude 0..9 ... +0-18deg
 * A ... longitude a..x ... +0-2deg
 * 
 * O ... latitude  A..R ... 90S to 90N
 * 2 ... latitude  0..9 ... +0-9deg
 * B ... latitude  a..x ... +0-1deg (finest resolution: 1/24 = 0.04167 deg)
 * 
 * GPS delivers 1/100 min, which is 1/6000 deg.
 * therefore we multiplay degrees with 10.000, so we can use integer arithmetics
*/

char *CalculateGridSquare(float flon, float flat, char londir, char latdir) 
{
static char grid[7];

    uint32_t lon = (uint32_t)(flon * 10000.0);  // we have now lon * 10000
	
    // W or E, west is negative
    if(londir == 'W') lon = -lon;
    // and make it positive, 0 starts in west -180
    lon += 1800000;

   	// calculate lat
	uint32_t lat = (uint32_t)(flat * 10000.0);  // lat * 10000
	
    // N or S, south is negative
    if(latdir == 'S') lat = -lat;
    // and make it positive, 0 starts in south -90
    lat += 900000;

    uint32_t lonm = lon * 60L;
    uint32_t longf = lonm / (20L*60L*10000L);
    uint32_t lonrest1 = lonm - longf*(20L*60L*10000L);
    uint32_t lonmid = lonrest1 / (2L*60L*10000L);

    uint32_t latm = lat * 60L;
    uint32_t latgf = latm / (10L*60L*10000L);
    uint32_t latrest1 = latm - latgf*(10L*60L*10000L);
    uint32_t latmid = latrest1 / (1L*60L*10000L);
    
    grid[0] = 'A' + longf;
    grid[2] = '0' + lonmid;
    grid[4] = 'A' + ((lonrest1 - lonmid*(2L*60L*10000L)) / (5L*10000L));
    
    grid[1] = 'A' + latgf;
    grid[3] = '0' + latmid;
    grid[5] = 'A' + ((latrest1 - latmid*(1L*60L*10000L)) / (25L*1000L));
    
    grid[6] = 0;
    return grid;
}

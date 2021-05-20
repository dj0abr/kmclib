#ifndef _SonnenPosition_H
#define _SonnenPosition_H

#include <math.h>
#include <stdint.h>

#define e 0.0167

typedef struct{
 uint8_t Sekunden;
 uint8_t Minuten;
 uint8_t Stunden;
 uint8_t Tag;
 uint8_t Monat;
 uint16_t Jahr;
} Zeit;

typedef struct{
 double Azimut;
 double Hoehenwinkel;
} Koordinaten;

typedef struct{
 double Laenge;
 double Breite;
} Standort;

typedef struct{
 double jd;
 double jd0;
 double T;
 double T0;
} JDatum;

void show(char *s);
void showln(char *s);
void showi(long s);
void showc(char s);


Koordinaten Positionsbestimmung(JDatum, Standort);
JDatum Datum2JDatum(Zeit);

#endif
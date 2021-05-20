#define GPSBUFLEN 500

void calcSonnenpos(char *z, char *d, float flat, float flon);
int getAz();
int getEl();

int gps_open(char *idserial, char *idVendor, int speed);
char *getTime();
char *getDate();
float getLatitude();
float getLongitude();
char *getQTHloc();
void getSunPos(int *pazimuth, int *pelevation);

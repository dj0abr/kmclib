#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <getopt.h>
#include <pwd.h>
#include <unistd.h>

int isRunning(char *prgname);
void install_signal_handler(void (*signalfunction)());
void showbitstring(char* title, uint8_t* data, int totallen, int anz);
void showbytestring(char *title, uint8_t *data, int totallen, int anz);
void showbytestring16(char *title, uint16_t *data, int anz);
void showbytestring32(char* title, uint32_t* data, int anz);
void showbytestringf(char* title, float* data, int totallen, int anz);
void getParameters(int argc, char *argv[]);
char *getConfigElement_string(char *elemname);
void getConfigElement_double(char *elemname, double *pv, double multiplier);
void getConfigElement_longlong(char *elemname, long long *pv, double multiplier);
void getConfigElement_int(char *elemname, int *pv, double multiplier);


extern int keeprunning;

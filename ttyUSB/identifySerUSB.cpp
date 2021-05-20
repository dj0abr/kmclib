/*
 * identify a serial-USB adapter
 * 
 * by DJ0ABR
 * 
 * function:
 * 1. find which /dev/ttyU* adapters are available
 * 2. get their attributes using this command line function:
 * udevadm info -a -p  $(udevadm info -q path -n /dev/ttyUSB0) | grep '{serial}' | cut -d \" -f2 | head -n 1
 * 3. get the Vendor ID using this command line function:
 * udevadm info -a -p  $(udevadm info -q path -n /dev/ttyUSB0) | grep '{idVendor}' | cut -d \" -f2 | head -n 1
 * 
 * udevadm.... gets the complete information
 * grep ... checks for lines with serial
 * cut ... extracts the first line, which contains the ID
 * 
 * Enter the first line of the result into :
    #define SERID_serial "xxxxx"
    #define SERID_idVendor "yyyy"
 * 
 * 
 * 3. this function comparse this ID with an specified ID, if equal, we found the right serial port
 * 
 */

#include <stdio.h>
#include <string.h>
#include "identifySerUSB.h"

void scan_serial_devices();
void get_serial_IDs();

#define SERDEVNAMELEN       50
#define SERDEVIDLEN         50
char serdev_name[MAXSERIALDEVICES][SERDEVNAMELEN];
char serdev_id[MAXSERIALDEVICES][SERDEVIDLEN];
char serdev_idVendor[MAXSERIALDEVICES][SERDEVIDLEN];

char *get_serial_device_name(char *SERID_serial, char *SERID_idVendor)
{
    // if SERID_serial starts with tty, then it is the device name already, so no search
    if(strncmp(SERID_serial,"/dev/tty",8) == 0)
    {
        return SERID_serial;
    }

    scan_serial_devices();
    get_serial_IDs();
    
    // check which adapter has the requested ID
    for(int i=0; i<MAXSERIALDEVICES; i++)
    {
        if(serdev_name[i][0] == 0)
            break;  // nothing found
            
        if(strstr(serdev_id[i],SERID_serial) && strstr(serdev_idVendor[i],SERID_idVendor))
        {
            printf("serial-USB adapter found: %s with serial:%s and idVendor:%s",serdev_name[i],serdev_id[i],serdev_idVendor[i]);
            return serdev_name[i];
        }
    }
    return NULL;
}

// fill serdev_name with all serUSB devices names available on the computer
void scan_serial_devices()
{
    // clear storage
    memset(serdev_name,0,sizeof(char)*MAXSERIALDEVICES*SERDEVNAMELEN);
    memset(serdev_id,0,sizeof(char)*MAXSERIALDEVICES*SERDEVIDLEN);
    
    printf("scan for serial ports\n");
    char s[50];
    snprintf(s,49,"ls -1 /dev/ttyU*");
    s[49] = 0;
    int serportnum = 0;
    FILE *fp = popen(s,"r");
    if(fp)
    {
        while (fgets(s, sizeof(s)-1, fp) != NULL) 
        {
            // delete \n
            if(strlen(s) > 5 && s[strlen(s)-1] == '\n')
                s[strlen(s)-1] = 0;
            strcpy(serdev_name[serportnum++],s);
            if(serportnum >= MAXSERIALDEVICES) break;
        }
        pclose(fp);
    }
    else
        printf("ERROR: cannot execute ls command\n");
}

// fill serdev_id with the IDs of the devices
void get_serial_IDs()
{
char s[1000];
FILE *fp;

    for(int i=0; i<MAXSERIALDEVICES; i++)
    {
        if(serdev_name[i][0] == 0) break;

        // read serial string
        snprintf(s,999,"udevadm info -a -p  $(udevadm info -q path -n %s) | grep '{serial}' | cut -d \\\" -f2 | head -n 1",serdev_name[i]);
        //printf("%s\n",s);
        s[999] = 0;
        fp = popen(s,"r");
        if(fp)
        {
            while (fgets(s, sizeof(s)-1, fp) != NULL) 
            {
                // delete \n
                if(strlen(s) > 5 && s[strlen(s)-1] == '\n')
                    s[strlen(s)-1] = 0;
                strcpy(serdev_id[i],s);
                //printf("%s\n",s);
                break;
            }
            pclose(fp);
        }
        else
            printf("ERROR: cannot execute udevadm... command\n");

        
        // read idVendor string
        snprintf(s,999,"udevadm info -a -p  $(udevadm info -q path -n %s) | grep '{idVendor}' | cut -d \\\" -f2 | head -n 1",serdev_name[i]);
        //printf("%s\n",s);
        s[999] = 0;
        fp = popen(s,"r");
        if(fp)
        {
            while (fgets(s, sizeof(s)-1, fp) != NULL) 
            {
                // delete \n
                if(strlen(s) > 5 && s[strlen(s)-1] == '\n')
                    s[strlen(s)-1] = 0;
                strcpy(serdev_idVendor[i],s);
                //printf("%s\n",s);
                break;
            }
            pclose(fp);
        }
        else
            printf("ERROR: cannot execute udevadm... command\n");
    }
}

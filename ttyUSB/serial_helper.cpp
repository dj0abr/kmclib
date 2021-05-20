/*
 * serial_helper
 * =============
 * by DJ0ABR
 * 
 * this program solves the problem of ttyUSB numbering. Everytime a serialUSB converter is connected it gets a ttyUSBxxx number.
 * This number can change all the time, its never sure which number it gets.
 * Using the serial-ID and Vendor-ID of the USBserial adapter we can easily identify the correct serial port.
 * 
 * * identifies a serialUSB adapter
 * * opens the serial interface
 * * start an RX thread
 * * provides a function to read received data async via a thread safe pipe
 * * provides a function to send data
 * 
 * int init_serial_interface(char *idserial, char *idVendor, int speed) ...
 *      create a worker thread and a pipe for a new serial port, then open the port using the SerialID and VendorID of a USBserial Interface
 *      return ... ID of the serial interface (which is a simple index, starting with 0)
 * 
 * int read_pipe(int idx, char direction) ...
 *      read data from the serial interface
 *      idx ... ID returned by init_serial_interface()
 *      direction ... only 'r' is allowed ('w' could be used, if implemented, by the thread to read data from the user and to send it via serial interface)
 * 
 * usage:
 *      1) get the serial- and Vendor ID of your USBserial Interface (see comments in indentifySerUSB.c)
 *      2) make a call to  init_serial_interface with above IDs and specify the serial speed
 *      3) repeadately call read_pipe(idx, 'r') to get the data received from the serial port
 *          
 * 
 * */

#include "../kmclib.h"
#include "identifySerUSB.h"

//#define VERBOSE     // activate to print received data into the terminal

int open_serial(char *serdevice, int *pfd_ser, int speed);
int read_serport(int *pfd_ser);
int write_serport(int *pfd_ser, int data);
void initpipes();
void showData(char *devname, int databyte, int pipeidx);

int serport_idx = 0;  // increments which each used serial port, used as an index
void *serialproc(void *pdata);
pthread_t serial_tid[MAXSERIALDEVICES]; 
int sstarted = 0;

typedef struct {
    char serdevname[20];
    int serspeed;
    int seridx;
    char idSerial[256];
    char idVendor[256];
} SERDATA;

SERDATA serdata[MAXSERIALDEVICES];

// ret: serID, -1=error
int init_serial_interface(char *idserial, char *idVendor, int speed)
{
int serid = serport_idx;

    if(speed > 100)
    {
        printf("wrong speed value, use i.e. B9600, not 9600 !\n");
        exit(0);
    }

    initpipes();
    
    // create a new thread for this device
    strcpy(serdata[serid].idSerial, idserial);
    strcpy(serdata[serid].idVendor, idVendor);
    serdata[serid].serspeed = speed;
    serdata[serid].seridx = serid;
    int ret = pthread_create(&serial_tid[serid],NULL,serialproc, &(serdata[serid]));
    if(ret)
    {
        printf("serial process %d NOT started\n",serid);
        return -1;
    }
    serport_idx++;
    
    usleep(100000);
    
    return serid;
}

void *serialproc(void *pdata)
{
int fd_ser = -1; // handle of the ser interface
char devname[20];
int speed;
int idx;

    pthread_detach(pthread_self());
    
    SERDATA *psd = (SERDATA *)pdata;
    
    //strcpy(devname,psd->serdevname);
    speed = psd->serspeed;
    idx = psd->seridx;
    
    printf("process for %s, %s started\n",psd->idSerial,psd->idVendor);
    sstarted = 1;
    
    while(keeprunning)
    {
        // if the device is closed, open it
        if(fd_ser == -1)
        {
            char *sif = get_serial_device_name(psd->idSerial, psd->idVendor);
            if(sif)
            {
                strcpy(devname,sif);    // sif has the valid device name 
                open_serial(devname,&fd_ser,speed);
                usleep(1000);
            }
            else
                usleep(1000000);    // device not found, wait a little longer
        }
        else
        {
            int action = 0;

            // serial IF is open, read data unblocked
            int d = read_serport(&fd_ser);
            if(d != -1)
            {
                showData(devname,d,idx);
                write_serpipe(idx,d,'r');
                action = 1;
            }

            // check if we got data to send
            int rp = read_serpipe(idx,'w');
            if(rp != -1)
            {
                write_serport(&fd_ser,rp);
                action = 1;
            }

            if(action == 0)
                usleep(10000);  // nothing to do, wait 10ms
        }
    }
    
    printf("exit serial thread\n");
    if(fd_ser != -1) close(fd_ser);
    pthread_exit(NULL);
}

#define MAXTEXTLEN  100
void showData(char *devname, int databyte, int pipeidx)
{
#ifdef VERBOSE
    
    static int charnum[MAXSERIALDEVICES];
    static char text[MAXSERIALDEVICES][(MAXTEXTLEN+1)*3];
    static int f = 1;
    
    if(f)
    {
        f = 0;
        memset(text,0,(MAXTEXTLEN+1)*3*MAXSERIALDEVICES);
        memset(charnum,0,MAXSERIALDEVICES*sizeof(int));
    }
    
    // a new line is either a \n or if the maximum number of chars per line is exceeded
    if(databyte == '\n' || charnum[pipeidx] >= MAXTEXTLEN)
    {
        charnum[pipeidx] = 0;
        printf("\n%s %d: %s",devname,pipeidx,text[pipeidx]);
        *text[pipeidx] = 0;
    }
    
    if(databyte != '\n')
    {
        if(databyte >= ' ' && databyte <= 'z')
            sprintf(text[pipeidx]+strlen(text[pipeidx]),"%c",databyte);
        else
            sprintf(text[pipeidx]+strlen(text[pipeidx]),"x%02X",databyte);
        charnum[pipeidx]++;
    }
    
#endif
}

// open serial interface
// 1=ok, 0=error
int open_serial(char *serdevice, int *pfd_ser, int speed)
{
struct termios tty;
    
	if(*pfd_ser != -1) close(*pfd_ser);
    printf("Open: %s\n",serdevice);
    
	*pfd_ser = open(serdevice, O_RDWR | O_NDELAY);
	if (*pfd_ser < 0) {
		printf("error when opening %s, errno=%d\n", serdevice,errno);
		return 0;
	}

	if (tcgetattr(*pfd_ser, &tty) != 0) {
		printf("error %d from tcgetattr %s\n", errno,serdevice);
		return 0;
	}

	cfsetospeed(&tty, speed);
	cfsetispeed(&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_iflag &= ~ICRNL; // binary mode (no CRLF conversion)
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_cc[VMIN] = 0; // 0=nonblocking read, 1=blocking read
	tty.c_cc[VTIME] = 0;
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_cflag |= (CLOCAL | CREAD);
	tty.c_cflag &= ~(PARENB | PARODD | CSTOPB);
	tty.c_cflag &= ~CRTSCTS;
    
    if (tcsetattr(*pfd_ser, TCSANOW, &tty) != 0) {
		printf("error %d from tcsetattr %s\n", errno, serdevice);
        *pfd_ser = -1;
		return 0;
	}
	
	// set RTS/DTR
    int flags;
    ioctl(*pfd_ser, TIOCMGET, &flags);
    flags &= ~TIOCM_DTR;
    flags &= ~TIOCM_RTS;
    ioctl(*pfd_ser, TIOCMSET, &flags);
    
    printf("%s opened\n",serdevice);
	return 1;
}

// read one byte non blocking
// ret: byte or -1 if no data
int read_serport(int *pfd_ser)
{
static unsigned char c;

    // test if serial IF is still available
    int flags;
    if(ioctl(*pfd_ser, TIOCMGET, &flags) == -1)
    {
        // serial IF lost
        *pfd_ser = -1;
        return -1;
    }
    
    if(*pfd_ser != -1)
    {
        int rxlen = read(*pfd_ser, &c, 1);
        if( rxlen <= 0)
            return -1;

        return (unsigned int)c;
    }
    return -1;
}

int write_serport(int *pfd_ser, int data)
{
    // test if serial IF is still available
    int flags;
    if(ioctl(*pfd_ser, TIOCMGET, &flags) == -1)
    {
        // serial IF lost
        *pfd_ser = -1;
        return -1;
    }
    
    if(*pfd_ser != -1)
    {
        uint8_t d = data & 0xff;
        int wrlen = write(*pfd_ser,&d,1);
        if(wrlen != 1)
            return -1;
        return 0;
    }
    return -1;
}

// ================== serial pipes =======================

#define NUM_OF_PIPES MAXSERIALDEVICES*2 // *2 weil einmal für direction 'r' und einmal für 'w'

pthread_mutex_t crit_sec[NUM_OF_PIPES];
pthread_mutex_t waitmutex[NUM_OF_PIPES];
pthread_cond_t	semaphore[NUM_OF_PIPES];
#define LOCK(pn)	pthread_mutex_lock(&(crit_sec[pn]))
#define UNLOCK(pn)	pthread_mutex_unlock(&(crit_sec[pn]))
#define WAKEUP(pn)	pthread_cond_signal(&(semaphore[pn]));

#define BUFFER_LENGTH 20

int wridx[NUM_OF_PIPES];
int rdidx[NUM_OF_PIPES];
uint8_t pipebuffer[NUM_OF_PIPES][BUFFER_LENGTH];

void initpipes()
{
static int f=1;

    if(f==0) return; // already initialized

	for (int i = 0; i < NUM_OF_PIPES; i++)
	{
		pthread_mutex_init(&(crit_sec[i]),NULL);
		pthread_cond_init(&(semaphore[i]),NULL);
		pthread_mutex_init(&(waitmutex[i]),NULL);

        rdidx[i] = wridx[i] = 0;
	}
	f=0;
}

// returns: -1=pipe full ,0=ok
int check_pipe(int idx, char direction)
{
int pipenum;

    pipenum = idx*2;
    if(direction == 'w') pipenum+=1;

    if (pipenum >= NUM_OF_PIPES || pipenum < 0)
	{
		printf("invalid write_pipe pipnum: %d idx:%d\n", pipenum,idx);
        exit(0);
	}

	LOCK(pipenum);
	int *pwridx = &(wridx[pipenum]);
    int v = ((*pwridx + 1) % BUFFER_LENGTH) == rdidx[pipenum];
    UNLOCK(pipenum);
	if (v)
		return -1;

    return 0;
}

// ret: 1=ok, 0=error
int write_serpipe(int idx, int data, char direction)
{
int pipenum;

    pipenum = idx*2;
    if(direction == 'w') pipenum+=1;

    if (pipenum >= NUM_OF_PIPES || pipenum < 0)
	{
		printf("invalid write_pipe pipnum: %d idx:%d\n", pipenum,idx);
        exit(0);
	}


	LOCK(pipenum);

	int *pwridx = &(wridx[pipenum]);
	// prüfe ob im Fifo noch Platz ist
	if (((*pwridx + 1) % BUFFER_LENGTH) == rdidx[pipenum])
	{
		UNLOCK(pipenum);
        printf("no space left in pipenum: %d\n", pipenum);
		return -1;
	}

    //printf("write pipe %d: %02X\n",pipenum,data);

	// schreibe Daten in den Fifo
	pipebuffer[pipenum][*pwridx] = (unsigned char)data;

    // stelle Schreibzeiger weiter
	if (++*pwridx == BUFFER_LENGTH) *pwridx = 0;
	UNLOCK(pipenum);

	return 0;
}

int read_serpipe(int idx, char direction)
{
int pipenum;

    pipenum = idx*2;
    if(direction == 'w') pipenum+=1;

	if (pipenum >= NUM_OF_PIPES)
	{
		printf("invalid read_pipe pipnum: %d\n", pipenum);
        exit(0);
	}

	LOCK(pipenum);

	int *prdidx = &(rdidx[pipenum]);

	if (*prdidx == wridx[pipenum])
	{
		// Fifo ist leer
		UNLOCK(pipenum);
		return -1;
	}

	// lese Daten
	int d = pipebuffer[pipenum][*prdidx];
    
	// stelle Lesezeiger weiter
	if (++*prdidx == BUFFER_LENGTH) *prdidx = 0;
	UNLOCK(pipenum);

    // printf("%d read: %02X\n",pipenum,d);

	return d;
}

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
*
*   Raspi I2C:
*   ==========
*   1) enable via raspi-config
*       this adds dtparam=i2c_arm=on to /boot/config.txt
*   2) set I2C bus speed in /boot/config.txt:
*       dtparam=i2c_arm=on,i2c_arm_baudrate=400000
*   3) reboot
* 
*   usage:
*   ======
*   set or reset an output pin:
*       setPort(port, onoff)
*       port ... output port number as defined in gpio.h
*       onoff .. 0 or 1
*
*   read an input pin
*       onoff = getPort(port)
*       port ... input port number as defined in gpio.h
*       onoff .. returns: 0 or 1
*
*   read an ADC value
*       getADC(channel)
*       channel ... ADC channel as defined in gpio.h
*       returns:ADC0-3 ... voltage on pin
*               TEMP1, TEMP2 ... temperature in degC
*               CURRENT ... real current through shunt
*               VOLTAGE ... real voltage on pin
*/


#include "../kmclib.h"
#include "i2c_rpi.h"
#include "mcp23017.h"
#include "max11615.h"
#include "gpio.h"


#define NUM_OF_I2C_PIPES 3 // 0=output 1=input 2=adc

pthread_mutex_t     i2c_crit_sec[NUM_OF_I2C_PIPES];
#define LOCK(pn)	pthread_mutex_lock(&(i2c_crit_sec[pn]))
#define UNLOCK(pn)	pthread_mutex_unlock(&(i2c_crit_sec[pn]))

void *i2cproc(void *);    

int i2cdev = -1;
int act_addr = -1;
pthread_t i2c_tid;

// open I2C Bus
// start I2C handling thread
// init critical sections to make variables thread safe
int i2c_init()
{
    // prepare access rights for i2c
    // -->> activate i2c in raspi-config
    // now the new device /dev/i2c-1 is created
    // add the user to the i2c group (may be already done by raspi-config):
    //   add user pi i2c
    // infos about the i2c bus:  i2cdetect -F 1  ("1" means i2c-1)
    // scan for i2c Devices:  i2cdetect -y 1
    // changing the i2c speed (default is 100kHz): /boot/config.txt
    //    dtparam=i2c_arm=on,i2c_arm_baudrate=400000 (add the speed parameter)
    //    then reboot
    // 1) unload module: modprobe -r i2c_bcm2835
    // 2) load module: modprobe isc_bcm2835 baudrate=400000

    for (int i = 0; i < NUM_OF_I2C_PIPES; i++)
		pthread_mutex_init(&(i2c_crit_sec[i]),NULL);

    // open i2c device
    i2cdev = open((char*)"/dev/i2c-1", O_RDWR);
    if(i2cdev < 0)
    {
        printf("cannot open I2C bus\n");
        i2cdev = -1;
        return -1;
    }

    printf("I2C bus OPEN, bus-id.: %d\n",i2cdev);  

    int pret = pthread_create(&i2c_tid,NULL,i2cproc, NULL);
    if(pret)
    {
        printf("i2c process NOT started\n");
        exit(0);
    }

    return 0;
}

// close I2C bus at program end
void i2c_close()
{
    if(i2cdev != -1)
    {
        printf("Close I2C bus\n");
        close(i2cdev);
    }
}

// select an I2C device
int i2c_seldev(int addr)
{
	if (ioctl(i2cdev, I2C_SLAVE, addr) < 0)
	{
		printf("I2C: cannot access slave: %d\n",addr);
        act_addr = -1;
		return -1;
	}
    act_addr = addr;
    return 0;
}

// write a byte array to the selected device
int i2c_write(int addr, uint8_t *pdata, int len)
{
    if(i2c_seldev(addr) == -1) return -1;

	if (write(i2cdev, pdata, len) != len)
	{
		printf("I2C: write error, device:%d on bus-id:%d\n",act_addr,i2cdev);
        return -1;
	}
    return 0;
}

int i2c_write_byte(uint8_t i2caddr, uint8_t reg, uint8_t data)
{
    if(i2c_seldev(i2caddr) == -1) return -1;

    uint8_t d[2];
    d[0] = reg; // register in the chip
    d[1] = data;
	if (write(i2cdev, &d, 2) != 2)
	{
		printf("I2C: write error, device:%d on bus-id:%d\n",act_addr,i2cdev);
        return -1;
	}
    return 0;
}

// read a byte array from the selected device
int i2c_read(int addr, uint8_t *pdata, int len)
{
    if(i2c_seldev(addr) == -1) return -1;
	if (read(i2cdev, pdata, len) != len)
	{
		printf("I2C: read error, device:%d on bus-id:%d\n",act_addr,i2cdev);
        return -1;
	}

    return len;
}

int i2c_write_register(uint8_t slave_addr, uint8_t reg, uint8_t data) 
{
    uint8_t outbuf[2];

    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset[1];

    outbuf[0] = reg;
    outbuf[1] = data;

    msgs[0].addr = slave_addr;
    msgs[0].flags = 0;
    msgs[0].len = 2;
    msgs[0].buf = outbuf;

    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 1;

    if (ioctl(i2cdev, I2C_RDWR, &msgset) < 0) 
    {
        printf("I2C: register %02X read error, device:%d on bus-id:%d\n",reg,act_addr,i2cdev);
        return -1;
    }

    return 0;
}

int i2c_write_register_duo(uint8_t slave_addr, uint8_t reg, uint8_t *pdata) 
{
    uint8_t outbuf[3];

    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset[1];

    outbuf[0] = reg;
    outbuf[1] = pdata[0];
    outbuf[2] = pdata[1];

    msgs[0].addr = slave_addr;
    msgs[0].flags = 0;
    msgs[0].len = 3;
    msgs[0].buf = outbuf;

    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 1;

    if (ioctl(i2cdev, I2C_RDWR, &msgset) < 0) 
    {
        printf("I2C: register %02X read error, device:%d on bus-id:%d\n",reg,act_addr,i2cdev);
        return -1;
    }

    return 0;
}

int i2c_read_register(uint8_t addr, uint8_t reg) 
{
    uint8_t inbuf, outbuf;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    outbuf = reg;
    messages[0].addr  = addr;
    messages[0].flags = 0;
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = &outbuf;

    messages[1].addr  = addr;
    messages[1].flags = I2C_M_RD | I2C_M_NOSTART;
    messages[1].len   = sizeof(inbuf);
    messages[1].buf   = &inbuf;

    packets.msgs      = messages;
    packets.nmsgs     = 2;
    if(ioctl(i2cdev, I2C_RDWR, &packets) < 0) 
    {
		printf("I2C: register %02X read error, device:%d on bus-id:%d\n",reg,act_addr,i2cdev);
        return 0;
    }

    return inbuf;
}

// ======================= set port, get port, get ADC value ==========================
//
// to be used be applications
// this is thread safe

uint8_t inputs[NUMINPUTS];
uint8_t outputs[NUMOUTPUTS];
uint16_t adcinputs[NUMADCINPUTS];

void setPort(int port, int onoff)
{
    LOCK(0);
    outputs[port] = onoff?1:0;
    UNLOCK(0);
}

int getOutPort(int port)
{
    LOCK(1);
    uint8_t v = outputs[port];
    UNLOCK(1);
    return v;
}

int getPort(int port)
{
    LOCK(1);
    uint8_t v = inputs[port];
    UNLOCK(1);
    return v;
}

// return real voltage on ADC input of MAX11615 chip
float getADCvoltage(int channel)
{
    uint16_t v = 0;
    int to = 100;

    LOCK(2);
    adcinputs[channel] = 0xffff;   // request conversion
    UNLOCK(2);
    while(1)
    {
        LOCK(2);
        v = adcinputs[channel];
        UNLOCK(2);
        if(v != 0xffff) break;

        usleep(10);
        if(--to <= 0) break;
    }

    return calcVoltage(v);
}

// get real voltage on analog input pins
float getADC(int channel)
{
    float fv = getADCvoltage(channel);

    if(channel >= ADC0 && channel <= ADC3)
        return fv;

    if(channel >= TEMP1 && channel <= TEMP2)
        return cf_calc_temp(fv);

    if(channel == CURRENT)
        return calc_current(fv);

    return calc_voltage(fv);
}


// ============================================================================
// I2C Process
// !!! this is the ONLY process allowed to access the I2C bus directly !!!
// ============================================================================

void read_gpios()
{
    // read all input ports
    uint8_t u2a, u2b;
    u2a = i2c_read_register(mcp23017_U2, GPIO);
    u2b = i2c_read_register(mcp23017_U2, GPIO+1);

    LOCK(1);
    inputs[KEY0] = (u2a & 0x01)?1:0;
    inputs[KEY1] = (u2a & 0x02)?1:0;
    inputs[KEY2] = (u2a & 0x04)?1:0;
    inputs[KEY3] = (u2a & 0x08)?1:0;
    inputs[IN4]  = (u2a & 0x10)?1:0;
    inputs[IN5]  = (u2a & 0x20)?1:0;
    inputs[IN6]  = (u2a & 0x40)?1:0;
    inputs[IN7]  = (u2a & 0x80)?1:0;

    inputs[IN8] = (u2b & 0x01)?1:0;
    inputs[IN9] = (u2b & 0x02)?1:0;
    inputs[IN10] = (u2b & 0x04)?1:0;
    inputs[IN11] = (u2b & 0x08)?1:0;
    inputs[OPTOIN12]  = (u2b & 0x10)?1:0;
    inputs[OPTOIN13]  = (u2b & 0x20)?1:0;
    UNLOCK(1);
}

void write_gpios()
{
    uint8_t u1a, u1b;
    static uint8_t old_u1a, old_u1b;

    LOCK(0);
    u1a = (outputs[OUT0] << 0);
    u1a |= (outputs[OUT1] << 1);
    u1a |= (outputs[OUT2] << 2);
    u1a |= (outputs[OUT3] << 3);
    u1a |= (outputs[OUT4] << 4);
    u1a |= (outputs[OUT5] << 5);
    u1a |= (outputs[OUT6] << 6);
    u1a |= (outputs[OUT7] << 7);

    u1b = (outputs[OUT8] << 0);
    u1b |= (outputs[OUT9] << 1);
    u1b |= (outputs[OUT10] << 2);
    u1b |= (outputs[OUT11] << 3);
    u1b |= (outputs[OUT12] << 4);
    u1b |= (outputs[OUT13] << 5);
    u1b |= (outputs[HSOUT14] << 6);
    u1b |= (outputs[HSOUT15] << 7);
    UNLOCK(0);

    if(u1a != old_u1a && u1b == old_u1b)
    {
        i2c_write_register(mcp23017_U1, OLAT, u1a) ;
        old_u1a = u1a;    
    }
    else if(u1b != old_u1b && u1a == old_u1a)
    {
        i2c_write_register(mcp23017_U1, OLAT+1, u1b) ;
        old_u1b = u1b;    
    }
    else
    {
        uint8_t u1[2];
        u1[0] = u1a;
        u1[1] = u1b;
        i2c_write_register_duo(mcp23017_U1, OLAT, u1) ;
        old_u1a = u1a;    
        old_u1b = u1b;    
    }
}

void read_adc()
{
    uint16_t dv,v;
    for(int c=0; c<NUMADCINPUTS; c++)
    {
        LOCK(2);
        dv = adcinputs[c];
        UNLOCK(2);
        if(dv == 0xffff)
        {
            v = read_adc16(c);
            LOCK(2);
            adcinputs[c] = v;
            UNLOCK(2);
        }
    }
}

void *i2cproc(void *pdata)
{
    pthread_detach(pthread_self());
    
    printf("i2c process started\n");
    init_mcp23017();
    init_adc();

    //while(keeprunning) // do not force exit, some processes need to close ports before exit
    while(1)
    {
        read_gpios();
        read_adc();
        write_gpios();
        usleep(10);
    }

    printf("exit i2c thread\n");
    i2c_close();
    pthread_exit(NULL);

}

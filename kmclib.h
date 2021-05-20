#include <stdio.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <freetype2/ft2build.h>
#include <freetype/freetype.h>


#include "ttyUSB/serial_helper.h" 
#include "udp/udp.h" 
#include "websocket/websocketserver.h" 
#include "kmlib/kmtimer.h" 
#include "kmlib/km_helper.h" 
#include "kmlib/kmfifo.h" 
#include "i2c_rpi/i2c_rpi.h"
#include "i2c_rpi/mcp23017.h"
#include "i2c_rpi/max11615.h"
#include "i2c_rpi/gpio.h"
#include "serial/serial.h"
#include "stepper/stepper.h"
#include "motor/motor.h"
#include "gps/gps.h"
#include "rotary/encoder.h"
#include "display/display.h"
#include "display/bargraph.h"
#include "touch/xpt2046_rpi.h"
#include "touch/touch_button.h"


// entry point for an application
int getActMenu();   // used by the touch button logic to assign buttons to menus

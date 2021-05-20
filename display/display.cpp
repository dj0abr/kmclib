/*
* Display Driver for Raspberry PI's 3,5" SPI display
* ==================================================
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


* Instructions for Raspi OS
===========================

1) make a fresh install of Rasp IO
2) setup PRI using raspi-config
    enable: ssh, i2c, spi
    enable: serial, but disable serial console
3) set locale to own country
4) reboot
5) install nfs-client (in my case only, because I have my sources on a network server)

* Instructions for DietPI
=========================
1) make a fresh install of DietPi 32 bit
2) dietpi-config: activate ssh, i2c 400kHz and spi
3) set locale to own country
4) reboot
5) install nfs-client (in my case only, because I have my sources on a network server)

* additional instructions for Raspi-OS and DietPi
=================================================
6) run the script prepare_ubuntu included in this software package
7) sudo cp tft35a-overlay.dtb /boot/overlays/
   sudo cp tft35a-overlay.dtb /boot/overlays/tft35a.dtbo
    (this dtb file is from https://github.com/goodtft/LCD-show. Do NOT run the scripts from this package
    because it will route the console output to the LCD which makes it unsuable for this project)
8) add to config.txt (if not already there):
    hdmi_force_hotplug=1
    dtoverlay=tft35a:rotate=90
    dtparam=i2c_arm=on
    dtparam=i2c1=on (optional)
    dtparam=i2c_arm_baudrate=400000
    dtparam=spi=on
9) reboot
--- now the framebuffer /dev/fb1 (LCD Display) and the touch are working ---

*
*/

#include "../kmclib.h"
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
char *fbp = 0;
int disp_width, disp_height;

int display_active = 0;

void display_init()
{
    // open frame buffer device
    int fbfd = open("/dev/fb1", O_RDWR);
    if (fbfd == -1) 
    {
        printf("No framebuffer device. Display not used\n");
        display_active = 0;
        return;
    }

    // get screen resolution
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) 
    {
        printf("Error reading display information.\n");
        display_active = 0;
        return;
    }
    printf("Display X x Y = %d x %d, resolution: %d bpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
    disp_width = vinfo.xres;
    disp_height = vinfo.yres;

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) 
    {
        printf("Error reading fixed information.\n");
        display_active = 0;
        return;
    }

    // map fb to user mem 
    screensize = finfo.smem_len;
    fbp = (char*)mmap(0, screensize,  PROT_READ | PROT_WRITE,  MAP_SHARED,  fbfd,  0);

    if (fbp == MAP_FAILED) 
    {
        printf("Failed to map framebuffer to user memory\n");
        display_active = 0;
        return;
    }

    // initialisation now finished
    // draw to display by writing to framebuffer "* fbp"

    printf("Display initialized\n");
    display_active = 1;

    display_clear();
}

void put_pixel_RGB24(int x, int y, int r, int g, int b)
{
    // calculate the pixel's byte offset inside the buffer
    // note: x * 3 as every pixel is 3 consecutive bytes
    unsigned int pix_offset = x * 3 + y * finfo.line_length;

    // now this is about the same as 'fbp[pix_offset] = value'
    *((char*)(fbp + pix_offset)) = b;
    *((char*)(fbp + pix_offset + 1)) = g;
    *((char*)(fbp + pix_offset + 2)) = r;

}

void put_pixel_RGB565(int x, int y, int r, int g, int b)
{
    // calculate the pixel's byte offset inside the buffer
    // note: x * 2 as every pixel is 2 consecutive bytes
    unsigned int pix_offset = x * 2 + y * finfo.line_length;

    // now this is about the same as 'fbp[pix_offset] = value'
    // but a bit more complicated for RGB565
    //unsigned short c = ((r / 8) << 11) + ((g / 4) << 5) + (b / 8);
    unsigned short c = ((r / 8) * 2048) + ((g / 4) * 32) + (b / 8);
    // write 'two bytes at once'
    *((unsigned short*)(fbp + pix_offset)) = c;
}

void draw_pixel(int x, int y, COLOR rgb)
{
    if(display_active == 0) return;
    if(x >= (int)vinfo.xres) return;
    if(y >= (int)vinfo.yres) return;

    #ifdef FLIPVERT
    x = disp_width - x - 1;
    y = disp_height - y - 1;
    #endif

    if (vinfo.bits_per_pixel == 16) 
        put_pixel_RGB565(x, y, rgb.r, rgb.g, rgb.b);
    else
        put_pixel_RGB24(x, y, rgb.r, rgb.g, rgb.b);
}

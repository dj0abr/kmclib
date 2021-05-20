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

Drawing functions

*/

#include "../kmclib.h"
#include "display.h"
#include "tjpgd.h"

COLOR WHITE = {0xff,0xff,0xff};
COLOR BLACK = {0,0,0};
COLOR RED = {0xff,0,0};
COLOR GREEN = {0,0xff,0};
COLOR BLUE = {0,0,0xff};
COLOR YELLOW = {0xff,0xff,0};
COLOR GREY = {0x80,0x80,0x80};
COLOR DARKGREY = {0x40,0x40,0x40};

void TFT_Fill_Rectangle(int xs, int ys, int xe, int ye, COLOR rgb)
{
    for(int x=xs; x<xe; x++)
        for(int y=ys; y<ye; y++)
            draw_pixel(x,y,rgb);
}

void display_clear()
{
    // short white background, so we can see that the display is working
    TFT_Fill_Rectangle(0, 0, disp_width, disp_height, WHITE);
    usleep(100000);
    TFT_Fill_Rectangle(0, 0, disp_width, disp_height, BLACK);
}

void TFT_Hor_Line(int xs, int xe, int ypos, COLOR rgb, COLOR brgb, int linewidth, uint32_t pattern)
{
    int bit = 0;
    for(int x=xs; x<xe; x++)
    {
        for(int y=ypos; y<ypos+linewidth; y++)
        {
            if(pattern != 0)
            {
                if((pattern&(1<<bit)))
                    draw_pixel(x,y,rgb);
                else
                    draw_pixel(x,y,brgb);
            }
            else
                draw_pixel(x,y,rgb);
        }
        if(++bit >= 32) bit = 0;
    }
}

void TFT_Vert_Line(int xpos, int ys, int ye, COLOR rgb, COLOR brgb, int linewidth, uint32_t pattern)
{
    int bit = 0;
    for(int y=ys; y<ye; y++)
    {
        for(int x=xpos; x<xpos+linewidth; x++)
        {
            if(pattern != 0)
            {
                if((pattern&(1<<bit)))
                    draw_pixel(x,y,rgb);
                else
                    draw_pixel(x,y,brgb);
            }
            else
                draw_pixel(x,y,rgb);
        }
        if(++bit >= 32) bit = 0;
    }
}

void TFT_DrawLine(int x0,int y0,int x1,int y1, COLOR rgb, int linewidth)
{
int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
int err = (dx>dy ? dx : -dy)/2, e2;

	while(1)
	{
		if(linewidth == 1)
			draw_pixel(x0,y0,rgb);
		else
			TFT_Fill_Rectangle(x0, y0, x0+linewidth-1, y0+linewidth-1, rgb);

		if (x0==x1 && y0==y1) break;
		e2 = err;
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}

void TFT_Rectangle(int xs, int ys, int xe, int ye, COLOR rgb, int linewidth)
{
	TFT_Hor_Line(xs+linewidth,xe+1,ys,rgb, BLACK, linewidth,0);
	TFT_Hor_Line(xs+linewidth,xe+linewidth-1,ye,rgb, BLACK, linewidth,0);
	TFT_Vert_Line(xs,ys,ye+linewidth,rgb, BLACK, linewidth, 0);
	TFT_Vert_Line(xe,ys,ye+linewidth,rgb, BLACK, linewidth, 0);
}

void TFT_Rectangle_filled(int xs, int ys, int xe, int ye, COLOR rgb)
{
    for(int y=ys; y<ye; y++)
    {
        TFT_Hor_Line(xs,xe,y,rgb, BLACK, 1,0);    
    }
}

void TFT_drawcircle_int(int x0, int y0, int radius,COLOR rgb)
{
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y)
    {
    	draw_pixel(x0 + x, y0 + y, rgb);
    	draw_pixel(x0 + y, y0 + x, rgb);
    	draw_pixel(x0 - y, y0 + x, rgb);
    	draw_pixel(x0 - x, y0 + y, rgb);
    	draw_pixel(x0 - x, y0 - y, rgb);
    	draw_pixel(x0 - y, y0 - x, rgb);
    	draw_pixel(x0 + y, y0 - x, rgb);
    	draw_pixel(x0 + x, y0 - y, rgb);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0)
        {
            x--;
            dx += 2;
            err += (-radius << 1) + dx;
        }
    }
}

void TFT_drawcircle(int x0, int y0, int radius,COLOR rgb, int linewidth)
{
    for(int i=-linewidth/2; i<linewidth/2; i++)
    {
        int rad = radius + i;
        if(rad < 1) rad = 1;
        TFT_drawcircle_int(x0, y0, rad, rgb);
    }
}

void TFT_drawcircle_filled(int x0, int y0, int radius,COLOR rgb)
{
    for(int i=1; i<radius; i++)
    {
        TFT_drawcircle_int(x0, y0, i, rgb);
    }
}

// =================== JPG Functions =================

FILE *fppic;            // jpg-Image File pointer

unsigned int in_func (  /* Returns number of bytes read (zero on error) */
    JDEC* jd,           /* Decompression object */
    uint8_t* buff,      /* Pointer to the read buffer (null to remove data) */
    unsigned int nbyte  /* Number of bytes to read/remove */
)
{
    if (buff) 
        return (unsigned int)fread(buff, 1, nbyte, fppic);
    else 
        return fseek(fppic, nbyte, SEEK_CUR) ? 0 : nbyte;
}

int out_func (      /* 1:Ok, 0:Aborted */
    JDEC* jd,       /* Decompression object */
    void* bitmap,   /* Bitmap data to be output */
    JRECT* rect     /* Rectangular region of output image */
)
{
    uint8_t *pd = (uint8_t*)bitmap;
    for(int y=rect->top; y<=rect->bottom; y++)
    {
        for(int x=rect->left; x<=rect->right; x++)
        {
            COLOR col;
            col.r = *pd++;
            col.g = *pd++;
            col.b = *pd++;            
            draw_pixel(x,y,col);
        }
    }
    return 1;    // Continue to decompress 
}

void TFT_drawJPGfile(char *filename, int x, int y)
{
    JRESULT res;      /* Result code of TJpgDec API */
    JDEC jdec;        /* Decompression object */
    uint8_t work[3100];

    fppic = fopen(filename, "rb");
    if (!fppic) return;

    /* Prepare to decompress */
    res = jd_prepare(&jdec, in_func, work, 3100, &fppic);
    if (res != JDR_OK) 
    {
        printf("Failed to prepare. (rc=%d)\n", res);
        return;
    }

    // It is ready to dcompress and image info is available here
    //printf("Image size is %u x %u.\n%u bytes of work ares is used.\n", jdec.width, jdec.height, 3100 - jdec.sz_pool);

    // Start to decompress with 1/1 scaling
    res = jd_decomp(&jdec, out_func, 0);
    if (res != JDR_OK) 
        printf("Failed to decompress. (rc=%d)\n", res);
}

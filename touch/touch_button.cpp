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

creates and handles touch buttons

*
*/

#include "../ctlbrd.h"

void *tbutproc(void *pdata);

TOUCHBUTTON touchbut[MAXTOUCHBUTTONS];
int touchbutanz = 0;
int touchbutproc_running = 0;
pthread_t tbut_tid;

pthread_mutex_t     tbut_crit_sec;
#define LOCK	pthread_mutex_lock(&tbut_crit_sec)
#define UNLOCK	pthread_mutex_unlock(&tbut_crit_sec)

int create_touchbutton(TOUCHBUTTON *ptb)
{
    memcpy(&(touchbut[touchbutanz]), ptb, sizeof(TOUCHBUTTON));
    touchbut[touchbutanz].active = 0;

    if(touchbutproc_running == 0)
    {
        // open process only once
        int pret = pthread_create(&tbut_tid,NULL,tbutproc, NULL);
        if(pret)
        {
            printf("touch button process NOT started\n");
            return -1;
        }
        usleep(100000);
        touch_init();
        touchbutproc_running = 1;
    }

    int id = touchbutanz;
    LOCK;
    touchbutanz++;
    UNLOCK;
    return id;
}

void *tbutproc(void *pdata)
{
    TOUCHINFO tp;
    int lastbutton = -1;

    pthread_detach(pthread_self());

    while(keeprunning)
    {
        // read touch panel
        while(touch_read(&tp) == 0)
        {
            LOCK;
            int butanz = touchbutanz;
            UNLOCK;
            // find button within the touched coordinates
            for(int i=0; i<butanz; i++)
            {
                TOUCHBUTTON *b = &touchbut[i];
                if( tp.scaledX >= b->x && tp.scaledX <= (b->x + b->width) &&
                    tp.scaledY >= b->y && tp.scaledY <= (b->y + b->height) &&
                    b->menu == getActMenu())
                {
                    // button found, call user supplied function
                    if(i != lastbutton)
                    {
                        if(b->touchbutfunc == NULL)
                            printf("pressed button %d, no callback specified\n",i);
                        else if(b->active == 0)
                            printf("pressed button %d, button inactive\n",i);
                        else
                            (*b->touchbutfunc)(i);   
                    }
                    lastbutton = i;
                }
            }
        }
        usleep(100000); // sleep 100ms, nobody will press a button faster than that
    }

    printf("exit touch button thread\n");
    pthread_exit(NULL);
}

void draw_button(int id)
{
    TOUCHBUTTON *b = &touchbut[id];

    // full button
    TFT_Rectangle_filled(b->x,b->y,b->x+b->width,b->y+b->height,b->button_color);
    // outer frame
    TFT_Rectangle(b->x,b->y,b->x+b->width,b->y+b->height,WHITE,2); 
    // text
    draw_font(b->text, b->x + b->text_xoff, b->y+b->text_yoff,b->font,b->text_color, b->fontsize);

    b->active = 1;
}

void deactivate_allbuttons()
{
    for(int i=0; i<touchbutanz; i++)
        touchbut[i].active = 0;
}

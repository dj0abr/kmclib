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
drawfont.cpp
===========
draws ttf Texts

functions to convert TTF Font-Text into pixels written by:
https://github.com/kevinboone/fbtextdemo

*
*/
#include "../kmclib.h"
#include "display.h"
#include <freetype2/ft2build.h>
#include <freetype/freetype.h>


int init_ft (const char *ttf_file, FT_Face *face, FT_Library *ft, int req_size, char **error)
{
    int ret = 0;
    if (FT_Init_FreeType (ft) == 0) 
    {
        if (FT_New_Face(*ft, ttf_file, 0, face) == 0)
        {
            // Note -- req_size is a request, not an instruction
            if (FT_Set_Pixel_Sizes(*face, 0, req_size) == 0)
                ret = 1;
            else
                if (error) *error = strdup ("Can't set font size");
        }
        else
            if (error) *error = strdup ("Can't load TTF file");
    }
    else
        if (error) *error = strdup ("Can't init freetype library");

    return ret;
}

void done_ft (FT_Library ft)
{
    FT_Done_FreeType (ft);
}

int face_get_line_spacing (FT_Face face)
{
    return face->size->metrics.height / 64;
}

void face_get_char_extent (const FT_Face face, int c, int *x, int *y)
{
    FT_UInt gi = FT_Get_Char_Index(face, c);

    FT_Load_Glyph (face, gi, FT_LOAD_NO_BITMAP);

    *y = face_get_line_spacing (face);
    *x = face->glyph->metrics.horiAdvance / 64;
}

void face_get_string_extent (const FT_Face face, const UTF32 *s, int *x, int *y)
{
    *x = 0;
    int y_extent = 0;
    while (*s)
    {
        int x_extent;
        face_get_char_extent (face, *s, &x_extent, &y_extent);
        *x += x_extent;
        s++;
    }
    *y = y_extent;
}

UTF32 *utf8_to_utf32 (const UTF8 *word)
{
    if(word == NULL) return 0;
    int l = strlen ((char *)word);
    UTF32 *ret = (UTF32 *)malloc ((l + 1) * sizeof (UTF32));

    for (int i = 0; i < l; i++)
        ret[i] = (UTF32) word[i];

    ret[l] = 0;
    return ret;
}

void face_draw_char_on_fb (FT_Face face, int c, int *x, int y, COLOR rgb)
  {
  FT_UInt gi = FT_Get_Char_Index (face, c);
  FT_Load_Glyph (face, gi, FT_LOAD_DEFAULT);
  int bbox_ymax = face->bbox.yMax / 64;
  int y_off = bbox_ymax - face->glyph->metrics.horiBearingY / 64;
  int glyph_width = face->glyph->metrics.width / 64;
  int advance = face->glyph->metrics.horiAdvance / 64;
  int x_off = (advance - glyph_width) / 2;
  FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

    for (int i = 0; i < (int)face->glyph->bitmap.rows; i++)
    {
        int row_offset = y + i + y_off;

        for (int j = 0; j < (int)face->glyph->bitmap.width; j++)
        {
            unsigned char p =face->glyph->bitmap.buffer [i * face->glyph->bitmap.pitch + j];
            if (p)
            {
                COLOR col;
                col.r = (p * rgb.r)/256;
                col.g = (p * rgb.g)/256;
                col.b = (p * rgb.b)/256;
                draw_pixel(*x + j + x_off, row_offset, col);
            }
        }
    }
    *x += advance;
}

void face_draw_string_on_fb (FT_Face face, const UTF32 *s, int *x, int y, COLOR rgb)
{
    while (*s)
    {
        face_draw_char_on_fb (face, *s, x, y, rgb);
        s++;
    }
}

#define MAXFONTS 12
char fontfile[MAXFONTS][256] = {
    {"/usr/share/fonts/truetype/msttcorefonts/Verdana.ttf"},            
    {"/usr/share/fonts/truetype/msttcorefonts/Verdana_Bold.ttf"},       
    {"/usr/share/fonts/truetype/msttcorefonts/Verdana_Italic.ttf"},
    {"/usr/share/fonts/truetype/msttcorefonts/Verdana_Bold_Italic.ttf"},

    {"/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf"},
    {"/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf"},
    {"/usr/share/fonts/truetype/liberation/LiberationMono-Italic.ttf"},
    {"/usr/share/fonts/truetype/liberation/LiberationMono-BoldItalic.ttf"},

    {"/usr/share/fonts/truetype/msttcorefonts/Courier_New.ttf"},
    {"/usr/share/fonts/truetype/msttcorefonts/Courier_New_Bold.ttf"},
    {"/usr/share/fonts/truetype/msttcorefonts/Courier_New_Italic.ttf"},
    {"/usr/share/fonts/truetype/msttcorefonts/Courier_New_Bold_Italic.ttf"},
};

void draw_font(char *word, int init_x, int init_y, int fontidx, COLOR rgb, int font_size)
{
    // A string representating a single space in UTF32 format
    static const UTF32 utf32_space[2] = {' ', 0};

    char *ttf_file = fontfile[fontidx];
    int height = 100;

    FT_Face face;
	FT_Library ft;
    char *error = NULL;

	if (init_ft(ttf_file, &face, &ft, font_size, &error))
    {
        int line_spacing = face_get_line_spacing (face);

        UTF32 *word32 = utf8_to_utf32 ((UTF8 *)word);
        int x = init_x; 
        int y = init_y;

        // If we're already below the specified height, don't write anything
        if (y + line_spacing < init_y + height)
        {
            face_draw_string_on_fb (face, word32, &x, y, rgb);
            face_draw_string_on_fb (face, utf32_space, &x, y, rgb);
        }
        free (word32);

        done_ft (ft);
    }
	else
	{
	  printf("TTF-Font error: %s\n", error);
	  if(error) free(error);
	}
}

/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999, 2000, 2001 Simon Peter, <dn.tlp@gmx.net>, et al.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * txtgfx.h - Textmode Graphics Library, by Simon Peter (dn.tlp@gmx.net)
 *
 * This library offers commonly used textmode BIOS and hardware graphics
 * functions.
 */

#ifndef H_TXTGFX
#define H_TXTGFX

#ifdef __cplusplus
extern "C" {
#endif

void clearscreen(unsigned char col);
/* clear screen with given color and reset cursor position
 *
 * col	= color to reset screen with
 */

unsigned char getvideomode(void);
/* Get BIOS video mode
 *
 * Returns:
 * Video mode number as reported by BIOS.
 */

void setvideomode(unsigned char mode);
/* set BIOS video mode
 *
 * mode	= BIOS video mode number
 */

void load88font(void);
/* load & activate VGA 8x8 font (enables 50 rows) */

void showcursor(void);
/* show hardware cursor on screen */

void hidecursor(void);
/* hide hardware cursor */

void setcolor(unsigned char newcol);
/* set text output color
 *
 * newcol	= BIOS color number
 */

void settextposition(unsigned char ypos, unsigned char xpos);
/* set cursor position
 *
 * ypos	= Cursor Y-Position
 * xpos	= Cursor X-Position
 */

void outchar(char c);
/* write a char directly into video memory
 *
 * c	= character to write to video memory
 */

void outtext(char *str);
/* write a string directly into video memory
 *
 * str	= string to write to video memory
 */

#ifdef __cplusplus
}
#endif

#endif

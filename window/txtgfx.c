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
 * txtgfx.c - Textmode Graphics Library, by Simon Peter (dn.tlp@gmx.net)
 */

#include <i86.h>
#include <string.h>

static unsigned char curcol=7;			/* current color */
static unsigned char scrsizex=0,scrsizey=0;	/* screen resolution */
static unsigned int scrpos=0;				/* current cursor position */
static unsigned char *vptr=0xb8000;			/* pointer to textmode vidmem */

void clearscreen(unsigned char col)
{
	unsigned int i;

	for(i=0;i<scrsizex*scrsizey;i++) {
		vptr[i*2] = 0;
		vptr[i*2+1] = col;
	}
	scrpos = 0;
}

unsigned char getvideomode(void)
{
	union REGS regs;

	regs.h.ah = 0xf;
	int386(0x10, &regs, &regs);
	return regs.h.al;
}

void setvideomode(unsigned char mode)
{
	union REGS regs;

	regs.h.ah = 0;
	regs.h.al = mode;
	int386(0x10, &regs, &regs);

	switch(mode) {
	case 0:
	case 1:
		scrsizex = 40;
		scrsizey = 25;
		break;
	case 2:
	case 3:
	case 7:
		scrsizex = 80;
		scrsizey = 25;
		break;
	default:
		scrsizex = 0;
		scrsizey = 0;
		break;
	}
}

void load88font(void)
{
	union REGS regs;
	static char font = 0;

	regs.h.ah = 0x11;
	regs.h.al = 0x12;
	regs.h.bl = 0;
	int386(0x10, &regs, &regs);

	if(!font) {
		scrsizey *= 2;
		font = 1;
	}
}

void showcursor(void)
{
	union REGS regs;

	regs.h.ah = 1;
	regs.h.ch = 6;
	regs.h.cl = 7;
	int386(0x10, &regs, &regs);
}

void hidecursor(void)
{
	union REGS regs;

	regs.h.ah = 1;
	regs.w.cx = 0xffff;
	int386(0x10, &regs, &regs);
}

void setcolor(unsigned char newcol)
{
	curcol = newcol;
}

void settextposition(unsigned char ypos, unsigned char xpos)
{
	scrpos = ypos*scrsizex+xpos;
}

void outchar(char c)
{
	vptr[scrpos*2] = c;
	vptr[scrpos*2+1] = curcol;
	scrpos++;
}

void outtext(char *str)
{
	unsigned int i;

	for(i=0;i<strlen(str);i++)
		outchar(str[i]);
}

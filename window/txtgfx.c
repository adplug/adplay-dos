/*
 * txtgfx.c - Textmode Graphics Library, by Simon Peter (dn.tlp@gmx.net)
 */

#include <i86.h>
#include <string.h>

static unsigned char curcol=7;			/* current color */
static unsigned char scrsizex=0,scrsizey=0;	/* screen resolution */
static unsigned int scrpos=0;				/* current cursor position */
static unsigned char *vptr = 0xb8000;		/* pointer to textmode vidmem */

void clearscreen(unsigned char col)
/* clear screen with given color and reset cursor position */
{
	unsigned int i;

	for(i=0;i<scrsizex*scrsizey;i++) {
		vptr[i*2] = 0;
		vptr[i*2+1] = col;
	}
	scrpos = 0;
}

void setvideomode(unsigned char mode)
/* set BIOS video mode */
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
/* load & activate 8x8 font */
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
/* show hardware cursor on screen */
{
	union REGS regs;

	regs.h.ah = 1;
	regs.h.ch = 6;
	regs.h.cl = 7;
	int386(0x10, &regs, &regs);
}

void hidecursor(void)
/* hide hardware cursor */
{
	union REGS regs;

	regs.h.ah = 1;
	regs.w.cx = 0xffff;
	int386(0x10, &regs, &regs);
}

void setcolor(unsigned char newcol)
/* set output color */
{
	curcol = newcol;
}

void settextposition(unsigned char ypos, unsigned char xpos)
/* set cursor position */
{
	scrpos = ypos*scrsizex+xpos;
}

void outchar(char c)
/* write c directly into video memory */
{
	vptr[scrpos*2] = c;
	vptr[scrpos*2+1] = curcol;
	scrpos++;
}

void outtext(char *str)
/* write str directly into video memory */
{
	unsigned int i;

	for(i=0;i<strlen(str);i++)
		outchar(str[i]);
}

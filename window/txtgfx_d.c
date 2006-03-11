/*
 * txtgfx.c - Textmode Graphics Library, by Simon Peter <dn.tlp@gmx.net>
 *
 * DJGPP backend
 */

#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include <go32.h>
#include <sys/farptr.h>

#include "txtgfx.h"

#define VIDMEM		0xb8000		/* pointer to textmode vidmem */
#define SCRSIZEX	_farpeekw(_dos_ds, 0x44a)
#define SCRSIZEY	_farpeekb(_dos_ds, 0x484)

static unsigned char curcol=7;			/* current color */
static unsigned int scrpos=0;                   /* current cursor position */
static VideoInfo *vidinfo=0;                    /* Holds video infos */

void clearscreen(unsigned char col)
{
	unsigned int i;
	unsigned char scrsizey = SCRSIZEY;
	unsigned short scrsizex = SCRSIZEX;

	_farsetsel(_dos_ds);
        for(i=0;i<scrsizex*scrsizey;i++) {
	        _farnspokeb(VIDMEM + i * 2, 0);
		_farnspokeb(VIDMEM + i * 2 + 1, col);
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
}

void load88font(void)
{
	union REGS regs;

	regs.h.ah = 0x11;
	regs.h.al = 0x12;
	regs.h.bl = 0;
	int386(0x10, &regs, &regs);
}

void load816font(void)
{
	union REGS regs;

	regs.h.ah = 0x11;
        regs.h.al = 0x14;
	regs.h.bl = 0;
	int386(0x10, &regs, &regs);
}

void setcursorsize(unsigned char start, unsigned char end)
{
	union REGS regs;

	regs.h.ah = 1;
        regs.h.ch = start;
        regs.h.cl = end;
	int386(0x10, &regs, &regs);
}

void getcursorsize(unsigned char *start, unsigned char *end)
{
	union REGS regs;

        regs.h.ah = 3;
        regs.h.bh = 0;
	int386(0x10, &regs, &regs);
        *start = regs.h.ch;
        *end = regs.h.cl;
}

void showcursor(void)
{
        setcursorsize(6,7);
}

void hidecursor(void)
{
        setcursorsize(0xff,0xff);
}

void setcolor(unsigned char newcol)
{
	curcol = newcol;
}

void settextposition(unsigned char ypos, unsigned char xpos)
{
        scrpos = ypos*SCRSIZEX+xpos;
}

void outchar(char c)
{
        _farpokeb(_dos_ds, VIDMEM + scrpos * 2, c);
	_farpokeb(_dos_ds, VIDMEM + scrpos * 2 + 1, curcol);
	scrpos++;
}

void outtext(char *str)
{
	unsigned int i;

	for(i=0;i<strlen(str);i++)
		outchar(str[i]);
}

VideoInfo *getvideoinfo(VideoInfo *vi)
{
        vi->mode = getvideomode();

	_farsetsel(_dos_ds);
        vi->cols = _farnspeekw(0x44a);
        vi->rows = _farnspeekb(0x484);
        vi->font = _farnspeekw(0x485);

        getcursorsize(&vi->curstart,&vi->curend);

        return vi;
}

void setvideoinfo(VideoInfo *vi)
{
        setvideomode(vi->mode);
        if(vi->font == 8) load88font();
        if(vi->font == 16) load816font();
        setcursorsize(vi->curstart,vi->curend);
}

void save_video(void)
{
        if(!vidinfo) vidinfo = (VideoInfo *)malloc(sizeof(VideoInfo));
        vidinfo = getvideoinfo(vidinfo);
}

void restore_video(void)
{
        if(vidinfo) {
                setvideoinfo(vidinfo);
                free(vidinfo);
        }
}

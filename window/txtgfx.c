/*
 * txtgfx.c - Textmode Graphics Library, by Simon Peter (dn.tlp@gmx.net)
 */

#include <i86.h>
#include <string.h>
#include <stdlib.h>

#include "txtgfx.h"

static unsigned char curcol=7;			/* current color */
static unsigned char *scrsizey=0x484;           /* # of rows */
static unsigned short *scrsizex=0x44a;          /* # of columns */
static unsigned int scrpos=0;                   /* current cursor position */
static unsigned char *vptr=0xb8000;             /* pointer to textmode vidmem */
static VideoInfo *vidinfo=0;                    /* Holds video infos */

void clearscreen(unsigned char col)
{
	unsigned int i;

        for(i=0;i<(*scrsizex)*(*scrsizey);i++) {
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
        scrpos = ypos*(*scrsizex)+xpos;
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

VideoInfo *getvideoinfo(VideoInfo *vi)
{
        vi->mode = getvideomode();
        vi->cols = *(unsigned short *)0x44a;
        vi->rows = *(unsigned char *)0x484;
        vi->font = *(unsigned short *)0x485;
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

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
 * window.h - Textmode window library, by Simon Peter (dn.tlp@gmx.net)
 */

#ifndef H_WINDOW_DEFINED
#define H_WINDOW_DEFINED

#define MAXSTRING		1024
#define MAXITEMS		100
#define MAXITEMSTRING	100
#define MAXCOLORS		3
#define DEFTXTBUFSIZE	1024

class CWindow
{
public:
	enum Color {Border, In, Caption};

	CWindow();
	~CWindow();

	void setcaption(char *newcap);				// sets new window caption
	char *getcaption();						// returns current window caption

	void out_setcolor(Color c, unsigned char v);		// sets window color
	unsigned char out_getcolor(Color c);			// returns window color

	void setxy(unsigned char newx, unsigned char newy);	// sets new on-screen x/y position
	unsigned char posx()
	{ return x; };
	unsigned char posy()
	{ return y; };

	void resize(unsigned char newx, unsigned char newy);	// resizes the window
	unsigned char getsizex()
	{ return sizex; };
	unsigned char getsizey()
	{ return sizey; };

	void redraw();							// redraws the window on screen
	virtual void update()
	{ };

protected:
	// tools
	void puts(char *str);						// like puts(), but in the window
	void outtext(char *str);					// outputs text, but does no linefeed
	void outc(char c);						// outputs the char c

	void setcursor(unsigned int newx, unsigned int newy)	// set window-cursor to new position
	{ curpos = newy*insizex+newx; };
	unsigned int wherex()						// returns cursor x-position
	{ return (curpos % insizex); };
	unsigned int wherey()						// returns cursor y-position
	{ return (curpos / insizex); };
	unsigned int getcursor()					// returns absolute cursor position inside buffer
	{ return curpos; };

	void clear();							// clears the window and resets cursor position

	char *wndbuf;							// inner-window text buffer
	unsigned char *colmap;						// inner-window color map
	unsigned char insizex,insizey;				// inner-window sizes
	bool autocolor;							// automatically colorize output flag

private:
	// positions, sizes & colors
	unsigned char x,y,sizex,sizey;				// window position and size
	unsigned int curpos;						// cursor position inside window text buffer
	unsigned char color[MAXCOLORS];

	// buffers
	char *caption;							// window caption
};

class CTxtWnd: public CWindow
{
public:
	CTxtWnd();
	~CTxtWnd()
	{ delete [] txtbuf; };

	void outtext(const char *str);				// outputs text, but does no linefeed
	void puts(const char *str)					// like puts(), but in the window
	{ outtext(str); outtext("\n"); };

	void scroll_set(unsigned int ns)
	{ start = ns * insizex; };
	void scroll_down(unsigned int amount = 1);
	void scroll_up(unsigned int amount = 1);

	void erase();							// clears text buffer
	void update();

private:
	unsigned int txtpos,bufsize,start;
	char *txtbuf;
};

class CListWnd: public CWindow
{
public:
	CListWnd();

	unsigned int additem(char *str);
	void removeitem(unsigned int nr);
	void removeall();
	char *getitem(unsigned int nr);

	void selectitem(unsigned int nr)
	{ selected = nr; };
	void select_next();
	void select_prev();
	unsigned int getselection()
	{ return selected; };

	void scroll_set(unsigned int sc)
	{ start = sc; };
	void scroll_down()
	{ if(start+insizey < numitems) start++; };
	void scroll_up()
	{ if(start) start--; };

	enum LColor {Select, Unselect};

	void setcolor(LColor c, unsigned char v);			// sets window color
	unsigned char getcolor(LColor c);				// returns window color

	void update();

private:
	char item[MAXITEMS][MAXITEMSTRING];
	bool useitem[MAXITEMS];

	unsigned int selected,start,numitems;
	unsigned char selcol,unselcol;
};

class CBarWnd: public CWindow
{
public:
	CBarWnd(unsigned int n, unsigned int nmax);
	~CBarWnd()
	{ delete [] bars; };

	enum BColor {Bar, Clip};

	void setcolor(BColor c, unsigned char v);			// sets window color
	unsigned char getcolor(BColor c);				// returns window color

	void set(unsigned int v, unsigned int n = 0);
	unsigned int get(unsigned int n = 0)
	{ return bars[n]; };

	void update();

private:
	unsigned char 	barcol,clipcol;
	unsigned int	*bars,nbars,max;
};

#endif

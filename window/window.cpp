/*
 * window.cpp - Basic Textmode Windows, by Simon Peter (dn.tlp@gmx.net)
 */

#include <stdio.h>
#include <string.h>

#include <conio.h>

//#include <stdarg.h>

#include "txtgfx.h"
#include "window.h"

CWindow::CWindow(unsigned char nsizex, unsigned char nsizey, unsigned char nx, unsigned char ny)
	: x(nx), y(ny), visible(true), refreshbb(false), curpos(0), autocolor(true), backbuf(0), colmap(0), wndbuf(0)
{
	strncpy(caption,"Window",MAXCAPTION);
	memset(color,7,MAXCOLORS);
	resize(nsizex,nsizey);
}

CWindow::~CWindow(void)
{
	delete [] colmap;
	delete [] wndbuf;
	delete [] backbuf;
}

void CWindow::resize(unsigned char newx, unsigned char newy)
{
	insizex = newx - 2; insizey = newy - 2;
	sizex = newx; sizey = newy;
	if(backbuf) delete [] backbuf;
	if(wndbuf) delete [] wndbuf;
	if(colmap) delete [] colmap;
	backbuf = new char [newx*newy];
	wndbuf = new char [insizex*insizey];
	colmap = new unsigned char [insizex*insizey];
	memset(wndbuf,' ',insizex*insizey);
	memset(colmap,color[In],insizex*insizey);
}

void CWindow::redraw(void)
{
	unsigned char i,j,wndx,wndy=0;

	if(visible) {	// draw window contents
		settextposition(y,x);
		::setcolor(color[Border]);
		outchar('Ú');
		for(i=x+1;i<x+((sizex-1)/2-(strlen(caption)/2+2));i++)
			outchar('Ä');
		::outtext("> ");
		::setcolor(color[Caption]);
		::outtext(caption);
		::setcolor(color[Border]);
		::outtext(" <");
		for(i+=strlen(caption)+4;i<x+sizex-1;i++)
			outchar('Ä');
		outchar('¿');
		for(j=y+1;j<y+sizey-1;j++) {
			settextposition(j,x);
			wndx = 0;
			for(i=x;i<x+sizex;i++)
				if(i==x || i==x+sizex-1)
					outchar('³');
				else {
					::setcolor(colmap[wndy*insizex+wndx]);
					outchar(wndbuf[wndy*insizex+wndx]);
					::setcolor(color[Border]);
					wndx++;
				}
			wndy++;
		}
		settextposition(y+sizey-1,x);
		outchar('À');
		for(i=x+1;i<x+sizex-1;i++)
			outchar('Ä');
		outchar('Ù');
	} else		// draw back buffer
		for(j=y;j<y+sizey;j++)
			for(i=x;i<x+sizex;i++) {
				settextposition(j,i);
				outchar(backbuf[j*sizex+i]);
			}
}

void CWindow::outc(char c)
{
	if(curpos >= insizex*insizey)
		return;

	if(c != '\n') {
		wndbuf[curpos] = c;
		if(autocolor)
			colmap[curpos] = color[In];
		curpos++;
	} else
		setcursor(0,wherey()+1);
}

void CWindow::outtext(char *str)
{
	unsigned int i;

	for(i=0;i<strlen(str) && curpos < insizex*insizey;i++)
		outc(str[i]);
}

void CWindow::puts(char *str)
{
	outtext(str);
	setcursor(0,wherey()+1);
}

void CWindow::clear()
{
	memset(colmap,In,insizex*insizey);
	memset(wndbuf,' ',insizex*insizey);
	setcursor(0,0);
}

CTxtWnd::CTxtWnd(unsigned char nsizex, unsigned char nsizey, unsigned char nx, unsigned char ny)
	: CWindow(nsizex, nsizey, nx, ny), bufsize(DEFTXTBUFSIZE)
{
	txtbuf = new char [DEFTXTBUFSIZE];
	clear();
}

void CTxtWnd::clear()
{
	memset(txtbuf,' ',bufsize);
	txtpos = 0;
	start = 0;
}

void CTxtWnd::outtext(const char *str)
{
	unsigned int i;

	if(txtpos+strlen(str) >= bufsize) {	// resize buffer
		char *newbuf = new char[txtpos+strlen(str)];
		memcpy(newbuf,txtbuf,bufsize);
		delete [] txtbuf;
		txtbuf = newbuf;
		bufsize = txtpos+strlen(str);
	}

	for(i=0;i<strlen(str);i++) {
		txtbuf[txtpos] = str[i];
		txtpos++;
	}
}

void CTxtWnd::redraw()
{
	CWindow::clear();
	CWindow::outtext(txtbuf+start);
	CWindow::redraw();
}

/*void CTxtWnd::printf(char *format, ...)
{
	va_list	ap;
	char		bigstr[MAXSTRING];

	sprintf(bigstr,format,ap);
	puts(bigstr);
} */

void CTxtWnd::scroll_down(unsigned int amount)
{
	unsigned int i,delta;

	for(i=0;i<amount;i++) {
		delta = strchr(txtbuf+start,'\n')-(txtbuf+start)+1;
		if(delta > insizex)
			start += insizex;
		else
			start += delta;
		if(start >= bufsize) start = bufsize-1;
	}
}

void CTxtWnd::scroll_up(unsigned int amount)
{
	unsigned int	i,delta;
	char			*ptr;

	for(i=0;i<amount;i++) {
		for(ptr=txtbuf+start-2;ptr>=txtbuf && *ptr != '\n';ptr--);
		delta = txtbuf+start-ptr-1;
		if(ptr <= txtbuf) {
			delta = 0;
			start = 0;
		}
		if(delta > insizex)
			start -= insizex;
		else
			start -= delta;
	}
}

CListWnd::CListWnd(unsigned char nsizex, unsigned char nsizey, unsigned char nx, unsigned char ny)
	: CWindow(nsizex, nsizey, nx, ny), selcol(0x70), unselcol(7)
{
	removeall();
}

unsigned int CListWnd::additem(char *str)
{
	unsigned int i;

	for(i=0;i<MAXITEMS;i++)
		if(!useitem[i]) {
			strcpy(item[i],str);
			useitem[i] = true;
			numitems++;
			return i;
		}

	return (MAXITEMS + 1);
}

void CListWnd::removeitem(unsigned int nr)
{
	if(useitem[nr]) {
		useitem[nr] = false;
		numitems--;
	}
}

char *CListWnd::getitem(unsigned int nr)
{
	if(useitem[nr])
		return item[nr];
	else
		return 0;
}

void CListWnd::redraw()
{
	unsigned int i;

	clear();
	for(i=start;(i<start+insizey) && (i<numitems);i++)
		if(useitem[i]) {
			if(i == selected) {
				memset(colmap+getcursor(),selcol,insizex);
				setcolor(In,selcol);
			} else {
				memset(colmap+getcursor(),unselcol,insizex);
				setcolor(In,unselcol);
			}
			puts(item[i]);
		}

	CWindow::redraw();
}

void CListWnd::select_next()
{
	if(selected + 1 < numitems)
		selected++;

	if(selected >= start + insizey)
		scroll_down();
}

void CListWnd::select_prev()
{
	if(selected)
		selected--;

	if(selected < start)
		start = selected;
}

void CListWnd::removeall()
{
	for(unsigned int i=0;i<MAXITEMS;i++)
		useitem[i] = false;
	selected = 0;
	start = 0;
	numitems = 0;
}

void CListWnd::setcolor(Color c, unsigned char v)
{
	switch(c) {
	case Select:
		selcol = v;
		break;
	case Unselect:
		unselcol = v;
		break;
	default:
		CWindow::setcolor((CWindow::Color)c,v);
		break;
	}
}

unsigned char CListWnd::getcolor(Color c)
{
	switch(c) {
	case Select:
		return selcol;
	case Unselect:
		return unselcol;
	default:
		return CWindow::getcolor((CWindow::Color)c);
	}
}

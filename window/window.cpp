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
 * window.cpp - Textmode window library, by Simon Peter (dn.tlp@gmx.net)
 */

#include <stdio.h>
#include <string.h>

#include "txtgfx.h"
#include "window.h"

#define DEFWNDSIZEX	20
#define DEFWNDSIZEY	20
#define DEFWNDPOSX	0
#define DEFWNDPOSY	0

CWindow::CWindow(): x(DEFWNDPOSX), y(DEFWNDPOSY), curpos(0), autocolor(true), colmap(0), wndbuf(0), caption(0)
{
	setcaption("Window");
	memset(color,7,MAXCOLORS);
	resize(DEFWNDSIZEX,DEFWNDSIZEY);
}

CWindow::~CWindow(void)
{
	delete [] colmap;
	delete [] wndbuf;
}

void CWindow::setcaption(char *newcap)
{
	if(caption)
		delete [] caption;

	caption = new char [strlen(newcap)+1];
	strcpy(caption,newcap);
}

char *CWindow::getcaption()
{
	return caption;
}

void CWindow::setxy(unsigned char newx, unsigned char newy)
{
	x = newx;
	y = newy;
}

void CWindow::out_setcolor(Color c, unsigned char v)
{
	color[c] = v;
}

unsigned char CWindow::out_getcolor(Color c)
{
	return color[c];
}

void CWindow::resize(unsigned char newx, unsigned char newy)
{
	insizex = newx - 2; insizey = newy - 2;
	sizex = newx; sizey = newy;
	if(wndbuf) delete [] wndbuf;
	if(colmap) delete [] colmap;
	wndbuf = new char [insizex*insizey];
	colmap = new unsigned char [insizex*insizey];
	memset(wndbuf,' ',insizex*insizey);
	memset(colmap,color[In],insizex*insizey);
}

void CWindow::redraw(void)
{
	unsigned char i,j,wndx,wndy=0;

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
			if(i==x || i==x+sizex-1) {
				::setcolor(color[Border]);
				outchar('³');
			} else {
				::setcolor(colmap[wndy*insizex+wndx]);
				outchar(wndbuf[wndy*insizex+wndx]);
				wndx++;
			}
		wndy++;
	}
	::setcolor(color[Border]);
	settextposition(y+sizey-1,x);
	outchar('À');
	for(i=x+1;i<x+sizex-1;i++)
		outchar('Ä');
	outchar('Ù');
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
	memset(colmap,color[In],insizex*insizey);
	memset(wndbuf,' ',insizex*insizey);
	setcursor(0,0);
}

CTxtWnd::CTxtWnd()
	: CWindow(), bufsize(DEFTXTBUFSIZE)
{
	txtbuf = new char [DEFTXTBUFSIZE+1];
	txtbuf[DEFTXTBUFSIZE] = '\0';
	erase();
}

void CTxtWnd::erase()
{
	memset(txtbuf,'\0',bufsize);
	txtpos = 0;
	start = 0;
}

void CTxtWnd::outtext(const char *str)
{
	unsigned int i;

	if(txtpos+strlen(str) >= bufsize) {	// resize buffer
		char *newbuf = new char[txtpos+strlen(str)+1];
		newbuf[txtpos+strlen(str)] = '\0';
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

void CTxtWnd::update()
{
	clear();
	CWindow::outtext(txtbuf+start);
	redraw();
}

void CTxtWnd::scroll_down(unsigned int amount)
{
	unsigned int i,delta;

	for(i=0;i<amount;i++) {
		delta = strchr(txtbuf+start,'\n')-(txtbuf+start)+1;
		if(delta > insizex)
			start += insizex;
		else
			start += delta;
		if(start >= strlen(txtbuf)) start = strlen(txtbuf)-1;
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

CListWnd::CListWnd()
	: CWindow(), selcol(0x70), unselcol(7)
{
	removeall();
	autocolor = false;
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

void CListWnd::update()
{
	unsigned int i;

	clear();
	for(i=start;(i<start+insizey) && (i<numitems);i++)
		if(useitem[i]) {
			if(i == selected)
				memset(colmap+getcursor(),selcol,insizex);
			else
				memset(colmap+getcursor(),unselcol,insizex);
			puts(item[i]);
		}
	redraw();
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

void CListWnd::setcolor(LColor c, unsigned char v)
{
	switch(c) {
	case Select:
		selcol = v;
		break;
	case Unselect:
		unselcol = v;
		break;
	}
}

unsigned char CListWnd::getcolor(LColor c)
{
	switch(c) {
	case Select:
		return selcol;
	case Unselect:
		return unselcol;
	default:
		return 0;
	}
}

CBarWnd::CBarWnd(unsigned int n, unsigned int nmax)
	: CWindow(), barcol(7), clipcol(4), nbars(n), max(nmax)
{
	unsigned int i;

	if(!n) return;
	bars = new unsigned int [n];
	for(i=0;i<n;i++)
		bars[i] = 0;
	autocolor = false;
}

void CBarWnd::update()
{
	unsigned int i;

	for(i=0;i<insizex*insizey;i++)
		if(i<(insizey/4)*insizex)
			colmap[i] = clipcol;
		else
			colmap[i] = barcol;
	redraw();
}

void CBarWnd::set(unsigned int v, unsigned int n)
{
	unsigned int i,j,k;

	memset(wndbuf,' ',insizex*insizey);
	bars[n] = v;
	for(i=0;i<nbars;i++)
		for(j=0;j<=(insizey*bars[i])/max;j++) {
			setcursor((insizex*i)/nbars,insizey-j);
			for(k=0;k<insizex/nbars;k++)
				outc('=');
		}
}

void CBarWnd::setcolor(BColor c, unsigned char v)
{
	switch(c) {
	case Bar:
		barcol = v;
		break;
	case Clip:
		clipcol = v;
		break;
	}
}

unsigned char CBarWnd::getcolor(BColor c)
{
	switch(c) {
	case Bar:
		return barcol;
	case Clip:
		return clipcol;
	default:
		return 0;
	}
}

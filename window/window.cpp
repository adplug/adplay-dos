/*
 * window.cpp - Textmode window library
 * Copyright (c) 2001, 2002 Simon Peter <dn.tlp@gmx.net>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "txtgfx.h"
#include "window.h"

#define DEBUG_FILE      "window.log"

// Defaults
#define DEFWNDSIZEX	20
#define DEFWNDSIZEY	20
#define DEFWNDPOSX	0
#define DEFWNDPOSY	0
#define DEFTXTBUFSIZE	1024
#define MAXCOLORS       8

// Static variable initialization
static unsigned char CWindow::color[MAXCOLORS] = {7, 7, 7, 7, 0x70, 7, 4, 4};
static CWindow *CWindow::focus = 0;

/***** CWindow *****/

CWindow::CWindow(): x(DEFWNDPOSX), y(DEFWNDPOSY), curpos(0), autocolor(true),
        colmap(0), wndbuf(0), caption(0)
{
        if(!focus) {    // First created window?
                setfocus();                     // Set focus here!
                Window_LogFile(DEBUG_FILE);     // Start debug logging
        }
	setcaption("Window");
	resize(DEFWNDSIZEX,DEFWNDSIZEY);
}

CWindow::~CWindow(void)
{
	delete [] colmap;
	delete [] wndbuf;
        if(caption) delete [] caption;
}

void CWindow::setcaption(const char *newcap)
{
        if(caption) delete [] caption;
	caption = new char [strlen(newcap)+1];
	strcpy(caption,newcap);
}

char *CWindow::getcaption()
{
	return caption;
}

void CWindow::center()
{
        VideoInfo *vi = (VideoInfo *)malloc(sizeof(VideoInfo));

        getvideoinfo(vi);
        setxy((vi->cols - getsizex()) / 2,(vi->rows - getsizey()) / 2);

        free(vi);
}

void CWindow::setxy(unsigned char newx, unsigned char newy)
{
	x = newx;
	y = newy;
}

static void CWindow::setcolor(Color c, unsigned char v)
{
        color[c] = v;
}

static unsigned char CWindow::getcolor(Color c)
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

void CWindow::setfocus()
{
        focus = this;
}

static CWindow *CWindow::getfocus()
{
        return focus;
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
        if(focus == this)
                ::setcolor(color[Focus]);
        else
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
                if(autocolor) colmap[curpos] = color[In];
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

/***** CTxtWnd *****/

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
        txtpos = start = 0;
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

void CTxtWnd::format(const char *str)
{
        unsigned int i, sx, lastmark = 0;
        char *tmpstr = new char [strlen(str)+1];

        strcpy(tmpstr,str);

        for(i=sx=0;i<strlen(tmpstr);i++,sx++)
                switch(tmpstr[i]) {
                case ' ': lastmark = i; break;
                case '\n': sx = 0; break;
                default:
                        if(sx >= getsizex()) {
                                tmpstr[lastmark] = '\n';
                                sx = i - lastmark;
                        }
                        break;
                }

        outtext(tmpstr);
        delete [] tmpstr;
}

void CTxtWnd::update()
{
	clear();
	CWindow::outtext(txtbuf+start);
	redraw();
}

bool CTxtWnd::scroll_set(unsigned int line)
{
        start = 0;
        return scroll_down(line);
}

bool CTxtWnd::scroll_down(unsigned int amount)
{
	unsigned int i,delta;

	for(i=0;i<amount;i++) {
		delta = strchr(txtbuf+start,'\n')-(txtbuf+start)+1;
		if(delta > insizex)
			start += insizex;
		else
			start += delta;
                if(start >= strlen(txtbuf)) {
                        start = strlen(txtbuf)-1;
                        return false;
                }
	}

        return true;
}

bool CTxtWnd::scroll_up(unsigned int amount)
{
	unsigned int	i,delta;
	char			*ptr;

	for(i=0;i<amount;i++) {
		for(ptr=txtbuf+start-2;ptr>=txtbuf && *ptr != '\n';ptr--);
		delta = txtbuf+start-ptr-1;
		if(ptr <= txtbuf) {
			delta = 0;
			start = 0;
                        return false;
		}
		if(delta > insizex)
			start -= insizex;
		else
			start -= delta;
	}

        return true;
}

/***** CListWnd *****/

CListWnd::CListWnd()
        : CWindow(), il(0)
{
	autocolor = false;
        removeall();
        Window_LogWrite("CListWnd::CListWnd(): created.\n");
}

CListWnd::~CListWnd()
{
        removeall();
        Window_LogWrite("CListWnd::~CListWnd(): destroyed.\n");
}

void CListWnd::additem(Item *newitem)
{
        ItemList *i = new ItemList;

        // Add item at beginning of list
        i->item = newitem; i->prev = 0; i->next = il;
        if(il) il->prev = i;
        il = start = selected = i;
}

CListWnd::Item *CListWnd::getitem(unsigned int nr)
{
        unsigned int i;
        ItemList *j = il;

        for(i=0;i<nr;i++)
                if(j) j = j->next; else return 0;

        return j->item;
}

void CListWnd::insertitem(Item *newitem, unsigned int nr)
{
        unsigned int i;
        ItemList *j = il, *k = new ItemList;

        for(i=0;i<nr;i++)
                if(j->next) j = j->next; else return;

        k->item = newitem; k->prev = j; k->next = j->next;
        j->next->prev = k; j->next = k;
}

void CListWnd::update()
{
        ItemList *i;
        unsigned int j = 0;

	clear();
        for(i=start;i && (wherey() < insizey);i=i->next) {
                if(i == selected)
                        memset(colmap+getcursor(),i->item->getcolor(Item::Selected),insizex);
                else
                        memset(colmap+getcursor(),i->item->getcolor(Item::Unselected),insizex);
                puts(i->item->gettext());
                j++;
        }

        Window_LogWrite("CListWnd::update(): %d items displayed.\n",j);
	redraw();
}

bool CListWnd::setselection(unsigned int nr)
{
        selected = start = il; selpos = 0;
        return select_next(nr);
}

bool CListWnd::select_next(unsigned int amount)
{
        unsigned int i;
        bool retval = true;

        for(i=0;i<amount;i++)
                if(selected->next) {
                        selected = selected->next;
                        selpos++;
                } else {
                        retval = false;
                        break;
                }

        // Scroll down window, if necessary
        while(selpos >= insizey) {
                scroll_down();
                selpos--;
        }

        return retval;
}

bool CListWnd::select_prev(unsigned int amount)
{
        unsigned int i;

        for(i=0;i<amount;i++)
                if(selected->prev) {
                        selected = selected->prev;
                        if(selpos) selpos--; else scroll_up();
                } else
                        return false;

        return true;
}

void CListWnd::scroll_down()
{
        if(start->next) start = start->next;
}

void CListWnd::scroll_up()
{
        if(start->prev) start = start->prev;
}

void CListWnd::removeall()
{
        ItemList *i;
        unsigned int j = 0;

        while(il) {
                i = il->next;
                delete il->item;
                delete il;
                il = i;
                j++;
        }

        selected = start = 0; selpos = 0;
        Window_LogWrite("CListWnd::removeall(): %d items deleted.\n",j);

}

/***** CListWnd::Item *****/

CListWnd::Item::Item(): text(0)
{
        color[Selected] = CWindow::getcolor(Select);
        color[Unselected] = CWindow::getcolor(Unselect);
}

CListWnd::Item::~Item()
{
        if(text) delete [] text;
}

void CListWnd::Item::settext(const char *str)
{
        text = new char[strlen(str)+1];
        strcpy(text,str);
}

void CListWnd::Item::setcolor(Color c, unsigned char v)
{
        color[c] = v;
}

/***** CBarWnd *****/

CBarWnd::CBarWnd(unsigned int n, unsigned int nmax)
        : CWindow(), nbars(n), max(nmax)
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
        unsigned int i,j,k;

        // set colormap
	for(i=0;i<insizex*insizey;i++)
		if(i<(insizey/4)*insizex)
                        colmap[i] = color[Clip];
		else
                        colmap[i] = color[Bar];

        // draw bars
	memset(wndbuf,' ',insizex*insizey);
	for(i=0;i<nbars;i++)
		for(j=0;j<=(insizey*bars[i])/max;j++) {
			setcursor((insizex*i)/nbars,insizey-j);
			for(k=0;k<insizex/nbars;k++)
				outc('=');
		}

	redraw();
}

void CBarWnd::set(unsigned int v, unsigned int n)
{
	bars[n] = v;
}

/***** CErrWnd *****/

static void CErrWnd::message(const char *errtxt, const char *caption)
{
        unsigned int rowln = 0, rows = 0, rwcnt = 0, i;
        CTxtWnd wnd;

        // Determine longest row and number of rows
        for(i=0;i<=strlen(errtxt);i++)
                if(errtxt[i] == '\n' || errtxt[i] == '\0') {
                        if(rwcnt > rowln)
                                rowln = rwcnt;
                        rwcnt = 0;
                        rows++;
                } else
                        rwcnt++;

        // Prepare a CTxtWnd with information
        wnd.setcaption(caption);
        wnd.resize(rowln + 2,rows + 2);
        wnd.center();
        wnd.format(errtxt);
        wnd.update();
}

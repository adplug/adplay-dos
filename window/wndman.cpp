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
 * wndman.cpp - Simple window manager, by Simon Peter (dn.tlp@gmx.net)
 */

#include "wndman.h"

CWndMan::CWndMan(): w(0)
{
}

CWndMan::~CWndMan()
{
	window *nextw;

	while(w) {
		nextw = w->next;
		delete w;
		w = nextw;
	}
}

bool CWndMan::reg(CWindow &nwnd)
{
	window *nw;

	if(!(&nwnd))
		return false;

	nw = new window;
	nw->wnd = &nwnd;
	nw->next = w;
	w = nw;
	return true;
}

void CWndMan::unreg(CWindow &nwnd)
{
	window *tw = w,*tw2;

	if(!w)
		return;

	if((&nwnd) == tw->wnd) {
		w = tw->next;
		delete tw;
		return;
	}

	while(tw->next) {
		if((&nwnd) == tw->next->wnd) {
			tw2 = tw->next;
			tw->next = tw->next->next;
			delete tw2;
		}
		tw = tw->next;
	}
}

void CWndMan::update()
{
	window *tw = w;

	while(tw) {
		tw->wnd->update();
		tw = tw->next;
	}
}

void CWndMan::setcolor(CWindow::Color c, unsigned char v)
{
	window *tw = w;

	while(tw) {
		tw->wnd->out_setcolor(c,v);
		tw = tw->next;
	}
}

/*
 * wndman.cpp - Simple window manager
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

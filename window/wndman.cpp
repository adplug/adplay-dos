/*
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

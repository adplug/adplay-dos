/*
 * wndman.h - Simple window manager, by Simon Peter (dn.tlp@gmx.net)
 */

#include "window.h"

class CWndMan
{
public:
	CWndMan();
	~CWndMan();

	bool reg(CWindow &nwnd);					// registers a window
	void unreg(CWindow &nwnd);					// unregisters a window

	void setcolor(CWindow::Color c, unsigned char v);	// sets all windows' colors
	void update();							// redraws all managed windows

private:
	struct window {
		CWindow	*wnd;
		window	*next;
	} *w;
};

/*
 * bars.h - Bars Window, by Simon Peter (dn.tlp@gmx.net)
 */

#include "window.h"
#include "analopl.h"

class CBarWnd: public CWindow
{
public:
	CBarWnd(unsigned int n, unsigned int nmax);
	~CBarWnd()
	{ delete [] bars; };

	void resize(unsigned char newx, unsigned char newy);	// resizes the window

	enum Color {Border, In, Caption, Bar, Clip};

	void setcolor(Color c, unsigned char v);			// sets window color
	unsigned char getcolor(Color c);				// returns window color

	void set(unsigned int v, unsigned int n = 0);
	unsigned int get(unsigned int n = 0)
	{ return bars[n]; };

private:
	unsigned char 	barcol,clipcol;
	unsigned int	*bars,nbars,max;
};

/*
 * bars.cpp - Bars Window, by Simon Peter (dn.tlp@gmx.net)
 */

#include "bars.h"

CBarWnd::CBarWnd(unsigned int n, unsigned int nmax)
	: CWindow(n+2), barcol(7), clipcol(4), nbars(n), max(nmax)
{
	unsigned int i;

	if(!n) return;
	bars = new unsigned int [n];
	for(i=0;i<n;i++)
		bars[i] = 0;
	autocolor = false;
}

void CBarWnd::resize(unsigned char newx, unsigned char newy)
{
	unsigned int i;

	CWindow::resize(newx,newy);
	for(i=0;i<insizex*insizey;i++)
		if(i<(insizey/4)*insizex)
			colmap[i] = clipcol;
		else
			colmap[i] = barcol;
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

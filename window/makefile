# Makefile for WATCOM C/C++ v11, (c) 2001 Simon Peter <dn.tlp@gmx.net>

CC = wcc386
CXX = wpp386

CFLAGS = -oneatx -oh -ei -zp8 -5 -fpi87 -fp5 -zq
CXXFLAGS = -oneatx -oh -oi+ -ei -zp8 -5 -fpi87 -fp5 -zq

.c.obj:
	$(CC) $(CFLAGS) $(CPPFLAGS) $[.

.cpp.obj:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $[.

window.lib: makefile txtgfx.obj window.obj wndman.obj .symbolic
	wlib -n -b window.lib +txtgfx +window +wndman

txtgfx.obj: txtgfx.c txtgfx.h

window.obj: window.cpp window.h

wndman.obj: wndman.cpp wndman.h

clean: .symbolic
	del *.obj

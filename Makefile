# AdPlay/DOS Makefile for WATCOM C/C++ 11.x
# Copyright (c) 2001, 2002 Simon Peter <dn.tlp@gmx.net>

CC = wcc386
CXX = wpp386
LD = wlink

CFLAGS = -oneatx -oh -ei -zp8 -5 -fpi87 -fp5 -zq
CXXFLAGS = -oneatx -oh -oi+ -ei -zp8 -5 -fpi87 -fp5 -zq
LDFLAGS = SYS pmodew OP ST=32k
CPPFLAGS = -dstd= -dstring=String	# trick std::string into WATCOM's String()

.cpp.obj:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $[.

all: adplay.exe

adplay.exe: adplay.obj cfgparse.obj arcfile.obj
        $(LD) F adplay,cfgparse,arcfile LIB window,timer,adplug $(LDFLAGS)
	pmwlite /C4 adplay.exe
	pmwsetup /Q /B0 adplay.exe

adplay.obj: adplay.cpp
cfgparse.obj: cfgparse.cpp cfgparse.h
arcfile.obj: arcfile.cpp arcfile.h

clean: .symbolic
	del *.obj
	del adplay.exe

distclean: clean .symbolic
        del *.err

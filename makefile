# AdPlay/DOS Makefile, (c) 2001 Simon Peter <dn.tlp@gmx.net>

CC = wcc386
CXX = wpp386
LD = wlink

CFLAGS = -oneatx -oh -ei -zp8 -5 -fpi87 -fp5 -zq
CXXFLAGS = -oneatx -oh -oi+ -ei -zp8 -5 -fpi87 -fp5 -zq
LDFLAGS = SYS pmodew OP ST=32k
CPPFLAGS = -dstd= -dstring=String	# trick std::string into WATCOM's String()

adplugdir = ..\adplug
windowdir = window
timerdir = timer

.cpp.obj:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $[.

all: adplay.exe

adplay.exe: adplay.obj cfgparse.obj
	$(LD) F adplay,cfgparse LIB $(windowdir)\window,$(timerdir)\timer,adplug $(LDFLAGS)
	pmwlite /C4 adplay.exe
	pmwsetup /Q /B0 adplay.exe

adplay.obj: adplay.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -i=$(adplugdir) adplay.cpp

cfgparse.obj: cfgparse.cpp cfgparse.h

clean: .symbolic
	del *.obj
	del adplay.exe

# WATCOM C/C++ 11 makefile, by Simon Peter (dn.tlp@gmx.net)

all: players.lib adplay.exe .symbolic

# global defines
CC	= wcc386		# C compiler
CPP	= wpp386		# C++ compiler
MAKE	= wmake		# make utility
LINK	= wlink		# link utility
LIB	= wlib		# library utility

CCOP		= -oneatx -oh -ei -zp8 -5 -fpi87 -fp5 -zq		# C compiler options
CPPOP		= -oneatx -oh -oi+ -ei -zp8 -5 -fpi87 -fp5 -zq	# C++ compiler options
LINKOP	= OP ST=32k					# linker options
DEFINES	= -dstd= -dstring=String -dADPLAY	# trick to convert std::string to WATCOM's String() class
SYSTEM	= pmodew					# target system

ADPLUGPATH	= ..\adplug				# path to adplug sources
WINDOWPATH	= window				# path to window library sources
TIMERPATH	= timer				# path to timer library sources

analopl.obj: analopl.cpp analopl.h
	$(CPP) $(CPPOP) -i=$(ADPLUGPATH) $(DEFINES) analopl.cpp

adplug.obj: $(ADPLUGPATH)\adplug.cpp $(ADPLUGPATH)\adplug.h
	$(CPP) $(CPPOP) -i=$(ADPLUGPATH)\players $(DEFINES) $(ADPLUGPATH)\adplug.cpp

adplay.exe: adplay.cpp makefile adplug.obj analopl.obj
	$(CPP) $(CPPOP) -i=$(ADPLUGPATH) -i=$(ADPLUGPATH)\players -i=$(WINDOWPATH) -i=$(TIMERPATH) $(DEFINES) adplay.cpp
      $(LINK) F adplay,$(WINDOWPATH)\window,$(WINDOWPATH)\wndman,$(WINDOWPATH)\txtgfx,analopl,adplug LIB $(TIMERPATH)\timer,players SYS $(SYSTEM) $(LINKOP)

players.lib: .autodepend
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\protrack.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\a2m.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\amd.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\d00.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\hsc.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\hsp.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\imf.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\ksm.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\mid.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\mtk.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\rad.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\raw.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\s3m.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\sa2.cpp
	$(CPP) $(CPPOP) $(DEFINES) $(ADPLUGPATH)\players\sng.cpp
	$(LIB) -n -b players.lib +protrack +a2m +amd +d00 +hsc +hsp +imf +mtk +rad +raw +s3m +sa2 +mid +sng +ksm

clean: .symbolic
	del *.obj

distclean: clean .symbolic
	del players.lib

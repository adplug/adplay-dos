INSTALL = install
STRIP = strip
ZIP = zip

prefix = /usr/local/djgpp

CPPFLAGS += -DHAVE_GCC_TIMER_H
CPPFLAGS += -O3 -Wall -Wextra
CXXFLAGS += -O3 -Wall -Wextra
CPPFLAGS += -I$(prefix)/include/libbinio -I$(prefix)/include -I$(prefix)/include/adplug -I$(prefix)/include/window
CXXFLAGS += -I$(prefix)/include/libbinio -I$(prefix)/include -I$(prefix)/include/adplug -I$(prefix)/include/window
LDFLAGS += -lstdc++ -lwindow -ladplug -lbinio -lgcctimer
LDFLAGS += -L$(prefix)/lib -L$(prefix)/lib/gcc/i586-pc-msdosdjgpp/$(gcc_version)

ifeq ($(DEBUG),1)
	CPPFLAGS += -DDEBUG
endif

DIST = readme.txt INSTALL.md Makefile build.txt \
	arcfile.h cfgparse.h filewnd.h helptxt.h adplay.cpp arcfile.cpp \
	cfgparse.cpp filewnd.cpp adplay.ini

BINARYDIST = adplay.exe $(srcdir)/adplay.ini $(srcdir)/readme.txt

VERSION = 1.6
NAME = adplay-$(VERSION)
BINARYNAME = adplay$(subst .,,$(VERSION))

bindir = $(prefix)/bin
srcdir = .

adplay.exe: adplay.o arcfile.o cfgparse.o filewnd.o
	$(CXX) -o $@ $^ $(LDFLAGS)

adplay.o: adplay.cpp cfgparse.h arcfile.h filewnd.h helptxt.h
arcfile.o: arcfile.cpp arcfile.h
cfgparse.o: cfgparse.cpp cfgparse.h
filewnd.o: filewnd.cpp filewnd.h arcfile.h

clean:
	rm -f *.o adplay.exe

dist:
	mkdir $(NAME)
	cd $(srcdir); cp $(DIST) $$OLDPWD/$(NAME)
	tar cfj $(NAME).tar.bz2 $(NAME)
	rm -r $(NAME)

binarydist: adplay.exe
	$(STRIP) adplay.exe
	rm -rf $(BINARYNAME).zip $(BINARYNAME)
	mkdir $(BINARYNAME)
	cp $(BINARYDIST) $(BINARYNAME)
	cd $(BINARYNAME) && $(ZIP) ../$(BINARYNAME).zip *
	rm -rf $(BINARYNAME)

install: adplay.exe
	$(INSTALL) adplay.exe $(bindir)

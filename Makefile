INSTALL = install
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

VERSION = 1.7
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
	echo "Compressing adplay.exe"
	@if which upx > /dev/null; then \
		upx --best adplay.exe; \
	elif which strip > /dev/null; then \
		echo "WARNING: Using strip, since upx is not found. Please consider installing upx since it gives the best compression of the binary. Also see UPX manpage https://github.com/upx/upx/blob/250c656b9eb24b0fb54fe8c015d38b0eba5ee80c/doc/upx-doc.txt#L272C6-L272C6 for additional information on upx vs strip."; \
		strip adplay.exe; \
		echo "strip completed"; \
	else \
		echo "ERROR: strip not found, please install either upx or strip" && exit 1; \
	fi
	rm -rf $(BINARYNAME).zip $(BINARYNAME)
	mkdir $(BINARYNAME)
	cp $(BINARYDIST) $(BINARYNAME)
	cd $(BINARYNAME) && $(ZIP) ../$(BINARYNAME).zip *
	rm -rf $(BINARYNAME)

install: adplay.exe
	$(INSTALL) adplay.exe $(bindir)

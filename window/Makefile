INSTALL = install

CC = gcc
DIST = txtgfx_d.c txtgfx.h window.cpp window.h wndman.cpp wndman.h \
	debug.c debug.h readme.txt INSTALL Makefile

VERSION = 1.2

CPPFLAGS += -O3 -Wall -Wextra -Werror
CXXFLAGS += -O3 -Wall -Wextra -Werror

NAME = window-$(VERSION)

prefix = /usr/local/djgpp
includedir = $(prefix)/include/window
libdir = $(prefix)/lib
srcdir = .

libwindow.a: txtgfx_d.o window.o wndman.o debug.o
	$(AR) cru $@ $?
	$(RANLIB) $@

txtgfx_d.o: txtgfx_d.c txtgfx.h
window.o: window.cpp window.h
wndman.o: wndman.cpp wndman.h
debug.o: debug.c debug.h

clean:
	rm -f *.o *.a

dist:
	mkdir $(NAME)
	cd $(srcdir); cp $(DIST) $$OLDPWD/$(NAME)
	tar cfj $(NAME).tar.bz2 $(NAME)
	rm -r $(NAME)

install: libwindow.a
	$(INSTALL) -d $(includedir)
	$(INSTALL) -m 644 $(srcdir)/txtgfx.h $(srcdir)/window.h $(srcdir)/wndman.h $(includedir)
	$(INSTALL) -m 644 libwindow.a $(libdir)

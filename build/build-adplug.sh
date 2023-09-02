#!/bin/bash
#Adplug
cd /build/
git clone https://github.com/adplug/adplug.git --branch adplug-2.3.3 --single-branch
cd adplug
autoreconf
automake --add-missing
autoreconf --install
autoupdate
#PKG_CONFIG_PATH=/djgpp/lib
export libbinio_CFLAGS=-I/djgpp/include/libbinio
export libbinio_LIBS=-L/djgpp/lib #-lbinio

libtoolize --force
aclocal
autoconf
automake --add-missing
./configure --host=i386-pc-msdosdjgpp --prefix=/djgpp iprefix=/djgpp

# patch makefile
# CXXFLAGS += -Wno-deprecated -I/djgpp/include/libbinio
# CPPFLAGS += -Wno-deprecated -I/djgpp/include/libbinio
# LDFLAGS += -lstdc++ -lbinio -lgcctimer -L/djgpp/lib

cp -v /build/adplay-dos/build/adplug-patches/Makefile /build/adplug/Makefile

# patch getopt.c #include "getopt.h" > #include "mygetopt.h"

# patch player.h string > string.h
# patch rix.h   unsigned int getsubsongs();
# patch rix.cpp unsigned int CrixPlayer::getsubsongs()

#cp -Rv /build/adplay-dos/build/adplug-patches/* /build/adplug
patch /build/adplug/src/adplug.h < /build/adplay-dos/build/adplug-patches/src/adplug.h.patch
patch /build/adplug/src/player.h < /build/adplay-dos/build/adplug-patches/src/player.h.patch
patch /build/adplug/src/rix.h < /build/adplay-dos/build/adplug-patches/src/rix.h.patch
patch /build/adplug/src/rix.cpp < /build/adplay-dos/build/adplug-patches/src/rix.cpp.patch
patch /build/adplug/adplugdb/getopt.c < /build/adplay-dos/build/adplug-patches/adplugdb/getopt.c.patch

mkdir /destadplug
make install DESTDIR=/destadplug

cp -Rv /destadplug/djgpp/lib /djgpp
cp -Rv /destadplug/djgpp/include /djgpp
#Adplug
git clone https://github.com/adplug/adplug.git --branch adplug-2.3.3 --single-branch
cd adplug
autoreconf
automake --add-missing
autoreconf --install
autoupdate
#PKG_CONFIG_PATH=/djgpp/lib
export libbinio_CFLAGS=-I/djgpp/include/libbinio
export libbinio_LIBS=-L/djgpp/lib #-lbinio

libtoolize --force && aclocal && autoconf && automake --add-missing && ./configure --host=i386-pc-msdosdjgpp prefix=/djgpp iprefix=/djgpp 

# patch makefile
# CXXFLAGS += -Wno-deprecated -I/home/aranvink/djgpp/include/libbinio
# CPPFLAGS += -Wno-deprecated -I/home/aranvink/djgpp/include/libbinio
LDFLAGS += -lstdc++ -lwindow -ladplug -lbinio -lgcctimer -L$(iprefix)/lib

# patch getopt.c #include "getopt.h" > #include "mygetopt.h"
# patch rix.h   unsigned int getsubsongs();
# patch rix.cpp unsigned int CrixPlayer::getsubsongs()

# patch copied libbinio.la in ~/djgpp/lib to point to correct stdc++ path
# Libraries that this one depends upon.
dependency_libs=' /home/aranvink/djgpp/lib/gcc/i586-pc-msdosdjgpp/12.2.0/libstdc++.la'

# Directory that this library needs to be installed in:
libdir='/home/aranvink/djgpp/lib'

# PATCHES

mkdir /destadplug
make install DESTDIR=/destadplug

cp -R /destadplug/usr/local/include /djgpp
cp -R /destadplug/usr/local/lib /djgpp